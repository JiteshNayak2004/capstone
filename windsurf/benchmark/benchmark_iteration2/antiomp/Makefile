CC=gcc
CFLAGS=-O3 -fopenmp -march=native -mtune=native -Wall -msse4.1
TARGET=sw_anti_omp_v2

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)
