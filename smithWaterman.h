#ifndef SMITH_WATERMAN_H
#define SMITH_WATERMAN_H

#include <string>
#include "blosumMatrix.h"

/**
 * SmithWaterman class for local sequence alignment
 */
class SmithWaterman {
public:
    /**
     * Calculates Smith-Waterman local alignment score
     * @param query Query sequence
     * @param target Target sequence from database
     * @param blosum BLOSUM scoring matrix
     * @param gapOpen Gap opening penalty (positive value, e.g., 11)
     * @param gapExtend Gap extension penalty (positive value, e.g., 1)
     * @return Maximum alignment score
     */
    static int align(const std::string& query,
                     const std::string& target,
                     const BlosumMatrix& blosum,
                     int gapOpen,
                     int gapExtend);
};

#endif