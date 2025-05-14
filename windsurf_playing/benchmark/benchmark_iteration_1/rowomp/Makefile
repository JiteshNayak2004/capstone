CC=gcc
CFLAGS=-fopenmp -O3 -Wall
TARGET=rowparallel_omp

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

.PHONY: clean
clean:
	rm -f $(TARGET)
