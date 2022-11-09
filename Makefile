CC := clang
CFLAGS := -mtune=native -O3 -Wall
LDLIBS := -lpthread

taxi: taxi.o

clean:
	rm -f taxi.o taxi
