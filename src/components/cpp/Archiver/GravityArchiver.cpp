#include "GravityArchiver.h"
#include <string>
#include <time.h>
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
Archiver::Archiver(GravityNode* gn, const string connection_str, const string table_name, std::vector<std::string> dpIDs)
{
    grav_node = gn;

    sql = new cppdb::session("odbc:" + connection_str);
    //("odbc:@Driver=MySql;DSN=test;database=" + db_name + ";user=" + db_user + ";password=" + db_pass);
    //sql = new cppdb::session("odbc:@Driver=MySql;DSN=test"); //We DO need to specify the DSN in ODBC.  That is the only thing we NEED to specify.

    insert_stmt = sql->prepare("INSERT INTO " + table_name + " (timestamp, DataproductID, Message) VALUES (?, ?, ?)");

    dataProductIDs = dpIDs;
}

void Archiver::start()
{
	//Subscribe
	for(std::vector<std::string>::iterator i = dataProductIDs.begin(); i != dataProductIDs.end(); i++)
		grav_node->subscribe(*i, *this);
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
//            time_t mytime = dataProduct.getGravityTimestamp();
//            insert_stmt.bind(1, *gmtime(&mytime));
            insert_stmt.bind(1, dataProduct.getGravityTimestamp());
            cout << dataProduct.getGravityTimestamp() << endl;

            insert_stmt.bind(2, dataProduct.getDataProductID());

            cout << dataProduct.getDataSize() << endl;
            //cout << messageData[4] << endl;

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
