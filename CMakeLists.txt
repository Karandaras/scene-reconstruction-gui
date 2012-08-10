cmake_minimum_required (VERSION 2.6)
include(UsePkgConfig)
project (GUI)


PKGCONFIG(gtkmm-3.0 GTK_INCLUDE_DIR GTK_LINK_DIR GTK_LINK_FLAGS
                   GTK_CFLAGS)
set(CMAKE_CXX_FLAGS " -I/usr/local/include/gazebo-1.0.1/gazebo -g -Wextra -Wall -Wno-long-long -pedantic-errors -std=c++98 ${GTK_CFLAGS} ${GTK_LINK_FLAGS} -lgazebo -lgazebo_common -lgazebo_transport -lgazebo_msgs -lgazebo_math -lboost_system-mt -lprotobuf" CACHE INTERNAL "General CXX Flags")

add_executable(GUI src/scenegui.cpp src/scenetab.cpp src/loggertab.cpp src/controltab.cpp src/modeltab.cpp src/robotcontrollertab.cpp)
