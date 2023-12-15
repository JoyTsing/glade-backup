cmake_minimum_required(VERSION 3.17)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_C_FLAGS_RELEASE -mwindows)
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTKMM REQUIRED gtkmm-3.0)
#PKG_CHECK_MODULES(GMODULE REQUIRED gmodule-export-2.0)

INCLUDE_DIRECTORIES(${GTKMM_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTKMM_LIBRARY_DIRS})

ADD_DEFINITIONS(${GTKMM_CFLAGS_OTHER})

include_directories("include")

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
foreach(file ${files})
    get_filename_component(name ${file} NAME)
    add_executable(${name} ${file} main.cpp utils/NamespaceDecl.h utils/Win32Util.h utils/Win32Util.cpp pack/Pack.h pack/Pack.cpp compress/Compress.h compress/HuffmanTree.h common/DataTypes.h common/Platform.h common/Optional.h common/File.h common/File.cpp common/BitStream.h common/BitStream.cpp aes/AESMatrices.h aes/AES.h aes/AES.cpp common/Stream.cpp common/Stream.h Backup.h Backup.cpp ui/MainWindow.cpp ui/MainWindow.h utils/Timer.cpp utils/Timer.h)

    target_link_libraries(${name} ${GTKMM_LIBRARIES})
#    target_link_libraries(${name} ${GMODULE_LIBRARIES})
endforeach()