PREFIX ?= /usr/local
SANITIZE ?= 0

# CC = clang
# CXX = clang++

CC = gcc
CXX = g++

BOTH_FLAGS = -Wall -Wno-unused-function -Wno-unknown-pragmas -Wno-unused-const-variable -O2 -flto -ggdb -Iinclude/
LDFLAGS += -flto -ggdb

ifeq ($(SANITIZE), 1)
  BOTH_FLAGS += -fsanitize=address,undefined,leak -fno-omit-frame-pointer
  LDFLAGS += -fsanitize=address,undefined,leak
endif

CFLAGS += $(BOTH_FLAGS) -std=c11
CXXFLAGS += $(BOTH_FLAGS) -std=c++20

OBJS = bin/bang_bang_controller.o bin/command_handler.o bin/daq.o bin/data_writer.o bin/main.o bin/queue.o bin/state_writer.o
FULL_OBJS = $(OBJS) bin/ads1263.o bin/hwif.o bin/gpio.o
SITL_OBJS = $(OBJS) bin/harness.o

# Create a proper set with no duplicates
ALL_OBJS = $(sort $(FULL_OBJS) bin/harness.o)

.PHONY: all sitl clean install

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

install: bin/fsw
	mkdir -p $(PREFIX)/bin
	install -Dm755 bin/fsw $(PREFIX)/bin/fsw
	install -Dm755 tools/fsw_telemetry_server $(PREFIX)/bin/fsw_telemetry_server
