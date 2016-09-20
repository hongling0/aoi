CC=gcc
CFLAG=-std=gnu99 -O3 -Wall -Wextra -g

TARGET:=test
LIB:=laoi.so

all:$(TARGET) $(LIB)

$(TARGET):aoi.c aoi_test.c
	$(CC) $(CFLAG) $^ -o $@ 

$(LIB):aoi.c laoi.c
	$(CC) $(CFLAG) -fPIC --shared $^ -o $@
clean:
	rm $(TARGET) $(LIB)
