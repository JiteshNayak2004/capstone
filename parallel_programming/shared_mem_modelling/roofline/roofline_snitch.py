import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter

# Platform specifications
peak_performance = 894.5  # GFLOP/s
peak_bandwidth = 912  # GB/s
arithmetic_intensity = 5.0925  # FLOP/byte

# Calculate ridge point
ridge_point = peak_performance / peak_bandwidth

# Create data for the roofline
x = np.logspace(-4, 10, base=2, num=1000)
y_memory = peak_bandwidth * x
y_compute = np.full_like(x, peak_performance)
y_roofline = np.minimum(y_memory, y_compute)

# Create the plot
fig, ax = plt.subplots(figsize=(12, 8))

# Plot the roofline
ax.plot(x, y_roofline, 'r-', linewidth=2, label='Roofline')

# Add lines for peak performance and ridge point
ax.axhline(y=peak_performance, color='r',
           linestyle='--', label='Peak Performance')
ax.axvline(x=ridge_point, color='g', linestyle='--', label='Ridge Point')

# Plot the application point
app_performance = min(peak_performance, arithmetic_intensity * peak_bandwidth)
ax.plot(arithmetic_intensity, app_performance,
        'bo', markersize=10, label='Application')

# Customize the plot
ax.set_xscale('log', base=2)
ax.set_yscale('log', base=2)
ax.set_xlabel('Arithmetic Intensity (FLOP/byte)')
ax.set_ylabel('Performance (MFLOP/s)')
ax.set_title('Roofline Model (Log Base 2)')
ax.grid(True, which="both", ls="-", alpha=0.5)
ax.legend()

# Add labels for points of interest
ax.text(ridge_point, peak_performance, f'Ridge Point\n({ridge_point:.2f}, {peak_performance})',
        verticalalignment='bottom', horizontalalignment='left')

ax.text(arithmetic_intensity, app_performance, f'Application\n({arithmetic_intensity}, {app_performance:.2f})',
        verticalalignment='top', horizontalalignment='right')

# Set axis limits
ax.set_xlim(1/16, 1024)
ax.set_ylim(1, 2048)

# Customize tick labels


def log2_formatter(x, pos):
    if x < 1:
        return f'1/{2**int(-np.log2(x))}'
    else:
        return str(int(x))


ax.xaxis.set_major_formatter(FuncFormatter(log2_formatter))
ax.yaxis.set_major_formatter(FuncFormatter(log2_formatter))

# Adjust tick positions to be at powers of 2
ax.xaxis.set_ticks([2**i for i in range(-4, 11)])
ax.yaxis.set_ticks([2**i for i in range(0, 11)])

plt.tight_layout()
plt.show()
