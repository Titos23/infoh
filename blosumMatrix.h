#ifndef BLOSUM_MATRIX_H
#define BLOSUM_MATRIX_H

#include <string>
#include <vector>
#include <map>

/**
 * BlosumMatrix class for loading and querying BLOSUM substitution matrices
 * Implementation notes: uses a precomputed O(1) lookup table (256x256)
 * to provide fast substitution-score access instead of map lookups.
 */
class BlosumMatrix {
private:
    std::vector<std::vector<int>> matrix;
    std::map<char, int> aminoAcidToIndex;
    
    // Precomputed O(1) lookup table (256x256 for all possible char combinations)
    // Aligned to 64 bytes to help cache performance on many architectures
    alignas(64) int lookupTable[256][256];
    
    void initializeLookupTable();

public:
    /**
     * Loads a BLOSUM matrix from a file
     * @param filepath Path to the BLOSUM matrix file
     * @return true if successful, false otherwise
     */
    bool load(const std::string& filepath);

    /**
     * Gets the substitution score between two amino acids.
     * This uses the precomputed lookup table to return scores in O(1) time.
     * @param aa1 First amino acid
     * @param aa2 Second amino acid
     * @return Substitution score
     */
    inline int getScore(char aa1, char aa2) const __attribute__((always_inline)) {
        return lookupTable[static_cast<unsigned char>(aa1)]
                         [static_cast<unsigned char>(aa2)];
    }
};

#endif