CXX := g++
CXXFLAGS := -std=c++20 -Og -g -I../include
LINKFLAGS :=  -L..
TARGET := cpu
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LINKFLAGS)

cpu_syscalls.o: cpu_syscalls.cpp types.hpp
	$(CXX) $(CXXFLAGS) -c cpu_syscalls.cpp -o cpu_syscalls.o

cpu_funcs.o: cpu_funcs.cpp types.hpp
	$(CXX) $(CXXFLAGS) -c cpu_funcs.cpp -o cpu_funcs.o

mmu_funcs.o: mmu_funcs.cpp types.hpp
	$(CXX) $(CXXFLAGS) -c mmu_funcs.cpp -o mmu_funcs.o

decoder.o: decoder.cpp types.hpp op_tables.hpp
	$(CXX) $(CXXFLAGS) -c decoder.cpp -o decoder.o

executer.o: executer.cpp types.hpp
	$(CXX) $(CXXFLAGS) -c executer.cpp -o executer.o

random_module.o: random_module.cpp random_module.hpp
	$(CXX) $(CXXFLAGS) -c random_module.cpp -o random_module.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LINKFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)
