
../../../../ThirdParty/cxxtest/bin/cxxtestgen --error-printer -o runner.cpp ServiceDirectoryTestSuite.h
g++ -o runner -I../../../../ThirdParty/cxxtest runner.cpp
./runner

