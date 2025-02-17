SRC_DIR := source
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))
DEPS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.d, $(SRCS))
TARGET := $(BIN_DIR)/program

CXX := g++
CXXFLAGS := -std=c++23 -Wall -Werror -g
CPPFLAGS := -I$(INC_DIR) -MMD -MP

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CXX) $(OBJS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR) $(BIN_DIR):
	@mkdir -p $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean

