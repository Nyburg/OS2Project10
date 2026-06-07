CC = gcc
CFLAGS = -Wall -Wextra -Werror
AR = ar
ARFLAGS = rcs

LIB = libvvsfs.a
TARGET = testfs

LIB_OBJS = image.o block.o free.o inode.o mkfs.o pack.o dir.o ls.o
TEST_OBJS = testfs.o

.PHONY: all test clean pristine

all: $(TARGET)

$(TARGET): $(TEST_OBJS) $(LIB)
	$(CC) $(CFLAGS) -o $(TARGET) $(TEST_OBJS) $(LIB)

$(LIB): $(LIB_OBJS)
	$(AR) $(ARFLAGS) $(LIB) $(LIB_OBJS)

image.o: image.c image.h
	$(CC) $(CFLAGS) -c image.c

block.o: block.c block.h image.h
	$(CC) $(CFLAGS) -c block.c

free.o: free.c free.h block.h inode.h
	$(CC) $(CFLAGS) -c free.c

inode.o: inode.c inode.h block.h pack.h
	$(CC) $(CFLAGS) -c inode.c

mkfs.o: mkfs.c mkfs.h block.h free.h
	$(CC) $(CFLAGS) -c mkfs.c

pack.o: pack.c pack.h
	$(CC) $(CFLAGS) -c pack.c

testfs.o: testfs.c image.h block.h free.h inode.h mkfs.h dir.h ctest.h
	$(CC) $(CFLAGS) -c testfs.c

dir.o: dir.c dir.h block.h inode.h pack.h
	$(CC) $(CFLAGS) -c dir.c

ls.o: ls.c ls.h dir.h
	$(CC) $(CFLAGS) -c ls.c

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f *.o

pristine: clean
	rm -f $(TARGET) *.a testfs.img