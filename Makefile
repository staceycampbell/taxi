CFLAGS := -mtune=native -Os -Wall
LDLIBS := -lpthread

taxi: taxi.o

clean:
	rm -f taxi.o taxi
