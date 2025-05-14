def analyze_wave_patterns(m, n):
    """
    Analyze anti-diagonal wave patterns for an m×n matrix
    Returns: List of (wave_number, cells_in_wave, theoretical_parallelism)
    """
    waves = []
    # Forward waves (growing phase)
    for i in range(min(m, n)):
        wave_num = i + 1
        cells = min(i + 1, m, n)
        parallelism = min(cells, m, n)
        waves.append((wave_num, cells, parallelism))

    # Peak waves (maximum width phase)
    max_width = min(m, n)
    start_peak = min(m, n)
    end_peak = abs(m - n) + start_peak
    for i in range(start_peak, end_peak):
        wave_num = i + 1
        cells = max_width
        parallelism = min(cells, m, n)
        waves.append((wave_num, cells, parallelism))

    # Backward waves (shrinking phase)
    remaining_dim = max(m, n)
    for i in range(max_width - 1, 0, -1):
        wave_num = end_peak + (max_width - i)
        cells = i
        parallelism = min(cells, m, n)
        waves.append((wave_num, cells, parallelism))

    return waves


# Example analysis for 6x6 (square) and 6x10 (rectangular) matrices
square_waves = analyze_wave_patterns(6, 6)
rect_waves = analyze_wave_patterns(6, 10)

# Calculate theoretical parallel efficiency
print(square_waves)
print(rect_waves)


def calc_efficiency(waves, num_cores):
    total_cells = sum(wave[1] for wave in waves)
    total_timesteps = len(waves)
    max_theoretical = total_cells / total_timesteps
    actual_parallelism = sum(min(wave[1], num_cores)
                             for wave in waves) / total_timesteps
    # Efficiency as fraction of perfect utilization
    return actual_parallelism / num_cores
