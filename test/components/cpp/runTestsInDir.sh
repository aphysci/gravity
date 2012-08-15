
../../../../ThirdParty/cxxtest/bin/cxxtestgen --error-printer -o runner.cpp *.h
g++ -o runner -I../../../../ThirdParty/cxxtest runner.cpp
./runner

