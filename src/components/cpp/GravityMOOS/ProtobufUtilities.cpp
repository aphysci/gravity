/** (C) Copyright 2018, Applied Physical Sciences Corp., A General Dynamics Company
 **
 ** Gravity is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program;
 ** If not, see <http://www.gnu.org/licenses/>.
 **
 */

#include "ProtobufUtilities.h"
using namespace std;
using namespace std::tr1;

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
namespace fs = boost::filesystem;

namespace gravity {

ProtobufRegistry::ProtobufRegistry() : _db(&_tree), _pool(&_db) {
    _factory.SetDelegateToGeneratedFactory(true);
    //_db.RecordErrorsTo(MYERRORCOLLECTOR);
}

ProtobufRegistry::~ProtobufRegistry() { }

void ProtobufRegistry::setProtobufPath(const std::string& path) {
    _tree.MapPath("", path);
    
    // boost:filesystem is now in C++17.
    fs::path root = fs::canonical(path);
    fs::recursive_directory_iterator iter(root, fs::symlink_option::no_recurse), eod;

    BOOST_FOREACH(boost::filesystem::path const& node, make_pair(iter, eod)) {
        if (is_regular_file(node) && node.extension() == ".proto"){
            // We're not following symlinks, and we are using canonical paths, so we can cheat.
            // We want to add everything relative to the same folder root, or the database
            // ends up with multiple names for the same thing and gets confused.
            string relative_path = fs::canonical(node).string().substr(root.string().size()+1);
            // Now parse all the types in the file.
            const gp::FileDescriptor* fd = _pool.FindFileByName(relative_path);
            if (!fd) {
                throw std::runtime_error("Couldn't load protobuf file " + relative_path + ".");
            }
            for (int i=0; i < fd->message_type_count(); ++i) {
                const gp::Descriptor* desc = fd->message_type(i);
                Log::debug("Loaded type %s from %s", desc->full_name().c_str(), relative_path.c_str());
            }
        }
    }
}

std::tr1::shared_ptr<gp::Message> ProtobufRegistry::createMessageByName(const std::string& name) {
    const gp::Descriptor* desc = _pool.FindMessageTypeByName(name);
    if (!desc) {
        throw std::runtime_error("Could not find protofile for type '" + name + "'.");
    }
    const gp::Message* prototype = _factory.GetPrototype(desc);
    return std::tr1::shared_ptr<gp::Message>(prototype->New());
}

bool parseTextPB(const char* filename, gp::Message& output) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        Log::critical("Failed to open '%s'", filename);
        return false;
    }
    gp::io::FileInputStream reader(fd);
    gp::TextFormat::Parser parser;
    PBErrorCollector errorCollector(filename);
    parser.RecordErrorsTo(&errorCollector);
    parser.Parse(&reader, &output);
    close(fd);
    
    return (errorCollector.errors == 0);
}

PBErrorCollector::PBErrorCollector(string filename_or_typename) : context(filename_or_typename) { }
PBErrorCollector::~PBErrorCollector() { }

void PBErrorCollector::AddError(int line, int column, const std::string& message) {
    errors += 1;
    Log::critical("In '%s' [%d,%d]: %s", context.c_str(), line, column, message.c_str());
}
void PBErrorCollector::AddWarning(int line, int column, const std::string& message) {
    warnings += 1;
    Log::warning("In '%s' [%d,%d]: %s", context.c_str(), line, column, message.c_str());
}

bool protobufToText(const gp::Message& input, std::string& output) {
    return gp::TextFormat::PrintToString(input, &output);
}

bool textToProtobuf(const std::string& input, gp::Message& output, const std::string& context) {
    gp::TextFormat::Parser parser;
    PBErrorCollector errorCollector(context);
    parser.RecordErrorsTo(&errorCollector);
    parser.ParseFromString(input, &output);
    
    return (errorCollector.errors == 0);
}
        
bool textFileToProtobuf(const char *filename, gp::Message& output) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        Log::critical("Failed to open '%s'", filename);
        return false;
    }
    gp::io::FileInputStream reader(fd);
    gp::TextFormat::Parser parser;
    PBErrorCollector errorCollector(filename);
    parser.RecordErrorsTo(&errorCollector);
    parser.Parse(&reader, &output);
    close(fd);
    
    return (errorCollector.errors == 0);
}
        
} /* namespace gravity */

