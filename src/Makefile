
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2

TARGET = temp_collector
SRC = src/temp_collector.cpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
