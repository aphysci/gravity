#include "GravityNode.h"
#include "GravitySubscriber.h"
#include <string>
#include <stdarg.h>
#include <stdio.h>

#include <cppdb/frontend.h>

namespace gravity {

class Archiver : public GravitySubscriber
{
public:
    /*
     * Initalizes the Archiver
     * \param grav_node Assumed to be initialized.
     */
    Archiver(GravityNode* gn, const string db_url, const string db_name, const string table_name, const string db_user, const string db_pass);

    /*
     * Starts the Archiver (Calls Subscribe).
     */
    void start();

    /**
     * Writes to the Database
     */
    virtual void subscriptionFilled(const GravityDataProduct &dataProducts);

    /**
     * Destructor: Unsubscribe from streams and close the DB connection.
     */
    virtual ~Archiver();
private:
    GravityNode* grav_node;

    cppdb::session* sql;
    cppdb::statement insert_stmt;

    std::vector<std::string> dataProductIDs;
};

}
