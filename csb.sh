mkdir -p /usr/local/lib/gravity
mkdir -p /usr/local/include/gravity
cp -r ./src/api/cpp/*.h /usr/local/include/gravity
mkdir -p /usr/local/include/gravity/protobuf
cp ./src/api/cpp/protobuf/*.h /usr/local/include/gravity/protobuf
mkdir -p /usr/local/include/gravity/MATLAB
cp ./src/api/MATLAB/*.m /usr/local/include/gravity/MATLAB
cp ./src/api/cpp/libgravity.a /usr/local/lib/gravity/
cp ./src/api/cpp/libgravity.so /usr/local/lib/gravity/
cp ./src/keyvalue_parser/libkeyvalue_parser.a /usr/local/lib/gravity/
cp ./src/keyvalue_parser/libkeyvalue_parser.so /usr/local/lib/gravity/
cp ./src/api/java/*.jar /usr/local/lib/gravity/
cp ./src/api/java/*.so /usr/local/lib/gravity/
cp -r ./src/api/python/src/python/gravity /usr/local/lib/gravity/
cp ./src/api/python/*.so /usr/local/lib/gravity/
mkdir -p /usr/local/lib/gravity/MATLAB
cp ./src/api/MATLAB/*.jar /usr/local/lib/gravity/MATLAB
mkdir -p /usr/local/bin/gravity
cp ./bin/* /usr/local/bin/gravity
