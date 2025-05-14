import random

def generate_random_sequence(length):
    return ''.join(random.choice('ACGT') for _ in range(length))

# Generate larger test sequences for better thread scaling analysis
QUERY_SEQUENCE_LENGTH = 500  # increased from 1000 to 5000
REFERENCE_SEQUENCE_LENGTH = 20000  # increased from 1000 to 5000

query = generate_random_sequence(QUERY_SEQUENCE_LENGTH)
ref = generate_random_sequence(REFERENCE_SEQUENCE_LENGTH)

# Save sequences
with open('./dataset/query.txt', 'w') as f:
    f.write(query)

with open('./dataset/ref.txt', 'w') as f:
    f.write(ref)
