
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2

TARGET = temperature_collector
SRC = src/temperature_collector.cpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
