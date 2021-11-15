TARGET_EXEC ?= game

LIBS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -DPLATFORM_DESKTOP
BUILD_DIR ?= ./build
SRC_DIRS ?= ./src
CC := g++

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
CPPFLAGS ?= $(INC_FLAGS) -MMD -g -std=c++17
DEPS := $(OBJS:.o=.d)

ifeq ($(OS),Windows_NT)
    CPPFLAGS += -I C:\raylib\raylib\src
    LIBS := -L C:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm -DPLATFORM_DESKTOP
endif

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@ $(LDFLAGS)

-include $(DEPS)
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


run-linux: $(BUILD_DIR)/$(TARGET_EXEC)
	./$(BUILD_DIR)/$(TARGET_EXEC)

$(TARGET_EXEC): $(BUILD_DIR)/$(TARGET_EXEC)
	cp $< $@

.PHONY: clean run-linux

clean:
	$(RM) -r $(BUILD_DIR)

MKDIR_P ?= mkdir -p
