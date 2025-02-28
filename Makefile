IGNORED_WARNINGS = -Wno-unused-function -Wno-unknown-pragmas -Wno-unused-const-variable
CFLAGS = -Wall $(IGNORED_WARNINGS) -std=c11 -O2
CXXFLAGS = -Wall $(IGNORED_WARNINGS) -std=c++17 -O2
C_OBJS = bin/ads1263.o bin/hwif.o
CXX_OBJS = bin/fsw.o

OBJS = $(C_OBJS) $(CXX_OBJS)

.PHONY: all clean
all: bin/fsw

bin/fsw: $(OBJS)
	g++ $(CXXFLAGS) -o $@ $^ -lgpiod

clean:
	rm -rf bin

bin/%.o: src/%.c bin
	gcc $(CFLAGS) -c $< -o $@

bin/%.o: src/%.cc bin
	g++ $(CXXFLAGS) -c $< -o $@

bin:
	mkdir -p bin