INCLUDE_DIRECTORIES (/home/william/build/include ../include)

ADD_LIBRARY(ketty SHARED
			logger.c ketty.c
			connection.c request.c response.c
			post_processor.c static_file.c
			../simclist-1.5/simclist.c)

## Compiler flags
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "-Wall -g")
#	set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary
endif()
