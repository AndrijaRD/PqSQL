# Compiler
CXX     := g++
CXXFLAGS:= -fPIC -Wall -Wextra -std=c++20 -O3 -I.

# PostgreSQL client
LDLIBS  := -lpq

SRCS    := PqSQL.cpp
OBJS    := $(SRCS:.cpp=.o)
TARGET  := libPqSQL.so

.PHONY: all clean install

all: $(TARGET)

# Link the shared library
$(TARGET): $(OBJS)
	$(CXX) -shared $(CXXFLAGS) -o $@ $^ $(LDLIBS)

# Compile each .cpp -> .o
%.o: %.cpp PqSQL.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.so

install: $(TARGET)
	@echo "Please run install.sh as root for proper installation"