#include "GravityPlayback.h"
#include <GravityConfigParser.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define sleep(x) Sleep(x)
#endif


std::string& trim_right_inplace(
  std::string&       s,
  const std::string& delimiters = " \f\n\r\t\v" )
{
  return s.erase( s.find_last_not_of( delimiters ) + 1 );
}

std::string& trim_left_inplace(
  std::string&       s,
  const std::string& delimiters = " \f\n\r\t\v" )
{
  return s.erase( 0, s.find_first_not_of( delimiters ) );
}

std::string& trim(
  std::string&       s,
  const std::string& delimiters = " \f\n\r\t\v" )
{
  return trim_left_inplace( trim_right_inplace( s, delimiters ), delimiters );
}

using namespace std;

namespace gravity {

class GravityPlaybackConfigParser : public GravityConfigParser
{
public:
    GravityPlaybackConfigParser(const char* config_filename);

    /// This call the below four functions
    bool Configure(int argc, const char** argv);

    void ParseConfigFile();
    void ParseCmdLine(int argc, const char** argv);
	void ParseDataproductFile(std::string dpfn);
    bool Validate();

    std::vector<std::string> & getDataProducts() { return dataProducts; }
    std::vector<int> & getPorts() { return ports; }
    std::vector<std::string> & getTransports() { return transports; }

    std::string getConnectionString() { return con_str; }
    uint64_t getStartTime() { return start_time; }
    uint64_t getEndTime() { return end_time; }
    std::string getTableName() { return table_name; }

private:
    std::vector<std::string> dataProducts;
    std::vector<int> ports;
    std::vector<std::string> transports;

    std::string con_str;
	uint64_t start_time;
	uint64_t end_time;
	std::string table_name;

	std::string dpfn;
};

GravityPlaybackConfigParser::GravityPlaybackConfigParser(const char* config_filename) : GravityConfigParser(config_filename)
{
	start_time = 0;
	end_time = 0;
}

void GravityPlaybackConfigParser::ParseConfigFile()
{
	//Do global configs
	GravityConfigParser::ParseConfigFile();

	con_str = getString("Playback:ConnectionString", "");

	string dsn, database, user, password, other;
	dsn = getString("Playback:DSN", "");
	database = getString("Playback:Database", "");
	user = getString("Playback:User", "");
	password = getString("Playback:Password", "");
	other = getString("Playback:OtherDBOpts", "");

	if(con_str != "")
	{
		if(dsn != "" || database != "" || user != "" || password != "" || other != "")
			cout << "Warning connection string specified but also conflicting DB parameters specified.  Using connection string" << endl;
	}
	else
	{
		stringstream ss;
		if(dsn != "")
			ss << "DSN=" << dsn << ";";
		if(database != "")
			ss << "Database=" << database << ";";
		if(user != "")
			ss << "Uid=" << user << ";";
		if(password != "")
			ss << "Pwd=" << password << ";";
		if(other != "")
			ss << other;
		con_str = ss.str();
	}

	table_name = getString("Playback:Table", "");

	{
		string st = getString("Playback:StartTime", "");
		string et= getString("Playback:EndTime", "");

		//NOTE: we need to do this like this to support 64 bit numbers.
		stringstream ss1(st);
		stringstream ss2(et);

    	ss1 >> start_time;
    	ss2 >> end_time;
	}

	dpfn = getString("Playback:DataproductFile", "dataproductids");
}

void GravityPlaybackConfigParser::ParseCmdLine(int argc, const char** argv)
{
    opt->add("", false, 1, '\0', "Database connection string", "-c", "--con_str");

    opt->add("", false, 1, '\0', "DataproductID Filename", "-c", "--dp_file");

    opt->add("", false, 1, '\0', "Table Name", "-c", "--table");

	GravityConfigParser::ParseCmdLine(argc, argv);

    if(opt->get("--con_str")->isSet)
        opt->get("--con_str")->getString(con_str);

    if(opt->get("--dp_file")->isSet)
    {
        opt->get("--dp_file")->getString(dpfn);
    }

    if(opt->get("--table")->isSet)
        opt->get("--table")->getString(table_name);

    if(opt->lastArgs.size() == 2)
    {
    	stringstream ss1(opt->lastArgs[0]->c_str());
    	stringstream ss2(opt->lastArgs[1]->c_str());

    	ss1 >> start_time;
    	ss2 >> end_time;
    }
}

void GravityPlaybackConfigParser::ParseDataproductFile(string dpfn)
{
	std::ifstream file(dpfn.c_str());
	if(!file.is_open())
		cerr << "Could not open file specified in config: " << dpfn << endl;
	else
	{
		int line_no = 0;

		while(!file.eof())
		{
			string line;
			getline(file, line);
			line_no++;
			//cout << "line " << line_no << endl;

			trim(line);

			if(line != "" && line.find_first_of('#') != 0)
			{
				string dp, port_str, transport;

				size_t split = line.find_first_of(':');

				if(split == string::npos)
				{
					cerr << "Invalid Configuration in file " << dpfn << " at line " << line_no << endl;
					continue;
				}

				size_t split2 = line.find_first_of(':', split + 1);
				if(split2 == string::npos)
				{
					split2 = line.length();
					transport = "tcp";
				}
				else
					transport = line.substr(split2 + 1, line.length());
				dp = line.substr(0, split);
				port_str = line.substr(split + 1, split2);

				unsigned short port;
				stringstream ss(port_str);
				ss >> port;

				//cout << dp << ":" << port << ":" << transport << endl;
				dataProducts.push_back(dp);
				ports.push_back(port);
				transports.push_back(transport);
			}
		}
	}
}

bool GravityPlaybackConfigParser::Validate()
{
    bool valid = true;

    if(con_str.length() == 0)
    {
    	valid = false;
    	cerr << "No connection string or parameters" << endl;
    }

    if(start_time == 0)
    {
    	valid = false;
    	cerr << "Invalid Start Time" << endl;
    }
	if(end_time == 0)
    {
    	valid = false;
    	cerr << "Invalid End Time" << endl;
    }
	if(table_name.length() == 0)
	{
    	valid = false;
    	cerr << "No Table Name" << endl;
	}

    if(dataProducts.size() == 0)
	{
    	valid = false;
    	cerr << "No Data products" << endl;
	}

    if(dataProducts.size() != ports.size())
    {
    	valid = false;
    	cerr << "Configuration Error: problem with ports" << endl;
    }

    if(dataProducts.size() != transports.size())
    {
    	valid = false;
    	cerr << "Configuration Error: problem with transports" << endl;
    }

	return valid;
}

bool GravityPlaybackConfigParser::Configure(int argc, const char** argv)
{
    ParseConfigFile();
    ParseCmdLine(argc, argv);

    ParseDataproductFile(dpfn);

	return Validate();
}

}//namespace

int main(int argc, const char** argv)
{
	using namespace gravity;

    GravityPlaybackConfigParser parser("GravityPlayback.ini");
    if(!parser.Configure(argc, argv))
    	return -1;

	GravityNode gn;
	gn.init();

	GravityPlayback gp(&gn, parser.getConnectionString());
	gp.start(parser.getStartTime(), parser.getEndTime(), parser.getTableName(), parser.getDataProducts(), parser.getPorts(), parser.getTransports());

	return 0;
}

namespace gravity
{

GravityPlayback::GravityPlayback(GravityNode* gn, const string connetion_str)
{
    grav_node = gn;

    sql.open("odbc:" + connetion_str);
}

uint64_t difftime(uint64_t start_time, uint64_t end_time)
{
	return end_time - start_time;
}

void GravityPlayback::start(uint64_t start_time, uint64_t end_time, string table_name, vector<string> &dps, vector<int> &ports, vector<string> &transports)
{
	end_time += 1; //So we have the end time inclusive.

    stringstream values;
    for(int i = 0; i < (int) dps.size(); i++)
    {
        int port = ports[i];
        string transport = transports[i];

        if(port == -1)
        {
            cerr << "Invalid Port for Stream '" << dps[i] << "'" << endl;
            continue;
        }

        if(transport != "tcp" && transport != "icp")
        {
            cerr << "Invalid Transport type '" << transport << "'for Stream '" << dps[i] << "'" << endl;
            continue;
        }

        cout << "Registering " << dps[i] << " on port " << port << endl;
        grav_node->registerDataProduct(dps[i], (unsigned short) port, transport);

//        string dp = sql.escape(dps[i].c_str()); //Just in case :?
        string dp = dps[i].c_str();
        values << '\"' << dp << '\"' ;
        //values << dp ;

        if(i != (int) dps.size() - 1) //If we're not the last guy.
            values << ",";
    }

    select_stmt = sql.prepare("SELECT `timestamp`, `DataproductID`, `message` FROM `" + table_name + "` where `DataproductID` IN (" + values.str() + ") AND `timestamp` >= ? AND `timestamp` < ? ORDER BY `timestamp`");

    //cppdb::result dp_results = sql << "SELECT DISTINCT DataproductID FROM " << table_name << " WHERE timestamp >= " << start_time <<
    //        "AND timestamp < " << end_time << cppdb::exec;

	uint64_t clock_start_time = grav_node->getCurrentTime(); //In System Time
	uint64_t current_time = clock_start_time; //In System Time
    uint64_t startdb_timestep = start_time;
    uint64_t currentdb_end_timestep = startdb_timestep + 1000000;
    if(currentdb_end_timestep > end_time)
        currentdb_end_timestep = end_time;

    do
    {
    	//Publish data products.
        select_stmt.reset();
        select_stmt.bind(1, startdb_timestep);
        select_stmt.bind(2, currentdb_end_timestep);
        cppdb::result res = select_stmt.query();

        while(res.next())
        {
        	std::stringstream timestream;
        	uint64_t currentdb_timestep;
            res.fetch(0, currentdb_timestep);

            std::string dpID, message;
            res.fetch(1, dpID);
            res.fetch(2, message);

            GravityDataProduct gdp(dpID);
            gdp.setData(message.c_str(), message.length());

            //Wait until the proper time to publish.
            current_time = grav_node->getCurrentTime();
            //sleep for (current time relative to query start - time when we need to publish relative to query start).
#ifndef WIN32
            if((currentdb_timestep - start_time) > (current_time - clock_start_time))
            	usleep((currentdb_timestep - start_time) - (current_time - clock_start_time));
#else
            if((currentdb_timestep - start_time) > (current_time - clock_start_time))
            	Sleep(((currentdb_timestep - start_time) - (current_time - clock_start_time))/1000);
#endif

            grav_node->publish(gdp);
        }

        startdb_timestep = currentdb_end_timestep;
        currentdb_end_timestep = startdb_timestep + 1000000;
        if(currentdb_end_timestep > end_time)
            currentdb_end_timestep = end_time;

    } while(currentdb_end_timestep > startdb_timestep && currentdb_end_timestep <= end_time); //While we still have time left.
}

}
