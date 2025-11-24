#include "smithWaterman.h"
#include <vector>
#include <algorithm>

using namespace std;

int SmithWaterman::align(const string& query,
                         const string& target,
                         const BlosumMatrix& blosum,
                         int gapOpen,
                         int gapExtend) {
    
    int m = query.length();
    int n = target.length();
    
    // Handle empty sequences
    if (m == 0 || n == 0) return 0;
    
    // Create scoring matrix (initialized to 0)
    vector<vector<int>> H(m + 1, vector<int>(n + 1, 0));
    
    int maxScore = 0;
    int gapPenalty = gapOpen + gapExtend;
    
    // Fill the matrix
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            // Get amino acids (convert to 0-indexed)
            char queryAA = query[i - 1];
            char targetAA = target[j - 1];
            
            // Calculate scores for each option
            int match = H[i - 1][j - 1] + blosum.getScore(queryAA, targetAA);
            int deleteGap = H[i - 1][j] - gapPenalty;
            int insertGap = H[i][j - 1] - gapPenalty;
            
            // Take maximum (including 0 for local alignment)
            H[i][j] = max({0, match, deleteGap, insertGap});
            
            // Track maximum score
            if (H[i][j] > maxScore) {
                maxScore = H[i][j];
            }
        }
    }
    
    return maxScore;
}