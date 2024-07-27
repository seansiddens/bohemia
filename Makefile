CXX := g++
SRC := src/main.cpp src/matrix_generator.cpp src/matrix.cpp src/logger.cpp src/complex_plane.cpp
TARGET := bohemia
BUILD_DIR := build
DEBUG_DIR := $(BUILD_DIR)/debug
RELEASE_DIR := $(BUILD_DIR)/release
INCLUDE_DIR := third_party

# Common flags
CXX_FLAGS := -std=c++17 -I$(INCLUDE_DIR)
LFLAGS := -lblas -llapack

# Debug flags
DEBUG_FLAGS := -Wall -DLOG_LEVEL=2  # 2 corresponds to DEBUG level

# Release flags
RELEASE_FLAGS := -O3 -DLOG_LEVEL=1  # 1 corresponds to INFO level

.PHONY: all debug release clean

all: debug release

debug: $(DEBUG_DIR)/$(TARGET)

release: $(RELEASE_DIR)/$(TARGET)

$(DEBUG_DIR)/$(TARGET): $(SRC)
	mkdir -p $(DEBUG_DIR)
	$(CXX) $(CXX_FLAGS) $(DEBUG_FLAGS) $(SRC) $(LFLAGS) -o $@

$(RELEASE_DIR)/$(TARGET): $(SRC)
	mkdir -p $(RELEASE_DIR)
	$(CXX) $(CXX_FLAGS) $(RELEASE_FLAGS) $(SRC) $(LFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR)