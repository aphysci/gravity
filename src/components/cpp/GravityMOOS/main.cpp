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

#include "GravityMOOS.h"

using namespace gravity;

char* getCmdOption(char** begin, char** end, const std::string& option) {
    char** itr = std::find(begin, end, option);
    if(itr != end && ++itr !=end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

void printUsage() {
    printf("usage: GravityMOOS -f filename [-h] \n");
    printf("\t -f filename:\t Configuration filename\n");
    printf("\t -h:\t\t Display this text\n");
}

int main(int argc, char* argv[]) {
    if(cmdOptionExists(argv, argv + argc, "-h")) {
        printUsage();
        return -1;
    }

    std::string config_file = "";

    char* fname = getCmdOption(argv, argv + argc, "-f");
    if (fname) {
        config_file = std::string(fname);
    } else {
        printUsage();
        return -1;
    }

    GravityMOOS gravityMOOS(config_file);
    
    return gravityMOOS.run();
}

