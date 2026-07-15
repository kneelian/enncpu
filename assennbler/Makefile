CXX := g++
CXXFLAGS := -std=c++20 -Og -g
TARGET := asm
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

break_line.o: break_line.cpp utility.hpp
	$(CXX) $(CXXFLAGS) -c break_line.cpp -o break_line.o

parse_lne.o: parse_line.cpp utility.hpp
	$(CXX) $(CXXFLAGS) -c parse_line.cpp -o parse_line.o

main.o: main2.cpp utility.hpp operations.hpp types.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

mappings.o: mappings.cpp operations.hpp
	$(CXX) $(CXXFLAGS) -c mappings.cpp -o mappings.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
