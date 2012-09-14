#include "GravityArchiver.h"
#include <GravityConfigParser.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN //Smaller include
#include <windows.h> //For Sleep
#define sleep Sleep
#endif

class GravityArchiverConfigParser : public gravity::GravityConfigParser
{
public:
    GravityArchiverConfigParser(const char* config_filename);

    /// This call the below four functions
    bool Configure(int argc, const char** argv);

    void ParseConfigFile();
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

GravityArchiverConfigParser::GravityArchiverConfigParser(const char* config_filename) : GravityConfigParser(config_filename)
{
}

bool GravityArchiverConfigParser::Configure(int argc, const char** argv)
{
    ParseConfigFile();
    ParseCmdLine(argc, argv);

    ParseDataproductFile(dpfn);

	return Validate();
}


void GravityArchiverConfigParser::ParseConfigFile()
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

	dpfn = getString("Playback:DataproductFile", "dataproductids");
}

void GravityArchiverConfigParser::ParseCmdLine(int argc, const char** argv)
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

		dataProducts.push_back(dataProductID);
	}
}

bool GravityArchiverConfigParser::Validate()
{
    bool valid = true;

    if(con_str.length() == 0)
    {
    	valid = false;
    	cerr << "No connection string or parameters" << endl;
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

	return valid;
}


int main(int argc, const char** argv)
{
  using namespace gravity;

  GravityArchiverConfigParser parser("GravityArchiver.ini");
  if(!parser.Configure(argc, argv))
  	return -1;

  GravityNode gn;
  gn.init();

  gravity::Archiver arch(&gn, parser.getConnectionString(),  parser.getTableName(), parser.getDataProducts());

  arch.start();

  while(true)
  {
    sleep(4294967295u); //Sleep for as long as we can.  (We can't join on the Subscription Manager thread).
  }

}
