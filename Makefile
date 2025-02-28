CFLAGS = -std=c11 -O2
OBJS = bin/ads1263.o bin/hwif.o

.PHONY: all clean
all: $(OBJS)

clean:
	rm -rf bin

bin/%.o: src/%.c bin
	gcc $(CFLAGS) -c $< -o $@

bin:
	mkdir -p bin