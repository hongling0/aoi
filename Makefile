CC=gcc
CFLAG=-std=gnu99 -O3 -Wall -Wextra -g

SOURCE:=$(wildcard *.c)
TARGET:=./test

all:$(TARGET)

$(TARGET):$(SOURCE)
	$(CC) $(CFLAG) $^ -o $@ 

clean:
	rm $(TARGET)
