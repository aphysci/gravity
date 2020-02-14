
if (WIN32)

    include(GravityExternalUrls)
    include(FetchContent)
    
    if (NOT PThreadsWin32_DIR)
        FetchContent_Populate(PThreadsWin32 URL ${pthreads_w32_url} SOURCE_DIR "${CMAKE_BINARY_DIR}/pthreads_w32")
        set(PThreadsWin32_DIR "${CMAKE_BINARY_DIR}/pthreads_w32")
        
        find_library(PThreadsWin32Lib NAMES pthreadVC2 PATHS "${PThreadsWin32_DIR}/Pre-built.2/lib/x64")
        find_file(PThreadsWin32Dll NAMES pthreadVC2.dll PATHS "${PThreadsWin32_DIR}/Pre-built.2/dll/x64")

        if (PThreadsWin32Lib AND PThreadsWin32Dll)
            set(PThreadsWin32_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/deps/pthreads-w32")
            file(MAKE_DIRECTORY "${PThreadsWin32_INSTALL_DIR}/bin" "${PThreadsWin32_INSTALL_DIR}/lib")
            file(COPY "${PThreadsWin32Lib}" DESTINATION "${PThreadsWin32_INSTALL_DIR}/lib")
            file(COPY "${PThreadsWin32Dll}" DESTINATION "${PThreadsWin32_INSTALL_DIR}/bin")
            file(COPY "${PThreadsWin32_DIR}/Pre-built.2/include" DESTINATION "${PThreadsWin32_INSTALL_DIR}/")
            set(PThreadsWin32_Include "${PThreadsWin32_INSTALL_DIR}/include")

            get_filename_component(PThreadsWin32LibName "${PThreadsWin32Lib}" NAME)
            get_filename_component(PThreadsWin32DllName "${PThreadsWin32Dll}" NAME)
            set(PThreadsWin32Lib_Installed "${PThreadsWin32_INSTALL_DIR}/lib/${PThreadsWin32LibName}")
            set(PThreadsWin32Dll_Installed "${PThreadsWin32_INSTALL_DIR}/bin/${PThreadsWin32DllName}")
            set(PThreadsWin32_DIR "${PThreadsWin32_INSTALL_DIR}" CACHE PATH "PThreadsWin32 Directory" FORCE)
        endif()
    else()
        find_library(PThreadsWin32Lib_Installed NAMES pthreadVC2 PATHS "${PThreadsWin32_DIR}/lib")
        find_file(PThreadsWin32Dll_Installed NAMES pthreadVC2.dll PATHS "${PThreadsWin32_DIR}/bin")
        get_filename_component(PThreadsWin32LibName "${PThreadsWin32Lib_Installed}" NAME)
        get_filename_component(PThreadsWin32DllName "${PThreadsWin32Dll_Installed}" NAME)
        find_file(PThreadsWin32_Include_File NAMES pthread.h PATHS "${PThreadsWin32_DIR}/include")
        get_filename_component(PThreadsWin32_Include "${PThreadsWin32_Include_File}" DIRECTORY)
    endif()

    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(GravityJava REQUIRED_VARS PThreadsWin32Lib_Installed PThreadsWin32Dll_Installed PThreadsWin32_Include PThreadsWin32LibName PThreadsWin32DllName)
endif()