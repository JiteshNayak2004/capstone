CC=gcc
CFLAGS=-fopenmp -O3 -Wall
TARGET=sw_serial_omp

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

.PHONY: clean
clean:
	rm -f $(TARGET)
