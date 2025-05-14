CC=gcc
CFLAGS=-O3 -fopenmp -march=native -mtune=native -Wall
TARGET=rowparallel_omp_v2

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)
