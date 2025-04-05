CC = gcc
CFLAGS = -Wall -Wextra -Werror -g 
TARGET = transpiler

.PHONY: all clean

all: $(TARGET)

$(TARGET): main.o
	$(CC) $(CFLAGS) -o ../$@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f ../$(TARGET) *.o
