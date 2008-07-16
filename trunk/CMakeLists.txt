cmake_minimum_required(VERSION 2.6)

SET(CMAKE_BUILD_TYPE gdb)
SET(CMAKE_CXX_FLAGS_GDB " -Wall -ggdb -O3")
SET(CMAKE_C_FLAGS_GDB " -Wall -ggdb -O3")

PROJECT (minnow)

INCLUDE (FindDoxygen)

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREAD ON)

#For MinGW+Boost install
IF (WIN32)
   SET(BOOST_ROOT c:/MinGW)
ENDIF (WIN32)

FIND_PACKAGE(Boost 1.35.0 REQUIRED COMPONENTS thread program_options )

IF (DOXYGEN_FOUND)
   MESSAGE(STATUS "Running doxygen...")
   EXECUTE_PROCESS(COMMAND ${DOXYGEN_EXECUTABLE} OUTPUT_QUIET)
ENDIF (DOXYGEN_FOUND)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} src/aquarium src/minnow)
SET (AQUARIUM_SRCS src/aquarium/aquarium.cpp)
SET (MINNOW_SRCS src/minnow/main.cpp src/minnow/parser.cpp src/minnow/codegen_cppoutput.cpp src/minnow/lexer.cpp)

ADD_LIBRARY (aquarium SHARED ${AQUARIUM_SRCS})
TARGET_LINK_LIBRARIES (aquarium debug ${Boost_LIBRARIES} optimized ${Boost_LIBRARIES})

ADD_EXECUTABLE (minnow ${MINNOW_SRCS})
TARGET_LINK_LIBRARIES (minnow debug ${Boost_LIBRARIES} optimized ${Boost_LIBRARIES})

