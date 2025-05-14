import pandas as pd


def calculate_ratio(m, n):
    min_dim = min(m, n)
    max_dim = max(m, n)
    sum_loads = 0

    for k in range(1, m + n):
        if k <= min_dim:
            x = k
        elif k <= max_dim:
            x = min_dim
        else:
            x = m + n - k

        if k <= min_dim:
            f_x = max(3 * x - 2, 0)
        else:
            f_x = 2 * x + 1

        sum_loads += f_x

    y = 5 * m * n / (2 * m * n + 2 * sum_loads)
    return y


# Test cases for m = 100, 300, 500, and n varying from 10 to 500
m_values = [100, 300, 500]
n_values = list(range(10, 501, 10))

ratios = []
data = []

# Loop through the m and n values
for m in m_values:
    for n in n_values:
        ratio = calculate_ratio(m, n)
        ratios.append(ratio)
        data.append([m, n, ratio])
        assert ratio > 0.65, f"Ratio {ratio} is not above 0.65 for matrix size {m}x{n}"

# Calculate average ratio
average_ratio = sum(ratios) / len(ratios)
print(f"\nAverage ratio of all test cases: {average_ratio:.4f}")

# Convert the data to a DataFrame and save as CSV
df = pd.DataFrame(data, columns=['m', 'n', 'Ratio'])
df.to_csv('ratio_test_cases.csv', index=False)

print("\nAll test cases passed successfully. The CSV file has been saved.")
