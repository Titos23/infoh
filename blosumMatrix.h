#ifndef BLOSUM_MATRIX_H
#define BLOSUM_MATRIX_H

#include <string>
#include <vector>
#include <map>

/**
 * BlosumMatrix class for loading and querying BLOSUM substitution matrices
 */
class BlosumMatrix {
private:
    std::vector<std::vector<int>> matrix;
    std::map<char, int> aminoAcidToIndex;

public:
    /**
     * Loads a BLOSUM matrix from a file
     * @param filepath Path to the BLOSUM matrix file
     * @return true if successful, false otherwise
     */
    bool load(const std::string& filepath);

    /**
     * Gets the substitution score between two amino acids
     * @param aa1 First amino acid
     * @param aa2 Second amino acid
     * @return Substitution score, or -23 if amino acid not found
     */
    int getScore(char aa1, char aa2) const;
};

#endif

