cmake_minimum_required(VERSION 2.6)

Find_Package(SDL REQUIRED)

SET (SDL_WRAPPER_SRCS sdl_base.c)

link_libraries ( ${SDL_LIBRARY} SDLmain )
INCLUDE_DIRECTORIES( ${SDL_INCLUDE_DIR} )
ADD_LIBRARY (minnowsdl SHARED ${SDL_WRAPPER_SRCS})

