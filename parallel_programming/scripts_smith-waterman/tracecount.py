import re
import sys

def count_instructions_in_range(file_path, start_time, end_time):
    memory_instructions = 0
    compute_instructions = 0

    with open(file_path, 'r') as file:
        for line in file:
            # Skip empty or incomplete lines
            if not line.strip() or len(line.split()) < 5:
                continue
            # Extract time (first parameter) and instruction mnemonic
            parts = line.split()
            try:
                timestamp = int(parts[0])  # Convert the time to an integer
            except ValueError:
                continue  # Skip lines where time is not a valid integer
            # Only consider lines within the specified time range
            if start_time <= timestamp <= end_time:
                instruction = parts[3].lower()
                if instruction in {'lw', 'sw'}:
                    memory_instructions += 1
                else:
                    compute_instructions += 1

    return memory_instructions, compute_instructions

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python3 tracecount.py <trace_file> <start_time> <end_time>")
        sys.exit(1)

    # Parse command-line arguments
    trace_file = sys.argv[1]
    try:
        start_time = int(sys.argv[2])
        end_time = int(sys.argv[3])
    except ValueError:
        print("Error: start_time and end_time must be integers.")
        sys.exit(1)

    mem_instr, comp_instr = count_instructions_in_range(trace_file, start_time, end_time)
    print(f"Memory Instructions (lw/sw): {mem_instr}")
    print(f"Compute Instructions (others): {comp_instr}")
    if mem_instr > 0:
        print("AI is:", comp_instr / (mem_instr * 4))
    else:
        print("AI is: Undefined (no memory instructions found)")
