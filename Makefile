BUILD_DIR := build
CMAKE_FLAGS := -DCMAKE_OSX_ARCHITECTURES=arm64

.PHONY: all configure build test clean

all: build

configure:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)

build: configure
	cmake --build $(BUILD_DIR)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	cmake --build $(BUILD_DIR) --target clean 2>/dev/null || true
