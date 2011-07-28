INCLUDE_DIRECTORIES (/home/william/build/include ../../include)
LINK_DIRECTORIES (/home/william/build/lib)

ADD_EXECUTABLE (ketty-web main.c)
TARGET_LINK_LIBRARIES (ketty-web microhttpd ketty)

## Compiler flags
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "-Wall -g")
#	set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary
endif()
