#include <sstream>
#include <GravityNode.h>
#include <iniparser.h>

namespace gravity {

class GravityPlayback
{
public:
    GravityPlayback(GravityNode* grav_node, const string db_name, const string table_name, const string db_user, const string db_pass);
    /**
     * Does not return until all data has been played back.
     */
    void start(time_t start_time, time_t end_time);
    //TODO: stop?? thread support???
private:
    GravityNode* grav_node;

    cppdb::session sql;
    cppdb::statement select_stmt;

    std::string table_name;
    time_t start_time;
};

/*
 * Assuming table_name, and other paramters are safe from attacks!!!
 * Throws: cppdb::cppdb_error
 */
GravityPlayback::GravityPlayback(GravityNode* gn, const string db_url, const string db_name, const string table_name, const string db_user, const string db_pass)
{
    grav_node = gn;

    sql.open("odbc:@Driver=MySql;database=" + db_name + ";user=" + db_user + ";password=" + db_pass);
}

class GravityPlaybackConfigParser : public GravityConfigParser
{
public:
    GravityPlaybackConfigParser(GravityNode &gn);
    std::vector<std::string> & getDataProducts() { return dataProducts; }
    std::vector<std::string> & getPorts() { return ports; }
    std::vector<std::string> & getTransports() { return transports; }
protected:
    virtual void GetOtherConfigOptions(dictionary* myconfig);
private:
    std::vector<std::string> dataProducts;
    std::vector<std::string> ports;
    std::vector<std::string> transports;
    GravityNode &grav_node;
};

GravityPlaybackConfigParser:GravityPlaybackConfigParser(GravityNode &gn) : grav_node(gn)
{
}

void GravityPlaybackConfigParser::GetOtherConfigOptions(dictionary* myconfig)
{
    int nsecs = iniparser_getnsec(myconfig);
    for(int i = 0; i < nsecs; i++)
    {
        char* sect_name = iniparser_getsecname(myconfig, i);

        if(strcmp(sect_name, "General") == 0 || strcmp(sect_name, "ServiceDirectory") == 0)
            continue;

        string portKey = sect_name + string(":port");
        string transportKey = sect_name + string(":transport");

        int port = iniparser_getint(myconfig, portKey.c_str(), -1);
        string transport = iniparser_getstring(myconfig, portKey.c_str(), "tcp");

    }
}

void GravityPlayback::start(time_t start_time, time_t end_time)
{
    GravityPlaybackConfigParser parser(grav_node);
    parser.ParseConfigFile("GravityPlayback.ini");

    vector<string> &dps = parser.getDataProducts();
    vector<string> &ports = parser.getPorts();
    vector<string> &transports = parser.getTransports();
    stringstream values;
    for(int i = 0; i < dps.size(); i++)
    {
        int port = ports[i];
        string transport = transports[i];

        if(port == -1)
        {
            cerr << "Invalid Port for Stream '" << sect_name << "'" << endl;
            continue;
        }

        if(transport != "tcp" && transport != "icp")
        {
            cerr << "Invalid Transport type '" << transport << "'for Stream '" << sect_name << "'" << endl;
            continue;
        }

        cout << "Registering " << dps[i] << " on port " << port << endl;
        grav_node.registerDataProduct(dps[i], (unsigned short) port, transport);

        string dp = sql.escape(dps[i].c_str()); //Just in case :?
        values << dp;

        if(i != dps.size() - 1) //If we're not the last guy.
            values << ",";
    }

    select_stmt = sql.prepare("SELECT (timestamp, DataproductID, Message) FROM " + table_name + " WHERE timestamp > ? AND timestamp <= ? AND DataproductID IN(" + values.str() + ")");


    //cppdb::result dp_results = sql << "SELECT DISTINCT DataproductID FROM " << table_name << " WHERE timestamp >= " << start_time <<
    //        "AND timestamp < " << end_time << cppdb::exec;

    time_t startdb_timestep, currentdb_end_timestep;
    time_t current_time, last_start_time, next_start_time;
    time(&current_time);
    next_start_time = current_time; //When we "hope" to start the next timestep (this time we want to start it immediately).
    last_start_time = current_time - 1;  //This will be our "last" time the next time we enter this loop.

    startdb_timestep = start_time - 1;
    currentdb_end_timestep = start_time;

    do
    {
        //If we've just processed a message sleep until we need to do the next message.
        time(&current_time);
        while(difftime(current_time, next_start_time) > 0.0) //TODO: make sure this is right.
        {
            sleep(0);
            time(&current_time);
        }
        startdb_timestep = currentdb_timestep;
        currentdb_end_timestep = startdb_timestep + difftime(last_start_time, current_time);
        if(currentdb_end_timestep > end_time)
            currentdb_end_timestep = end_time;


        next_start_time = current_time + 1; //When we "hope" to start the next timestep.
        last_start_time = current_time;  //This will be our "last" time the next time we enter this loop.

        //Publish data products.
        select_stmt.reset();
        select_stmt.bind(1, startdb_timestep);
        select_stmt.bind(2, currentdb_end_timestep);
        cppdb::result res = select_stmt.query();

        while(res.next())
        {
            res.fetch(1, currentdb_timestep);

            //TODO: fix timestamp!!!

            std::string dpID, message;
            res.fetch(2, dpID);
            res.fetch(3, message);

            GravityDataProduct gdp(dpID);
            gdp.setData(message.c_str(), message.length());
            grav_node->publish(gdp);
        }

    } while(currentdb_end_timestep != end_time); //While we still have time left.
}

}
