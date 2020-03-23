#
#Gravity dependencies download locations
#

## Protobuf version requirements
# Versions greater than 3.5.1 check for a __cplusplus value that's incorrect and too low in VS2013.  So this is the latest protobuf version that will still build with 2013.
# A Cmake build is required so the minimum version you can use is 3.4.1
set(protobuf_url "https://github.com/protocolbuffers/protobuf/archive/v3.5.1.zip")


## ZeroMQ version requirements
# The minimum version you can use externally is 4.2.0
set(libzmq_url "https://github.com/zeromq/libzmq/archive/v4.3.2.zip")

set(boost_url "https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.zip")

set(swigwin_url "http://prdownloads.sourceforge.net/swig/swigwin-4.0.1.zip")

set(lexyacc_win_url "https://sourceforge.net/projects/winflexbison/files/win_flex_bison-2.5.22.zip")

set(pthreads_w32_url "https://sourceforge.net/projects/pthreads4w/files/pthreads-w32-2-9-1-release.zip")


