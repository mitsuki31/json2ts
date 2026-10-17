EXEC_FILE := json2ts
BUILD_DIR := build
TEST_DIR := test

.PHONY: all build configure build-test clean test configure-test

all: build

# ===============================
# >>> BUILD
# ===============================

build:
	$(info Building the project using CMake...)
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) --parallel

configure:
	$(info Configuring the project...)
	cmake -S . -B $(BUILD_DIR)


# ===============================
# >>> TEST
# ===============================

EXEC_FILE := $(BUILD_DIR)/$(EXEC_FILE)
TEST_JSON_FILES := $(wildcard $(TEST_DIR)/cases/*.json)
TEST_TS_FILES := $(wildcard $(TEST_DIR)/expect/*.ts)
TEST_BUILDER := $(TEST_DIR)/build_test.sh
TEST_RUNNER := $(TEST_DIR)/runner.ts
TSX := $(TEST_DIR)/node_modules/.bin/tsx

build-test: $(EXEC_FILE)
	$(info Converting JSON files into TypeScript...)
	$(TEST_BUILDER) --bin $< $(TEST_JSON_FILES)

configure-test:
ifeq ($(wildcard $(TSX)),)
	$(info Configuring project's test...)
	cd $(TEST_DIR) && npm install
	$(TSX) --version
else
	$(info Nothing to configure.)
	$(TSX) --version
	@:
endif

test:
ifeq ($(wildcard $(TSX)),)
	$(error Please run `make configure-test` first!)
endif
ifeq ($(TEST_TS_FILES),)
	$(error Please run `make build-test` first!)
endif

	$(TSX) $(TEST_RUNNER)
	@echo -e "\nAll tests ran."

# ===============================
# >>> CLEAN
# ===============================

clean:
	$(info Cleaning the project...)
	rm -rfv $(BUILD_DIR) $(TEST_DIR)/expect $(TEST_DIR)/expect_classes
	@echo 'Clean completed! Run `make configure` to reconfigure the project'
