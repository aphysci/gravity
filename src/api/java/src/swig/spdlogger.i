/** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
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

namespace gravity{

#define SPD_LOG_OFF 6
#define SPD_LOG_CRITICAL 5
#define SPD_LOG_ERROR 4
#define SPD_LOG_WARNING 3
#define SPD_LOG_INFO 2
#define SPD_LOG_DEBUG 1
#define SPD_LOG_TRACE 0

class SpdLog
{
public:
    enum LogLevel {
        TRACE ,
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL,
        OFF
    };

%typemap(in) (const char* message) {
    $1 = (char *)jenv->GetStringUTFChars($input, 0);
    if (!$1) return ;
}
%typemap(freearg) (const char* message) {
    if ($1) jenv->ReleaseStringUTFChars($input, (const char *)$1);
}

    static void critical(const char* message);
	static void error(const char* message);
    static void warn(const char* message);
    static void info(const char* message);
    static void debug(const char* message);
    static void trace(const char* message);

};

};
