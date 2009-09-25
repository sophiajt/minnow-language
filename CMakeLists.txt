cmake_minimum_required(VERSION 2.6)

SET(CMAKE_BUILD_TYPE gprof)
SET(CMAKE_CXX_FLAGS_GPROF " -Wall -g -pg -O3")
SET(CMAKE_C_FLAGS_GPROF " -Wall -g -pg -O3")

SET(CMAKE_BUILD_TYPE gdb)
SET(CMAKE_CXX_FLAGS_GDB " -Wall -ggdb -O3")
SET(CMAKE_C_FLAGS_GDB " -Wall -ggdb -O3")

#To use the 'make install' uncomment this section
#SET(CMAKE_BUILD_TYPE Release)
#SET(CMAKE_CXX_FLAGS_RELEASE " -Wall -DINSTALL_PREFIX=\\\"${CMAKE_INSTALL_PREFIX}/\\\" -O3")
#SET(CMAKE_C_FLAGS_RELEASE " -Wall -DINSTALL_PREFIX=\\\"${CMAKE_INSTALL_PREFIX}/\\\" -O3")

SET(INCLUDE_TESTS true)

PROJECT (minnow)

INCLUDE (FindDoxygen)

IF (DOXYGEN_FOUND)
   MESSAGE(STATUS "Running doxygen...")
   EXECUTE_PROCESS(COMMAND ${DOXYGEN_EXECUTABLE} "Doxyfile_Aquarium" OUTPUT_QUIET)
ENDIF (DOXYGEN_FOUND)

INCLUDE_DIRECTORIES(aquarium minnow)
SET (AQUARIUM_SRCS aquarium/Aquarium.cpp aquarium/Typeless_Vector.cpp aquarium/Char_String.cpp aquarium/Typeless_Dictionary.cpp aquarium/Object_Feature.cpp aquarium/Scheduler.cpp aquarium/Message_Channel.cpp aquarium/Actor.cpp aquarium/Minnow_Prelude.cpp)
SET (MINNOW_SRCS minnowc/main.cpp minnowc/Var_Scope_Analyzer.cpp minnowc/Type_Analyzer.cpp minnowc/Codegen_C.cpp minnowc/LexParser.cpp minnowc/Program.cpp minnowc/Compiler.cpp)

SET (AQUARIUM_LIBS )
#Platform ident hackery
IF (WIN32)
  IF (MINGW)
    #To use the installer uncomment this section
    #SET(CMAKE_BUILD_TYPE Release)
    #SET(CMAKE_CXX_FLAGS_RELEASE " -Wall -DINSTALL_PREFIX=\"\\\"${CMAKE_INSTALL_PREFIX}/\"\\\" -O3")
    #SET(CMAKE_C_FLAGS_RELEASE " -Wall -DINSTALL_PREFIX=\"\\\"${CMAKE_INSTALL_PREFIX}/\"\\\" -O3")
  ENDIF (MINGW)

  SET (AQUARIUM_SRCS ${AQUARIUM_SRCS} aquarium/Windows/Threading.cpp)
ENDIF (WIN32)

IF (UNIX) 
  IF (APPLE)
    IF (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    SET (AQUARIUM_SRCS ${AQUARIUM_SRCS} aquarium/Mac/Threading.cpp)
    ENDIF (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  ENDIF (APPLE)

  IF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    SET (AQUARIUM_SRCS ${AQUARIUM_SRCS} aquarium/BSD/Threading.cpp)
    SET (AQUARIUM_LIBS ${AQUARIUM_LIBS} pthread)
  ENDIF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

  IF (CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    SET (AQUARIUM_SRCS ${AQUARIUM_SRCS} aquarium/Solaris/Threading.cpp)
    SET (AQUARIUM_LIBS ${AQUARIUM_LIBS} pthread)
    #issues with the C compiler not handling a C++ library.  
    SET(INCLUDE_TESTS false)
  ENDIF (CMAKE_SYSTEM_NAME STREQUAL "SunOS")

  IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    SET (AQUARIUM_SRCS ${AQUARIUM_SRCS} aquarium/Linux/Threading.cpp)
    SET (AQUARIUM_LIBS ${AQUARIUM_LIBS} pthread)
  ENDIF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
ENDIF (UNIX)

#SET (MINNOW_SRCS src/minnow/main.cpp src/minnow/parser.cpp src/minnow/codegen_cppoutput.cpp src/minnow/lexer.cpp)

ADD_LIBRARY (aquarium SHARED ${AQUARIUM_SRCS} )
TARGET_LINK_LIBRARIES (aquarium ${AQUARIUM_LIBS} )

ADD_EXECUTABLE(bin/minnowc ${MINNOW_SRCS} )
TARGET_LINK_LIBRARIES (bin/minnowc aquarium)

INSTALL(TARGETS bin/minnowc aquarium
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
INSTALL(FILES aquarium/Actor.hpp aquarium/Aquarium.hpp aquarium/Char_String.hpp
 aquarium/Common.hpp aquarium/Message.hpp aquarium/Message_Channel.hpp 
 aquarium/Object_Feature.hpp aquarium/Scheduler.hpp aquarium/Typeless_Vector.hpp
 aquarium/Minnow_Prelude.hpp aquarium/Typeless_Dictionary.hpp DESTINATION include/minnow)
INSTALL(FILES prelude.mno DESTINATION share/minnow)

IF(INCLUDE_TESTS) 
  ADD_EXECUTABLE (bin/tests/Test_Threading aquarium/tests/Test_Threading.cpp)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Threading aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Typeless_Vector aquarium/tests/Test_Typeless_Vector.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Typeless_Vector aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Cmd_Line aquarium/tests/Test_Cmd_Line.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Cmd_Line aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Exceptions aquarium/tests/Test_Exceptions.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Exceptions aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Rebalance aquarium/tests/Test_Rebalance.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Rebalance aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Msg_Actor aquarium/tests/Test_Msg_Actor.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Msg_Actor aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Msg_Isolated_Actor aquarium/tests/Test_Msg_Isolated_Actor.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Msg_Isolated_Actor aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Data_Msg_Actor aquarium/tests/Test_Data_Msg_Actor.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Data_Msg_Actor aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Threadring aquarium/tests/Test_Threadring.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Threadring aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Data_Threadring aquarium/tests/Test_Data_Threadring.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Data_Threadring aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Big_Bang aquarium/tests/Test_Big_Bang.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Big_Bang aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Char_String aquarium/tests/Test_Char_String.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Char_String aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Typeless_Dictionary aquarium/tests/Test_Typeless_Dictionary.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Typeless_Dictionary aquarium)

  ADD_EXECUTABLE (bin/tests/Test_Object_Feature aquarium/tests/Test_Object_Feature.c)
  TARGET_LINK_LIBRARIES (bin/tests/Test_Object_Feature aquarium)
ENDIF(INCLUDE_TESTS) 
