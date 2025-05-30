cmake_minimum_required(VERSION 3.17)

project(GravityExamples)

add_subdirectory("1-BasicDataProduct")
add_subdirectory("2-ProtobufDataProduct")
add_subdirectory("3-MultipleDataProduct")
add_subdirectory("4-BasicService")
add_subdirectory("5-MiscFunctionality")
add_subdirectory("8-ConfigFile")

if (NOT SKIP_JAVA)
    add_subdirectory("6-JavaProtobufDataProduct")
endif()

if (NOT SKIP_PYTHON)
    add_subdirectory("11-PythonPubSub")
    add_subdirectory("12-PythonBasicService")
endif()
