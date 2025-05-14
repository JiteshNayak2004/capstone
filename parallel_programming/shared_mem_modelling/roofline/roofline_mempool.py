import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter
from typing import Optional, Tuple, List, Union


class RooflineModel:
    """
    A comprehensive, modular class for creating Roofline model visualizations.
    """

    def __init__(self,
                 peak_performance: float,
                 peak_bandwidth: float,
                 log_base: int = 2,
                 x_range: Tuple[int, int] = (-4, 10),
                 y_min: float = 1/16,  # New parameter to set minimum y-value
                 plot_style: Optional[dict] = None):
        """
        Initialize the Roofline Model parameters.

        Args:
            peak_performance (float): Peak computational performance (GLOP/s)
            peak_bandwidth (float): Peak memory bandwidth (GB/s)
            log_base (int, optional): Base for logarithmic scaling. Defaults to 2.
            x_range (Tuple[int, int], optional): Range for x-axis. Defaults to (-4, 10).
            y_min (float, optional): Minimum y-value for the plot. Defaults to 1/16.
            plot_style (Optional[dict]): Custom plot styling options
        """
        self.peak_performance = peak_performance
        self.peak_bandwidth = peak_bandwidth
        self.log_base = log_base
        self.x_range = x_range
        self.y_min = y_min

        # Default plot style
        self.plot_style = {
            'roofline_color': 'r',
            'roofline_linewidth': 2,
            'peak_perf_style': {'color': 'r', 'linestyle': '--'},
            'ridge_point_style': {'color': 'g', 'linestyle': '--'},
            'app_point_style': {'color': 'bo', 'markersize': 7}
        }

        # Update with user-provided style if any
        if plot_style:
            self.plot_style.update(plot_style)

    def _generate_roofline_data(self, num_points: int = 1000) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
        """
        Generate data points for the Roofline model.

        Args:
            num_points (int, optional): Number of points to generate. Defaults to 1000.

        Returns:
            Tuple of x, y_roofline, y_memory data arrays
        """
        x = np.logspace(self.x_range[0], self.x_range[1],
                        base=self.log_base, num=num_points)
        y_memory = self.peak_bandwidth * x
        y_compute = np.full_like(x, self.peak_performance)
        y_roofline = np.minimum(y_memory, y_compute)

        return x, y_roofline, y_memory

    def _calculate_ridge_point(self) -> float:
        """
        Calculate the ridge point where memory bandwidth and computational performance intersect.

        Returns:
            float: Ridge point arithmetic intensity
        """
        return self.peak_performance / self.peak_bandwidth

    def plot(self,
             points: Optional[List[Tuple[float, float]]] = None,
             ax: Optional[plt.Axes] = None) -> plt.Figure:
        """
        Create the Roofline model plot.

        Args:
            points (Optional[List[Tuple[float, float]]]): List of (x,y) points to plot
            ax (Optional[plt.Axes]): Optional existing axis to plot on

        Returns:
            plt.Figure: The generated plot figure
        """
        # Create figure and axis if not provided
        if ax is None:
            fig, ax = plt.subplots(figsize=(12, 8))
        else:
            fig = ax.get_figure()

        # Generate roofline data
        x, y_roofline, y_memory = self._generate_roofline_data()
        ridge_point = self._calculate_ridge_point()

        # Plot roofline
        ax.plot(x, y_roofline,
                color=self.plot_style['roofline_color'],
                linewidth=self.plot_style['roofline_linewidth'],
                label='Roofline')

        # Plot peak performance line
        ax.axhline(y=self.peak_performance,
                   **self.plot_style['peak_perf_style'],
                   label='Peak Performance')

        # Plot ridge point line
        ax.axvline(x=ridge_point,
                   **self.plot_style['ridge_point_style'],
                   label='Ridge Point')

        # Plot points if provided
        if points:
            for point in points:
                x_point, y_point = point
                ax.plot(x_point, y_point,
                        **self.plot_style['app_point_style'],
                        label='Application Point')

                # Annotate point
                ax.text(x_point, y_point,
                        f'({x_point:.2f}, {y_point:.2f})',
                        verticalalignment='top',
                        horizontalalignment='right')

        # Customize plot
        ax.set_xscale('log', base=self.log_base)
        ax.set_yscale('log', base=self.log_base)
        ax.set_xlabel('Arithmetic Intensity (OP/byte)')
        ax.set_ylabel('Performance (GOP/s)')
        ax.set_title(f'Roofline Model (Log Base {self.log_base})')
        ax.grid(True, which="both", ls="-", alpha=0.5)
        ax.legend()

        # Set axis limits and ticks
        ax.set_xlim(self.log_base **
                    self.x_range[0], self.log_base**self.x_range[1])
        # Updated y-axis minimum
        ax.set_ylim(self.y_min, self.log_base**self.x_range[1])

        # Custom tick formatter
        def log_formatter(x, pos):
            if x < 1:
                return f'1/{self.log_base**int(-np.log(x)/np.log(self.log_base))}'
            else:
                return str(int(x))

        ax.xaxis.set_major_formatter(FuncFormatter(log_formatter))
        ax.yaxis.set_major_formatter(FuncFormatter(log_formatter))

        # Set ticks at powers of log_base
        ax.xaxis.set_ticks(
            [self.log_base**i for i in range(self.x_range[0], self.x_range[1]+1)])
        ax.yaxis.set_ticks(
            [2**i for i in range(int(np.log2(self.y_min)), self.x_range[1]+1)])

        plt.tight_layout()

        return fig

# Example usage


def main():
    # Platform specifications
    peak_performance = 153.6  # GLOP/s
    peak_bandwidth = 233.28   # GB/s

    # Create Roofline model
    roofline = RooflineModel(peak_performance, peak_bandwidth)

    # Calculate application performance point
    arithmetic_intensity = 0.1  # FLOP/byte
    app_performance = min(
        peak_performance, arithmetic_intensity * peak_bandwidth)

    # Plot the Roofline model with multiple points
    points = [
        (arithmetic_intensity, app_performance),  # First application point
        (0.5, 50),  # Another example point
        (2, 100)   # Another example point
    ]
    fig = roofline.plot(points)
    plt.show()


if __name__ == "__main__":
    main()   main()
