#include <sstream>
#include <GravityNode.h>
#include <iniparser.h>

namespace gravity {

class GravityPlayback
{
public:
    GravityPlayback(GravityNode* grav_node, const string db_url, const string db_name, const string table_name, const string db_user, const string db_pass);
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

    time(&start_time);
}

}
//    GravityReturnCode registerDataProduct(string dataProductID, unsigned short networkPort, string transportType);
//    GravityReturnCode publish(const GravityDataProduct& dataProduct);
