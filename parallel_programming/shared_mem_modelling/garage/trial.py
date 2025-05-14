seq1 = "ABC"  # Sequence 1
seq2 = "ABD"  # Sequence 2
seq1_len = len(seq1)  # Length of seq1
seq2_len = len(seq2)  # Length of seq2


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


index = 2
num_elem = diagonal_length_cal(index)

print(f"the diagonal length for index {index} is {num_elem}")
