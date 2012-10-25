#include "GravityArchiver.h"
#include <GravityConfigParser.h>
#include <sstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#include <ezOptionParser.hpp> //This must be included before Windows.h because of #defines
#pragma GCC diagnostic pop


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN //Smaller include
#include <windows.h> //For Sleep
#define sleep Sleep
#endif

class GravityArchiverConfigParser
{
public:
    GravityArchiverConfigParser();

    /// This call the below four functions
    bool Configure(int argc, const char** argv, gravity::GravityNode &gn);

    void ParseGravityConfig(gravity::GravityNode &gn);
    void ParseCmdLine(int argc, const char** argv);
	void ParseDataproductFile(std::string dpfn);
    bool Validate();

    std::vector<std::string> & getDataProducts() { return dataProducts; }

    std::string getConnectionString() { return con_str; }
    std::string getTableName() { return table_name; }

private:
    std::vector<std::string> dataProducts;

    std::string con_str;
	std::string table_name;

	std::string dpfn;
};

GravityArchiverConfigParser::GravityArchiverConfigParser()
{
}

bool GravityArchiverConfigParser::Configure(int argc, const char** argv, gravity::GravityNode &gn)
{
	ParseGravityConfig(gn);
    ParseCmdLine(argc, argv);

    ParseDataproductFile(dpfn);

	return Validate();
}


void GravityArchiverConfigParser::ParseGravityConfig(gravity::GravityNode &gn)
{
	con_str = gn.getStringParam("ConnectionString", "");

	string dsn, database, user, password, other;
	dsn = gn.getStringParam("DSN", "");
	database = gn.getStringParam("Database", "");
	user = gn.getStringParam("User", "");
	password = gn.getStringParam("Password", "");
	other = gn.getStringParam("OtherDBOpts", "");

	if(con_str != "")
	{
		if(dsn != "" || database != "" || user != "" || password != "" || other != "")
			gravity::Log::warning("Connection string specified but also conflicting DB parameters specified.  Using connection string");
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

	table_name = gn.getStringParam("Table", "");

	dpfn = gn.getStringParam("DataproductFile", "dataproductids");
}

void GravityArchiverConfigParser::ParseCmdLine(int argc, const char** argv)
{
	using namespace ez;
	ezOptionParser* opt = new ezOptionParser();

    opt->add("", false, 1, '\0', "Database connection string", "-c", "--con_str");

    opt->add("", false, 1, '\0', "DataproductID Filename", "-c", "--dp_file");

    opt->add("", false, 1, '\0', "Table Name", "-c", "--table");

    if(opt->get("--con_str")->isSet)
        opt->get("--con_str")->getString(con_str);

    if(opt->get("--dp_file")->isSet)
    {
        opt->get("--dp_file")->getString(dpfn);
    }

    if(opt->get("--table")->isSet)
        opt->get("--table")->getString(table_name);
}


std::string& trim_inplace(
  std::string&       s,
  const std::string& delimiters = " \f\n\r\t\v" )
{
    s.erase( s.find_last_not_of( delimiters ) + 1 );
    return s.erase( 0, s.find_first_not_of( delimiters ) );
}

void GravityArchiverConfigParser::ParseDataproductFile(std::string dpfn)
{
	ifstream config_file;
	string dataProductID;
	config_file.open(dpfn.c_str());

	while(true)
	{
		//Get Next DataproductID
		dataProductID = "";
		while(dataProductID == "" || dataProductID.find_first_of("#") == 0)
		{
			if(!config_file.eof())
			{
				getline(config_file, dataProductID);
				trim_inplace(dataProductID);
			}
			else
				break;
		}

		if(config_file.eof() && dataProductID == "")
			break;

		gravity::Log::message("Recording data product: %s", dataProductID.c_str());
		dataProducts.push_back(dataProductID);
	}
}

bool GravityArchiverConfigParser::Validate()
{
    bool valid = true;

    if(con_str.length() == 0)
    {
    	valid = false;
    	gravity::Log::critical("No connection string or parameters");
    }
	if(table_name.length() == 0)
	{
    	valid = false;
    	gravity::Log::critical("No Table Name");
	}

    if(dataProducts.size() == 0)
	{
    	valid = false;
    	gravity::Log::critical("No Data products");
	}

	return valid;
}


int main(int argc, const char** argv)
{
  using namespace gravity;

  GravityNode gn;
  gn.init("GravityArchiver");

  GravityArchiverConfigParser parser;
  if(!parser.Configure(argc, argv, gn))
  	return -1;

  gravity::Archiver arch(&gn, parser.getConnectionString(),  parser.getTableName(), parser.getDataProducts());

  arch.start();
}
