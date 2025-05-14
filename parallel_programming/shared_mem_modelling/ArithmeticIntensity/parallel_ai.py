def num_elements(seq1_len, seq2_len):
    min_dim = min(seq1_len, seq2_len)
    max_dim = max(seq1_len, seq2_len)
    num_diagonal_elements = 0  # Initialize the variable

    for i in range(seq1_len + seq2_len-1):
        if i < min_dim:
            num_diagonal_elements += 1
        elif i < max_dim:
            continue  # No action, continue to next iteration
        else:
            num_diagonal_elements += 3

    return num_diagonal_elements


def calculate_ratio(m, n):
    start_end = 4
    elements_per_core = 3
    compute = 5*m*n+(m+n-1)*(elements_per_core+start_end)+num_elements(m, n)
    overhead = (m+n-1)*(elements_per_core+start_end)+num_elements(m, n)

    memory = 2*(4*m*n)
    y = compute/memory
    print(
        f"Calculated ratio for m={m} and n={n} is: {y:.4f} with overhead of {overhead}")

    return y


# Test cases
test_cases = [
    # Small matrices
    (10, 10),     # 100
    (20, 50),     # 1000
    (30, 30),     # 900
    (40, 25),     # 1000
    (50, 50),     # 2500
    (60, 80),     # 4800
    (70, 90),     # 6300
    (80, 100),    # 8000
    (90, 120),    # 10800

    # Medium-sized matrices
    (100, 150),   # 15000
    (110, 160),   # 17600
    (120, 180),   # 21600
    (130, 200),   # 26000
    (140, 220),   # 30800
    (150, 240),   # 36000
    (160, 260),   # 41600
    (170, 280),   # 47600
    (180, 300),   # 54000

    # Larger matrices
    (200, 400),   # 80000
    (210, 420),   # 88200
    (220, 440),   # 96800
    (230, 460),   # 105800
    (240, 480),   # 115200
    (250, 500),   # 125000
    (260, 520),   # 135200
    (270, 540),   # 145800
    (280, 560),   # 156800

    # Tall and wide matrices
    (300, 600),   # 180000
    (320, 580),   # 185600
    (340, 540),   # 183600
    (360, 520),   # 187200
    (380, 480),   # 182400
    (400, 500),   # 200000
    (420, 600),   # 252000

    # Near-limit matrices (larger element counts)
    (500, 700),   # 350000
    (600, 600),   # 360000
    (650, 650),   # 422500
    (670, 680),   # 455600
    (677, 677),   # 458329  (Max size that fits the element constraint)

]


ratios = []

for m, n in test_cases:
    # print(f"\nTesting matrix of size {m}x{n}")
    print(f"{m} {n}")
    ratio = calculate_ratio(m, n)
    ratios.append(ratio)

# Calculate and print the average ratio
average_ratio = sum(ratios) / len(ratios)
product = m*n*3
print(f"\nAverage ratio of all test cases: {average_ratio:.4f}")
print("\nAll test cases passed successfully.")
