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
    /**
     * Initalizes the Archiver
     * \param grav_node Assumed to be initialized.
     */
    Archiver(GravityNode* gn, const std::string connection_str, const std::string table_name, std::vector<std::string> dpIDs);

    /**
     * Starts the Archiver (Calls Subscribe).
     */
    void start();

    /**
     * Writes to the Database
     */
    virtual void subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts);

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
