
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

#include "GravityLogger.h"
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/compiler/importer.h>
namespace gp = google::protobuf;
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/unistd.h>
#include <tr1/memory>

namespace gravity {
    class ProtobufRegistry {
    public:
        ProtobufRegistry();
        virtual ~ProtobufRegistry();
        
        void setProtobufPath(const std::string& path);
        std::tr1::shared_ptr<gp::Message> createMessageByName(const std::string& name);

    private:
        gp::compiler::DiskSourceTree _tree;
        gp::compiler::SourceTreeDescriptorDatabase _db;
        gp::DescriptorPool _pool;
        gp::DynamicMessageFactory _factory;
    };

    class PBErrorCollector : public gp::io::ErrorCollector {
    public:
        const std::string context;
        int errors;
        int warnings;
        
        PBErrorCollector(std::string filename_or_typename);
        ~PBErrorCollector();

        void AddError(int line, int column, const std::string& message);
        void AddWarning(int line, int column, const std::string& message);
    };

    bool protobufToText(const gp::Message& input, std::string& output);
    bool textToProtobuf(const std::string& input, gp::Message& output,
                        const std::string& context="");
    bool textFileToProtobuf(const char *filename, gp::Message& output);

} /* namespace gravity */
#endif /* _GRAVITY__PROTOBUF_UTILITIES_H_ */
