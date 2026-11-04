CXX      = clang++
CXXFLAGS = -g3 -Wall -Wextra -Wpedantic -Wshadow -std=c++17 \
           -Iinclude -I/usr/local/opt/asio/include
LDFLAGS  = -g3 -pthread

# Build the listener executable
listener: listener.o
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o listener listener.o

main: main.o
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o main main.o

# Build the listener object file
listener.o: listener.cpp
	${CXX} ${CXXFLAGS} -c listener.cpp

# Remove executables, object files, and temporary files