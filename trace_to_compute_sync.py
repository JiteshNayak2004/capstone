import re
import argparse

# Define a function to parse the log file and calculate sums
def parse_log_and_sum(file_path):
    compute_times = []
    sync_times = []

    # Open and read the file
    with open(file_path, 'r') as file:
        for line in file:
            # Match rows containing compute and sync times
            match = re.search(r"Compute time: (\d+), Sync time: (\d+)", line)
            if match:
                compute_times.append(int(match.group(1)))
                sync_times.append(int(match.group(2)))

    # Calculate sums
    total_compute_time = sum(compute_times)
    total_sync_time = sum(sync_times)

    return compute_times, sync_times, total_compute_time, total_sync_time

# Main function to handle command-line arguments
def main():
    parser = argparse.ArgumentParser(description="Parse a simulation log file.")
    parser.add_argument("file_path", help="Path to the simulation log file")
    args = parser.parse_args()

    # Parse the log file
    compute, sync, total_compute, total_sync = parse_log_and_sum(args.file_path)

    # Output the results
    print("Compute Times:", compute)
    print("Sync Times:", sync)
    print("Total Compute Time:", total_compute)
    print("Total Sync Time:", total_sync)

if __name__ == "__main__":
    main()

