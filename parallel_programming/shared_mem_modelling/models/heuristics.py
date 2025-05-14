import math


def assign_tile(i, j, rows, cols):

    sequential_index = i*rows+j+1
    # Compute the tile as floor(product / 7168)
    tile = math.floor(sequential_index / 7168)
    return tile


m = 100
n = 100
found = 0


def compute_diagonal_segment(diagonal_index):
    for i in range(diagonal_index, -1, -1):  # Iterate over rows
        for j in range(diagonal_index + 1):  # Iterate over columns
            if (i + j == diagonal_index) and (i < m) and (j < n):
                tile = assign_tile(i, j, m, n)
                if (tile > 0):
                    print(f"found at {i} {j}  indices")
                    return


total_diagonal = m+n-1
for i in range(0, total_diagonal):

    if (found == 0):
        compute_diagonal_segment(i)
