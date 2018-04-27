
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
namespace gp = google::protobuf;
#include <string>
#include <fcntl.h>
#include <sys/unistd.h>

namespace gravity {
    class PBErrorCollector : public gp::io::ErrorCollector {
    public:
        std::string _context;
        bool errors;
        bool warnings;
        
        PBErrorCollector(std::string context) : _context(context) { }
        ~PBErrorCollector() {}

        void AddError(int line, int column, const std::string& message) {
            errors += 1;
            Log::critical("In '%s' [%d,%d]: %s", _context.c_str(), line, column, message.c_str());
        }
        void AddWarning(int line, int column, const std::string& message) {
            warnings += 1;
            Log::warning("In '%s' [%d,%d]: %s", _context.c_str(), line, column, message.c_str());
        }
    };

    template <class T>
    static bool parseTextPB(const char* filename, T& output) {
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
#endif /* _GRAVITY__PROTOBUF_UTILITIES_H_ */
