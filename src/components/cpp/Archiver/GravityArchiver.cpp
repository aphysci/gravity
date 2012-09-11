#include "GravityArchiver.h"
#include <string>
#include <stdarg.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <fstream>

#include <cppdb/frontend.h>

using namespace std;

namespace gravity {

/*
 * Assuming table_name, and other paramters are safe from attacks!!!
 * Throws: cppdb::cppdb_error
 */
Archiver::Archiver(GravityNode* gn, const string db_name, const string table_name, const string db_user, const string db_pass)
{
    grav_node = gn;

    //sql = new cppdb::session("odbc:@Driver=MySql;database=" + db_name + ";user=" + db_user + ";password=" + db_pass);    
    sql = new cppdb::session("odbc:@Driver=MySql;DSN=timtest");

    insert_stmt = sql->prepare("INSERT INTO " + table_name + " (timestamp, DataproductID, Message) VALUES (?, ?, ?)");
}

std::string& trim_inplace(
  std::string&       s,
  const std::string& delimiters = " \f\n\r\t\v" )
{
    s.erase( s.find_last_not_of( delimiters ) + 1 );
    return s.erase( 0, s.find_first_not_of( delimiters ) );
}

void Archiver::start()
{
    ifstream config_file;
    string dataProductID;
    config_file.open("config_file");

    //Subscribe to the DataProduct IDs in config file.


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

        //Subscribe
        grav_node->subscribe(dataProductID, *this);
        dataProductIDs.push_back(dataProductID);
    }
}

void Archiver::subscriptionFilled(const GravityDataProduct& dataProduct)
//(string dataProductID, const vector<shared_ptr<GravityDataProduct> > dataProducts)
{
    try
    {
//        for(vector<shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin(); i != dataProducts.end(); i++)
//        {
//            shared_ptr<GravityDataProduct> dataProduct = *i;

            char messageData[1024];
            int msg_size = 1024;
            dataProduct.getData(messageData, msg_size); //Warning: This copies the data!!!

            insert_stmt.reset();
            insert_stmt.bind(1, dataProduct.getGravityTimestamp());
            insert_stmt.bind(2, dataProduct.getDataProductID());

            cout << dataProduct.getDataSize() << endl;
            cout << messageData[4] << endl;

            insert_stmt.bind(3, messageData, messageData + dataProduct.getDataSize());

            insert_stmt.exec();
//        }
    }
    catch(cppdb::cppdb_error const &e)
    {
        cerr << "Error Writing to Database.  " << e.what() << endl;
    }
}

Archiver::~Archiver()
{
    //Unsubscribe
    for(vector<string>::iterator i = dataProductIDs.begin(); i != dataProductIDs.end(); i++)
    {
            const string dataProductID = *i;
            grav_node->unsubscribe(dataProductID, *this);
    }

    //Close the database connection!!!
    delete sql;
}

}
