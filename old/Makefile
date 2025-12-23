# Compiler
CXX     := g++
CXXFLAGS:= -fPIC -Wall -Wextra -std=c++20 -O3

# PostgreSQL client
LDLIBS  := -shared -lpq

SRC_DIR := lib
BUILD   := build
SRCS    := $(wildcard $(SRC_DIR)/*.cpp)
OBJS    := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD)/%.o,$(SRCS))
TARGET  := libPqSQL.so

.PHONY: all clean

all: $(TARGET)

# Link the shared library
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

# Compile each .cpp → .o, ensuring the build directory exists
$(BUILD)/%.o: $(SRC_DIR)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create the build directory
$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD) $(TARGET)
