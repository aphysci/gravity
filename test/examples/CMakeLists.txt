cmake_minimum_required(VERSION 3.17)

project(GravityExamples)
#Examples 2,9 and 13 all use source files with the same name, so only one can be built at a time
add_subdirectory("1-BasicDataProduct")
add_subdirectory("2-ProtobufDataProduct")
add_subdirectory("3-MultipleDataProduct")
add_subdirectory("4-BasicService")
add_subdirectory("5-MiscFunctionality")
add_subdirectory("8-ConfigFile")
add_subdirectory("9-Domains")
add_subdirectory("10-Archiving")
add_subdirectory("13-Relay")

if (NOT SKIP_JAVA)
    add_subdirectory("6-JavaProtobufDataProduct")
    add_subdirectory("7-JavaBasicService")


endif()

if (NOT SKIP_PYTHON)
    add_subdirectory("11-PythonPubSub")
    add_subdirectory("12-PythonBasicService")
    add_subdirectory("14-AnomalyDetection")
endif()
