cmake_minimum_required(VERSION 2.6)

Find_Package(SDL REQUIRED)

SET(CMAKE_BUILD_TYPE gdb)
SET(CMAKE_CXX_FLAGS_GDB " -Wall -ggdb -O3")
SET(CMAKE_C_FLAGS_GDB " -Wall -ggdb -O3")

SET (SDL_WRAPPER_SRCS sdl_base.c)

IF (APPLE)
  link_libraries ( ${SDL_LIBRARY} )
  SET(SDL_WRAPPER_SRCS sdl_base_os_x.c SDLMain.m)
ELSE (APPLE)
  link_libraries ( ${SDL_LIBRARY} SDLmain )
ENDIF (APPLE)
INCLUDE_DIRECTORIES( ${SDL_INCLUDE_DIR} )
ADD_LIBRARY (minnowsdl SHARED ${SDL_WRAPPER_SRCS})

