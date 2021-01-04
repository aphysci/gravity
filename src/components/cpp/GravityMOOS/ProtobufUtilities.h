
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

#ifndef _GRAVITY__PROTOBUF_UTILITIES_H_
#define _GRAVITY__PROTOBUF_UTILITIES_H_

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>

#include <string>

namespace gravity {

bool protobufToText(const google::protobuf::Message& input, std::string& output);
bool textToProtobuf(const std::string& input, google::protobuf::Message& output,
                    const std::string& context="");
bool textFileToProtobuf(const char *filename, google::protobuf::Message& output);

class MultiPBErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector {
public:
    MultiPBErrorCollector();
    ~MultiPBErrorCollector();

    void AddError(const std::string& filename, int line, int column, const std::string& message);
    void AddWarning(const std::string& filename, int line, int column, const std::string& message);
};

class PBErrorCollector : public google::protobuf::io::ErrorCollector {
public:
    const std::string context;
    int errors = 0;
    int warnings = 0;
    
    PBErrorCollector(std::string filename_or_typename);
    ~PBErrorCollector();

    void AddError(int line, int column, const std::string& message);
    void AddWarning(int line, int column, const std::string& message);
};

class ProtobufRegistry {
public:
    ProtobufRegistry();
    virtual ~ProtobufRegistry();
    
    void setProtobufPath(const std::string& path);
    std::shared_ptr<google::protobuf::Message> createMessageByName(const std::string& name);

private:
    google::protobuf::compiler::DiskSourceTree _tree;
    google::protobuf::compiler::SourceTreeDescriptorDatabase _db;
    google::protobuf::DescriptorPool _pool;
    google::protobuf::DynamicMessageFactory _factory;
    
    MultiPBErrorCollector _errors;
};

} /* namespace gravity */
#endif /* _GRAVITY__PROTOBUF_UTILITIES_H_ */
