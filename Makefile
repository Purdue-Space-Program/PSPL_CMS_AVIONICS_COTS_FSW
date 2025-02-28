SANITIZE ?= 0

IGNORED_WARNINGS = -Wno-unused-function -Wno-unknown-pragmas -Wno-unused-const-variable
CFLAGS = -Wall $(IGNORED_WARNINGS) -std=c11 -O2 -flto
CXXFLAGS = -Wall $(IGNORED_WARNINGS) -std=c++17 -O2 -flto
LDFLAGS = -flto

ifeq ($(SANITIZE), 1)
  CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -g
  CXXFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -g
  LDFLAGS += -fsanitize=address,undefined
endif

C_OBJS = bin/ads1263.o bin/hwif.o
CXX_OBJS = bin/fsw.o

OBJS = $(C_OBJS) $(CXX_OBJS)

.PHONY: all clean
all: bin/fsw

bin/fsw: $(OBJS)
	g++ $(LDFLAGS) -o $@ $^ -lgpiod

clean:
	rm -rf bin

bin/%.o: src/%.c bin
	gcc $(CFLAGS) -c $< -o $@

bin/%.o: src/%.cc bin
	g++ $(CXXFLAGS) -c $< -o $@

bin:
	mkdir -p bin