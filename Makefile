SANITIZE ?= 0

# CC = clang
# CXX = clang++

CC = gcc
CXX = g++

IGNORED_WARNINGS = -Wno-unused-function -Wno-unknown-pragmas -Wno-unused-const-variable
CFLAGS = -Wall $(IGNORED_WARNINGS) -std=c11 -O2 -flto -Iinclude/ -ggdb
CXXFLAGS = -Wall $(IGNORED_WARNINGS) -std=c++20 -O2 -flto -Iinclude/ -ggdb
LDFLAGS = -flto -ggdb

ifeq ($(SANITIZE), 1)
  CFLAGS += -fsanitize=address,undefined,leak -fno-omit-frame-pointer -g
  CXXFLAGS += -fsanitize=address,undefined,leak -fno-omit-frame-pointer -g
  LDFLAGS += -fsanitize=address,undefined,leak -g
endif

OBJS = bin/bang_bang_controller.o bin/command_handler.o bin/daq.o bin/data_writer.o bin/main.o bin/queue.o
FULL_OBJS = $(OBJS) bin/ads1263.o bin/hwif.o bin/gpio.o
SITL_OBJS = $(OBJS) bin/harness.o

# Create a proper set with no duplicates
ALL_OBJS = $(sort $(FULL_OBJS) bin/harness.o)

.PHONY: all sitl clean

all: bin/fsw
sitl: bin/fsw_sitl

bin/fsw: $(FULL_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ -lgpiod

bin/fsw_sitl: $(SITL_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	rm -rf bin

DEPS = $(ALL_OBJS:.o=.d)
-include $(DEPS)

bin/%.o: src/%.c | bin
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

bin/%.o: src/%.cpp | bin
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

bin/harness.o: sitl_harness/harness.cpp | bin
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

bin:
	mkdir -p bin