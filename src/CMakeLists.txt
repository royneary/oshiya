# src/CMakeLists.txt

include(CheckIncludeFileCXX)

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

# Expat
find_package(EXPAT)
if(NOT EXPAT_FOUND)
	include(ExternalProject)
	ExternalProject_Add(expat_static
		SOURCE_DIR ${THIRD_PARTY_DIR}/expat
		CONFIGURE_COMMAND ${THIRD_PARTY_DIR}/expat/configure
		--prefix=${CMAKE_BINARY_DIR}
		BUILD_COMMAND ${MAKE})
	add_library(expat STATIC IMPORTED)
	set_property(TARGET expat PROPERTY
		IMPORTED_LOCATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libexpat.a)
	add_dependencies(expat expat_static)
	#add_subdirectory( ${THIRD_PARTY_DIR}/expat )
	#set( NINJITSU_INCLUDE_DIRS ${NINJITSU_INCLUDE_DIRS}
	#	${THIRS_PARTY_DIR}/expat ${THIRS_PARTY_DIR}/expat/lib
	#)
	set( LIBS ${LIBS} expat )
else()
	set( LIBS ${LIBS} ${EXPAT_LIBRARIES} )
endif()

# libstrophe
set(BUILD_EXAMPLES OFF CACHE BOOL "disable strophe examples")
add_subdirectory(${THIRD_PARTY_DIR}/libstrophe)
set(OSHIYA_INCLUDE_DIRS ${OSHIYA_INCLUDE_DIRS} ${THIRD_PARTY_DIR}/libstrophe)
set(LIBS ${LIBS} strophe)

# yaml-cpp
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "disable yaml-cpp tests")
add_subdirectory(${THIRD_PARTY_DIR}/yaml-cpp)
set(OSHIYA_INCLUDE_DIRS ${OSHIYA_INCLUDE_DIRS}
    ${THIRD_PARTY_DIR}/yaml-cpp/include)
set(LIBS ${LIBS} yaml-cpp)

# jsoncpp
find_package(JsonCpp)
if(NOT JSONCPP_FOUND)
    set(JSONCPP_WITH_TESTS OFF CACHE BOOL "disable jsoncpp tests")
    add_subdirectory(${THIRD_PARTY_DIR}/jsoncpp)
    set(LIBS ${LIBS} jsoncpp_lib_static)
else()
    set(OSHIYA_INCLUDE_DIRS ${OSHIYA_INCLUDE_DIRS} ${JSONCPP_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${JSONCPP_LIBRARIES})
endif()

# curlcpp
add_subdirectory(${THIRD_PARTY_DIR}/curlcpp)
set(OSHIYA_INCLUDE_DIRS ${OSHIYA_INCLUDE_DIRS}
    ${THIRD_PARTY_DIR}/curlcpp/include
)
set(LIBS ${LIBS} curlcpp)

# pthread
set(LIBS ${LIBS} pthread)

#set(CMAKE_CXX_FLAGS "-O2 -Os -std=c++11 -Wall -Wextra -pedantic -lstdc++" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "-O2 -std=c++11 -Wall -Wextra -pedantic")
set(CMAKE_EXE_LINKER_FLAGS "")

set(source_files
    Component.cpp
    Config.cpp
    Backend.cpp
    Base64.cpp
    ApnsBackend.cpp
    GcmBackend.cpp
    UbuntuBackend.cpp
    Registration.cpp
    AppServer.cpp
    XData.cpp
    UriCodec.cpp
    XmppUtils.cpp
    XmlElement.cpp
    OutPacket.cpp
    RNG.cpp
    StanzaDispatcher.cpp
    Oshiya.cpp
)

add_executable(oshiya ${source_files})
target_include_directories(oshiya PUBLIC
                           ${OSHIYA_INCLUDE_DIRS}
                           ${PROJECT_SOURCE_DIR} # for config.h
                           ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(oshiya ${LIBS})

