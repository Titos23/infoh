#ifndef SMITH_WATERMAN_H
#define SMITH_WATERMAN_H

#include <string>
#include "blosumMatrix.h"

/**
 * SmithWaterman class for local sequence alignment
 * Implementation notes: uses 1D arrays and compiler hints to improve
 * memory locality and enable vectorization where applicable.
 */
class SmithWaterman {
public:
    /**
     * Performs Smith-Waterman local alignment
     * @param query Query sequence
     * @param target Target sequence
     * @param blosum BLOSUM substitution matrix
     * @param gapOpen Gap opening penalty
     * @param gapExtend Gap extension penalty
     * @return Maximum alignment score
     */
    static int align(const std::string& query,
                    const std::string& target,
                    const BlosumMatrix& blosum,
                    int gapOpen,
                    int gapExtend);
};

#endif