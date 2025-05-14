match_score = 2
mismatch_score = 1
gap_penalty = -1


def compute_diagonal_elements(matrix, diagonal_idx, start, end, seq1, seq2):
    rows = len(matrix)
    cols = len(matrix[0])

    # For diagonal k, elements are at positions where i + j = k + 1
    diagonal_elements = []

    # Calculate how many elements are in this diagonal
    if diagonal_idx <= cols:
        # Diagonals starting from top row
        max_elements = min(diagonal_idx, rows)
        start_col = diagonal_idx - 1
        start_row = 0
    else:
        # Diagonals starting from rightmost column
        max_elements = min(rows - (diagonal_idx - cols), rows)
        start_col = cols - 1
        start_row = diagonal_idx - cols

    # Validate start and end parameters
    if start < 0 or end > max_elements or start >= end:
        raise ValueError("Invalid start/end indices")

    # Compute only the requested portion of the diagonal
    for i in range(start, end):
        current_row = start_row + i
        current_col = start_col - i

        # Get the match/mismatch score
        if seq1[current_row - 1] == seq2[current_col - 1]:
            match = match_score
        else:
            match = mismatch_score

        # Calculate the score using Smith-Waterman rules
        diagonal_score = matrix[current_row - 1][current_col - 1] + match
        left_score = matrix[current_row][current_col - 1] + gap_penalty
        up_score = matrix[current_row - 1][current_col] + gap_penalty

        # Take the maximum score, with 0 as the minimum
        matrix[current_row][current_col] = max(
            0, diagonal_score, left_score, up_score)

    return matrix


def test_compute_diagonal_elements():
    # Test case 1: 4x4 matrix
    matrix = [
        [0, 0, 0, 0],
        [0, 0, 0, 0],
        [0, 0, 0, 0],
        [0, 0, 0, 0]
    ]
    seq1 = "ACGT"
    seq2 = "AGCT"

    print("Initial matrix:")
    print_matrix(matrix)

    # Test diagonal 3 (should compute 'c', 'f', 'i')
    print("\nTesting diagonal 3 (start=0, end=1):")
    matrix = compute_diagonal_elements(matrix, 3, 0, 1, seq1, seq2)
    print("After computing first element of diagonal 3:")
    print_matrix(matrix)

    # Test diagonal 2 (should compute 'b', 'e')
    print("\nTesting diagonal 2 (start=0, end=2):")
    matrix = compute_diagonal_elements(matrix, 2, 0, 2, seq1, seq2)
    print("After computing elements of diagonal 2:")
    print_matrix(matrix)


def print_matrix(matrix):
    """Helper function to print matrix in a readable format"""
    for row in matrix:
        print([f"{x:2d}" for x in row])


# Run the test
if __name__ == "__main__":
    test_compute_diagonal_elements()
