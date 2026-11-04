#########################################################
#                PI CONNECT LISTENER                    #
#########################################################

CXX = g++
ASIO_INCLUDE = $(shell if [ -d /usr/local/opt/asio/include ]; then echo /usr/local/opt/asio/include; else echo /usr/include; fi)
CXXFLAGS = -g3 -Wall -Wextra -Wpedantic -Wshadow -std=c++17 -pthread \
           -Iinclude -I$(ASIO_INCLUDE)
LDFLAGS = -g3 -pthread

# Build the listener executable
all: listener

listener: listener.o
	${CXX} ${LDFLAGS} -o listener listener.o

# Build the listener object file
listener.o: listener.cpp
	${CXX} ${CXXFLAGS} -c listener.cpp

# Remove executables and object files
clean:
	rm -f listener *.o *~

.PHONY: all clean