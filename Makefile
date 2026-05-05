CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = prog
SRC      = main.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
