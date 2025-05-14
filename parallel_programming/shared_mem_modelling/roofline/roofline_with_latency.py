import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter

# Platform specifications
peak_performance = 9.6  # GLOP/s for 16 cores
peak_bandwidth = [38.4, 12.8, 7.68]  # GB/s for 16 cores
# 8 cores max
# peak_performance = 4.8
# peak_bandwidth = [19.2, 6.4, 3.8]  # GB/s for 8 cores
arithmetic_intensity_antidiagonal = 2.35  # FLOP/byte
arithmetic_intensity_rowparallel = 1.24  # FLOP/byte
arithmetic_intensity_serial = 2.621  # FLOP/byte

# Calculate ridge points
ridge_point_1 = peak_performance / peak_bandwidth[0]
ridge_point_2 = peak_performance / peak_bandwidth[1]
ridge_point_3 = peak_performance / peak_bandwidth[2]


# Calculate performance for each arithmetic intensity

# Display the plot with the new points
plt.tight_layout()
plt.show()

# Create data for the roofline
x = np.logspace(-4, 10, base=2, num=1000)
y_memory_1 = peak_bandwidth[0] * x
y_memory_2 = peak_bandwidth[1] * x
y_memory_3 = peak_bandwidth[2] * x

y_compute = np.full_like(x, peak_performance)

# Calculate the roofline for each memory-bound case
y_roofline_1 = np.minimum(y_memory_1, y_compute)
y_roofline_2 = np.minimum(y_memory_2, y_compute)
y_roofline_3 = np.minimum(y_memory_3, y_compute)

# Create the plot
fig, ax = plt.subplots(figsize=(12, 8))

# Plot the rooflines for each bandwidth
ax.plot(x, y_roofline_1, 'r-', linewidth=2,
        label=f'Roofs for 1 cycle latency (BW=[{peak_bandwidth[0]}GB/s)')
ax.plot(x, y_roofline_2, 'g-', linewidth=2,
        label=f'Roofline for 3 cycle latency(BW={peak_bandwidth[1]}GB/s)')
ax.plot(x, y_roofline_3, 'b-', linewidth=2,
        label=f'Roofline for 5 cycle latency(BW={peak_bandwidth[2]}GB/s)')

# Add lines for peak performance and ridge points
ax.axvline(x=ridge_point_1, color='r', linestyle='--',
           label=f'Ridge Point for 1,3,5 cycle latency is {ridge_point_1:.2f} {ridge_point_2:.2f} {ridge_point_3:.2f}')

ax.axvline(x=ridge_point_1, color='r', linestyle='--',
           label=f'Arithmetic_intensity of Anti-diagonal is {arithmetic_intensity_antidiagonal: .2f} ')
ax.axvline(x=ridge_point_1, color='r', linestyle='--',
           label=f'Arithmetic_intensity of Rowparallel is {arithmetic_intensity_rowparallel: .2f} ')

ax.axvline(x=ridge_point_1, color='r', linestyle='--',
           label=f'Arithmetic_intensity of serial is {arithmetic_intensity_serial: .2f} ')

# Plot the application points
# serial stuff
#ax.plot(arithmetic_intensity_serial, 0.114, 'kd', markersize=8,
#        label='serial implementation (20x40)')


# rowparallel
ax.plot( 0.215, 'kd', markersize=8,label='core 2,8,16 rowparallel in diamond')
ax.plot(0.153, 'ko',
        markersize=8, label='core 2,8,16 antidiagonal in circle')


ax.plot( 0.215, 'rd', markersize=8,label='20x40 in red')
#ax.plot( 0.215, 'gd', markersize=8,label='30x70 in green')
ax.plot( 0.215, 'bd', markersize=8,label='40x100 in blue')

# 20x40
#ax.plot(arithmetic_intensity_rowparallel, 0.144, 'rd', markersize=8)
ax.plot(arithmetic_intensity_rowparallel, 0.251, 'rd', markersize=8)
#ax.plot(arithmetic_intensity_rowparallel, 0.386, 'rd', markersize=8)
ax.plot(arithmetic_intensity_rowparallel, 0.608, 'rd', markersize=8)
ax.plot(arithmetic_intensity_rowparallel, 0.761, 'rd', markersize=8)
# 30x70
#ax.plot(arithmetic_intensity_rowparallel, 0.149, 'gd', markersize=8)
#ax.plot(arithmetic_intensity_rowparallel, 0.278, 'gd', markersize=8)
#ax.plot(arithmetic_intensity_rowparallel, 0.485, 'gd', markersize=8)
#ax.plot(arithmetic_intensity_rowparallel, 0.759, 'gd', markersize=8)
#ax.plot(arithmetic_intensity_rowparallel, 1.059, 'gd', markersize=8)
# 40x100
#ax.plot(arithmetic_intensity_rowparallel, 0.148, 'bd', markersize=8)
ax.plot(arithmetic_intensity_rowparallel, 0.290, 'bd', markersize=8)
#ax.plot(arithmetic_intensity_rowparallel, 0.515, 'bd', markersize=8)
ax.plot(arithmetic_intensity_rowparallel, 0.858, 'bd', markersize=8)
ax.plot(arithmetic_intensity_rowparallel, 1.29, 'bd', markersize=8)

# antidiagonal

#20x40

# 20x40
#ax.plot(arithmetic_intensity_antidiagonal, 0.163, 'ro', markersize=8)
ax.plot(arithmetic_intensity_antidiagonal, 0.254, 'ro', markersize=8)
#ax.plot(arithmetic_intensity_antidiagonal, 0.371, 'ro', markersize=8)
ax.plot(arithmetic_intensity_antidiagonal, 0.481, 'ro', markersize=8)
ax.plot(arithmetic_intensity_antidiagonal, 0.552, 'ro', markersize=8)
# 30x70
#ax.plot(arithmetic_intensity_antidiagonal, 0.160, 'go', markersize=8)
#ax.plot(arithmetic_intensity_antidiagonal, 0.292, 'go', markersize=8)
#ax.plot(arithmetic_intensity_antidiagonal, 0.417, 'go', markersize=8)
#ax.plot(arithmetic_intensity_antidiagonal, 0.607, 'go', markersize=8)
#ax.plot(arithmetic_intensity_antidiagonal, 0.767, 'go', markersize=8)
# 40x100
#ax.plot(arithmetic_intensity_antidiagonal, 0.166, 'bo', markersize=8)
ax.plot(arithmetic_intensity_antidiagonal, 0.277, 'bo', markersize=8)
#ax.plot(arithmetic_intensity_antidiagonal, 0.465, 'bo', markersize=8)
ax.plot(arithmetic_intensity_antidiagonal, 0.725, 'bo', markersize=8)
ax.plot(arithmetic_intensity_antidiagonal, 0.856, 'bo', markersize=8)


# Customize the plot
ax.set_xscale('log', base=2)
ax.set_yscale('log', base=2)
ax.set_xlabel('Arithmetic Intensity (OP/byte)')
ax.set_ylabel('Performance (GOP/s)')
ax.set_title('Roofline Model (minpool) ')
ax.grid(True, which="both", ls="-", alpha=0.5)
ax.legend()

# Set axis limits
ax.set_xlim(1/16, 1024)
ax.set_ylim(1/16, 1024)

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
ax.yaxis.set_ticks([2**i for i in range(-4, 11)])

plt.tight_layout()
plt.show()
