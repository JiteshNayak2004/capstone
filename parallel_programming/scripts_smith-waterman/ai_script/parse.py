compute_ops = 0
memory_ops = 0

# Open and read the file
with open("demo.txt", 'r') as f:
    instr_list = f.read().split("\n")  # Read lines and split them into a list

print("The type is", type(instr_list))  # Confirm the type of instr_list

# Iterate through each line in the instruction list
for line in instr_list:
    if line.strip():  # Ensure the line is not empty
        # Extract the instruction (assuming it's always the 3rd column)
        list_line = line.split()[2]
        print(list_line)

        # Check for memory operations
        if any(op in list_line for op in ["lw", "sw", "lbu"]):
            memory_ops += 1

        # Check for compute operations
        elif any(op in list_line for op in ["add", "addi", "slli", "bge", "bgez", "bne"]):
            compute_ops += 1

# Print the counts of memory and compute operations
print("Memory operations:", memory_ops)
print("Compute operations:", compute_ops)
