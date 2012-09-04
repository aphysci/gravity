#ifndef GRAVITYCONFIGPARSER_H_
#define GRAVITYCONFIGPARSER_H_


#include "GravityLogger.h"
#include <string>
#include <map>
#include <iniparser.h>
#include <dictionary.h>

/**
 * A really simple C++ wrapper for the iniparser library.
 */
class IniConfigParser
{
public:
    /**
     * Constructor
     */
    IniConfigParser() {
        mydict = NULL;
    }

    /**
     * Opens the config file and reads in the contents.
     */
    bool Open(const char* filename) {
        if(mydict != NULL)
            return false;

        mydict = iniparser_load(filename);

        bool isSuccess = (mydict != NULL);
        return isSuccess;
    }

    /**
     * Frees the resources asscociated with the config file.  Same as the destructor.
     */
    void Close()
    {
        if(mydict)
            iniparser_freedict(mydict);
        mydict = NULL;
    }

    /**
     * Destructor: closes the config file.
     */
    virtual ~IniConfigParser() {
        Close();
    }


    /** @name Getters and Setters
     * These functions use the format "section:key" for the key name.
     * @{
     */
    std::string getString(const std::string key, const std::string default_value = "") {
        return iniparser_getstring(mydict, key.c_str(), (char*) default_value.c_str());
    }

    int getInt(const std::string key, int default_value = -1) {
        return iniparser_getint(mydict, key.c_str(), default_value);
    }

    double getDouble(const std::string key, double default_value = 0.0) {
        return iniparser_getdouble(mydict, key.c_str(), default_value);
    }

    bool getBoolean(const std::string key, bool default_value = false) {
        return iniparser_getboolean(mydict, key.c_str(), default_value);
    }

    void Set(const std::string key, const std::string value) {
        iniparser_set(mydict, key.c_str(), value.c_str());
    }

    void Clear(const std::string key) {
        iniparser_unset(mydict, key.c_str());
    }
    /**@}*/ //End getters and setters

    /**
     * Write the config to a file.
     * \param   filename    The name and/or path of the file to write to.
     */
    bool Save(const char* filename) {
        FILE* file = fopen(filename, "w");
        if(!file)
            return false;
        iniparser_dump_ini(mydict, file);
        fclose(file);

        return true;
    }

    /**
     * @Name Finding
     * @{
     */
    /** Get Section Names */
    std::vector<std::string> GetSections() {
        int nsect = iniparser_getnsec(mydict);
        std::vector<std::string> sects(nsect);
        for(int i = 0; i < nsect; i++)
        {
            sects[i] = iniparser_getsecname(mydict, i);
        }

        return sects;
    }
    /**
     * Get Key Names in Section
     * \param   section   The name of the section to get the keys for.
     */
    std::vector<std::string> GetSectionKeys(const std::string section) {
        int nsect = iniparser_getsecnkeys(mydict, (char*) section.c_str());
        std::vector<std::string> sects(nsect);
        const char** keys = iniparser_getseckeys(mydict, (char*) section.c_str());
        for(int i = 0; i < nsect; i++)
        {
            sects[i] = keys[i];
        }

        return sects;
    }

    /**
     * Check if a key exists
     * \param   key   The key to check for.  Use the format "section:key" for the key name.
     */
    bool KeyExists(const std::string key) {
        return (iniparser_find_entry(mydict, key.c_str()) == 1);
    }
    /**@}*/ //End Finding
private:
    dictionary* mydict;
};

namespace gravity {

class GravityConfigParser : public IniConfigParser
{
public:
    /** Constructor */
    GravityConfigParser(const char* config_filename);
    /** Update the configuration based on command line parameters */
    void ParseCmdLine(int argc, const char** argv);
    /** Update configuration based on .ini file */
    void ParseConfigFile();

    /** Get the url of the service directory from the config. */
    std::string getServiceDirectoryUrl() { return serviceDirectoryUrl; }

    /** Get the Local Logging level from the config. */
    gravity::Log::LogLevel getLocalLogLevel() { return log_local_level; }

    /** Get the Network Logging level from the config. */
    gravity::Log::LogLevel getNetLogLevel() { return log_net_level; }

private:
    std::string serviceDirectoryUrl;
    gravity::Log::LogLevel log_local_level;
    gravity::Log::LogLevel log_net_level;
};

}

#endif // GRAVITYCONFIGPARSER_H_
