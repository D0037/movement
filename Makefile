BUILD_DIR := build
SRC_DIR := src
BIN_DIR := bin
EXECUTABLE := main
CXXFLAGS := -O3 --std=c++20 -Wall -Wextra -Wno-literal-suffix -Iinclude 
CXX := toolchain/bin/arm-unknown-linux-gnueabi-c++
SRCS := $(shell find $(SRC_DIR) -name 'main.cpp')

OBJS := $(addsuffix .o,$(addprefix $(BUILD_DIR)/, $(notdir $(SRCS))))

$(BIN_DIR)/$(EXECUTABLE): $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $<

$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp 
	mkdir -p $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) -o $@ $< 

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)/$(EXECUTABLE)

.PHONY: main
main:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)/$(EXECUTABLE)
	mkdir -p $(BIN_DIR)
	mkdir -p $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) -o build/main.cpp.o src/main.cpp
	$(CXX) -o bin/main build/main.cpp.o


.PHONY: test
test:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)/$(EXECUTABLE)
	mkdir -p $(BIN_DIR)
	mkdir -p $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) -o build/main.cpp.o src/test.cpp
	$(CXX) -o bin/main build/main.cpp.o

.PHONY: start
start:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)/$(EXECUTABLE)
	mkdir -p $(BIN_DIR)
	mkdir -p $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) -o build/main.cpp.o src/start.cpp
	$(CXX) -o bin/start build/main.cpp.o

.PHONY: 2nd
2nd:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)/$(EXECUTABLE)
	mkdir -p $(BIN_DIR)
	mkdir -p $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) -o build/main.cpp.o src/2nd.cpp
	$(CXX) -o bin/start build/main.cpp.o