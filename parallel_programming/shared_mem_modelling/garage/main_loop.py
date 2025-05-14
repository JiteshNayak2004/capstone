import math

# Constants
MAX_SEQ_LENGTH = 5
GAP_SCORE = -1

# Global variables
seq1 = "ABC"  # Sequence 1
seq2 = "ABD"  # Sequence 2
seq1_len = len(seq1)  # Length of seq1
seq2_len = len(seq2)  # Length of seq2
cc = 2  # Number of cores (for testing purposes)
dp = [[0 for _ in range(MAX_SEQ_LENGTH + 1)]
      for _ in range(MAX_SEQ_LENGTH + 1)]  # DP matrix

# Function to initialize the DP matrix


def initialize_dp_matrix():
    for i in range(seq1_len + 1):
        dp[i][0] = i * GAP_SCORE  # Fill first column
    for j in range(seq2_len + 1):
        dp[0][j] = j * GAP_SCORE  # Fill first row


def minimum(x, y):
    return x if x < y else y


def maximum(x, y):
    return x if x > y else y


def diagonal_length_cal(diagonal_index):
    min_dim = minimum(seq1_len, seq2_len)
    max_dim = maximum(seq1_len, seq2_len)

    if diagonal_index <= min_dim:
        num_diagonal_elements = diagonal_index+1
    elif diagonal_index <= max_dim:
        num_diagonal_elements = min_dim
    else:
        num_diagonal_elements = seq1_len + seq2_len + 1 - diagonal_index

    return num_diagonal_elements

# Smith-Waterman function
# Iterates through all diagonals and divides the diagonal elements equally


def smith_waterman(core_id):
    total_diagonals = seq1_len + seq2_len   # Total number of diagonals

    for diagonal_index in range(total_diagonals + 1):
        # Calculate the diagonal length
        num_diagonal_elements = diagonal_length_cal(diagonal_index)

        # Determine how many cores are active
        active_cores = minimum(num_diagonal_elements, cc)
        if (active_cores == 0):
            print(
                f"active cores=0 for cc={cc} and diagonal_index={diagonal_index} combos")
            break

        # Calculate elements per core (use float division and ceiling)
        elements_per_core = math.ceil(num_diagonal_elements / active_cores)

        # Calculate start and end indices for this core
        start = core_id * elements_per_core
        end = num_diagonal_elements if core_id == cc - \
            1 else (core_id + 1) * elements_per_core

        # Process the diagonal segment (for demonstration, just print the indices)
        print(
            f"Core {core_id}, Diagonal {diagonal_index}: Processing from element {start} to element {end}")

# Test function for different core IDs


def test_smith_waterman():
    initialize_dp_matrix()

    for core_id in range(cc):
        print(f"Testing with core ID: {core_id}")
        smith_waterman(core_id)


if __name__ == "__main__":
    test_smith_waterman()
