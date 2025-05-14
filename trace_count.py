import sys
from collections import defaultdict

sum_compute_instr = 0


def count_instructions_in_range(file_path, start_time, end_time):
    memory_instructions = defaultdict(int)
    compute_instructions = defaultdict(int)
    lsu_stalls = 0
    acc_stalls = 0

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

                # Check for memory instructions (loads and stores)
                if any(mem_op in instruction for mem_op in ['lw', 'sw', 'lh',
                                                            'lhu', 'sh','shu',
                                                            'lb', 'lbu',
                                                            'sb','sbu']):
                    memory_instructions[instruction] += 1
                else:
                    # Check for compute instructions (exclude stalls)
                    if instruction not in {'(lsu)', '(acc)'}:
                        compute_instructions[instruction] += 1

                # Check for LSU stalls
                if instruction == '(lsu)':
                    lsu_stalls += 1
                if instruction == '(acc)':
                    acc_stalls += 1

    return memory_instructions, compute_instructions, lsu_stalls, acc_stalls


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

    mem_instr_dict, comp_instr_dict, lsu_stalls, acc_stalls = count_instructions_in_range(
        trace_file, start_time, end_time)

    print("\nMemory Instructions:")
    total_byte_ls = sum(count for instr, count in mem_instr_dict.items() if
                        any(op in instr for op in ['lb', 'lbu', 'sb','sbu']))
    total_half_word_ls = sum(count for instr, count in mem_instr_dict.items() if
                             any(op in instr for op in ['lh', 'lhu', 'sh','shu']))
    total_word_ls = sum(count for instr, count in mem_instr_dict.items() if any(op in instr for op in ['lw', 'sw']))

    for instr, count in sorted(mem_instr_dict.items(), key=lambda x: x[1], reverse=True):
        print(f"{instr}: {count}")

    total_weighted_ls = total_byte_ls * 1 + total_half_word_ls * 2 + total_word_ls * 4

    print(f"\nTotal Byte Load/Store: {total_byte_ls}")
    print(f"Total Half Word Load/Store: {total_half_word_ls}")
    print(f"Total Word Load/Store: {total_word_ls}")

    print("\nCompute Instructions:")
    for instr, count in sorted(comp_instr_dict.items(), key=lambda x: x[1], reverse=True):
        sum_compute_instr += count
        print(f"{instr}: {count}")

    if total_weighted_ls > 0:
        print("Total compute instructions are", sum_compute_instr)
        print(f"\nArithmetic Intensity (AI) is: {sum_compute_instr / total_weighted_ls}")
    else:
        print("\nArithmetic Intensity (AI) is: Undefined (no memory instructions found)")

    print("\nStalls:")
    print(f"Total LSU stalls: {lsu_stalls}")
    print(f"Total Accelerator stalls: {acc_stalls}")

