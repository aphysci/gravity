#ifndef INIPARSER_WRAP_H__
#define INIPARSER_WRAP_H__
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
    	std::string key_lower = gravity::StringCopyToLowerCase(key);
    	return iniparser_getstring(mydict, key_lower.c_str(), (char*) default_value.c_str());
    }

    int getInt(const std::string key, int default_value = -1) {
    	std::string key_lower = gravity::StringCopyToLowerCase(key);
        return iniparser_getint(mydict, key_lower.c_str(), default_value);
    }

    double getDouble(const std::string key, double default_value = 0.0) {
    	std::string key_lower = gravity::StringCopyToLowerCase(key);
        return iniparser_getdouble(mydict, key_lower.c_str(), default_value);
    }

    bool getBoolean(const std::string key, bool default_value = false) {
    	std::string key_lower = gravity::StringCopyToLowerCase(key);
        return (bool) iniparser_getboolean(mydict, key_lower.c_str(), default_value);
    }

    void Set(const std::string key, const std::string value) {
    	std::string key_lower = gravity::StringCopyToLowerCase(key);
        iniparser_set(mydict, key_lower.c_str(), value.c_str());
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
    	std::string section_cpy = gravity::StringCopyToLowerCase(section);

        int nsect = iniparser_getsecnkeys(mydict, (char*) section_cpy.c_str());
        std::vector<std::string> sects(nsect);
        const char** keys = iniparser_getseckeys(mydict, (char*) section_cpy.c_str());
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
protected:
    dictionary* mydict;
};

#endif //INIPARSER_WRAP_H__