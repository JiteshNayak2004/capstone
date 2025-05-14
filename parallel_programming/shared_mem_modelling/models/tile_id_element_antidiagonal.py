import math
from collections import defaultdict


def assign_tile(i, j, rows, cols):

    sequential_index = i*rows+j+1
    # Compute the tile as floor(product / 7168)
    tile = math.floor(sequential_index / 7168)
    return tile


def count_tiles_in_antidiagonals(rows, cols):
    # Dictionary to store tile counts for each anti-diagonal
    antidiagonal_tile_counts = defaultdict(lambda: defaultdict(int))

    for i in range(rows):
        for j in range(cols):
            tile = assign_tile(i, j, rows, cols)
            antidiag_index = i + j  # Anti-diagonal index where i + j is constant
            # Increment the count for the tile in this anti-diagonal
            antidiagonal_tile_counts[antidiag_index][tile] += 1

    # Print the counts for each anti-diagonal
    for antidiag, tile_counts in antidiagonal_tile_counts.items():
        print(f"Anti-diagonal {antidiag}: ", end="")
        for tile, count in sorted(tile_counts.items()):
            print(f"Tile {tile}: {count}", end="  ")
        print()


# Example: Set matrix size (e.g., 20x20 for testing)
count_tiles_in_antidiagonals(100, 100)
