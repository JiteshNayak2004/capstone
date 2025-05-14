#ifndef TEST_SEQUENCES_H
#define TEST_SEQUENCES_H

// Test sequence pairs of different sizes
static const char* query_seq = "GTACGTAGCTAGCTACGATCGATCGATCGATCGTAGCTAG";  // 40 chars
static const char* ref_seq = "ATCGATCGATCGTAGCTAGCTAGCTACGATCGATCGATCGATCGTAGCTAGCTAGCTACGATCGATCGATCGATCGTAGCTAGCTAGCTACGAT";  // 100 chars

// Additional test pairs
static const struct {
    const char* query;
    const char* ref;
} test_pairs[] = {
    {
        "ACGTACGT",  // 8 chars
        "ACGTACGTACGT"   // 12 chars
    },
    {
        "CTTGTCGCTCGTGCACGCT",  // 20 chars
        "ACAAAAAGGCAGTGGGATT"   // 20 chars
    },
    {
        "GTACGTAGCTAGCTACGAT",  // 20 chars
        "ATCGATCGATCGTAGCTAG"   // 20 chars
    }
};

#define NUM_TEST_PAIRS (sizeof(test_pairs) / sizeof(test_pairs[0]))

#endif // TEST_SEQUENCES_H
