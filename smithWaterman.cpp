#include "smithWaterman.h"
#include <algorithm>
#include <vector>

using namespace std;

int SmithWaterman::align(const string& query,
                         const string& target,
                         const BlosumMatrix& blosum,
                         int gapOpen,
                         int gapExtend) {
    
    const int m = query.length();
    const int n = target.length();
    
    if (m == 0 || n == 0) return 0;
    
    const int gapPenalty = gapOpen + gapExtend;
    
    // Use 1D arrays instead of 2D to improve cache locality and reduce memory overhead
    vector<int> prevRow(n + 1, 0);
    vector<int> currRow(n + 1, 0);
    
    int maxScore = 0;
    
    // Use restricted pointers to make intent explicit for the compiler and reduce aliasing
    const char* __restrict__ queryPtr = query.c_str();
    const char* __restrict__ targetPtr = target.c_str();
    
    for (int i = 1; i <= m; i++) {
        const char queryAA = queryPtr[i - 1];
        currRow[0] = 0;
        
        int* __restrict__ curr = currRow.data();
        const int* __restrict__ prev = prevRow.data();
        
        // Provide a hint to the compiler to help auto-vectorization where appropriate
        #pragma GCC ivdep
        for (int j = 1; j <= n; j++) {
            const char targetAA = targetPtr[j - 1];
            
            // Use BLOSUM lookup via the precomputed table 
            const int blosumScore = blosum.getScore(queryAA, targetAA);
            
            // Calculate three options
            const int match = prev[j - 1] + blosumScore;
            const int deleteGap = prev[j] - gapPenalty;
            const int insertGap = curr[j - 1] - gapPenalty;
            
            // Compute maximum of candidates using conditional expressions
            int score = 0;
            score = (match > score) ? match : score;
            score = (deleteGap > score) ? deleteGap : score;
            score = (insertGap > score) ? insertGap : score;
            
            curr[j] = score;
            
            // Track maximum score
            if (score > maxScore) {
                maxScore = score;
            }
        }
        
        // Swap rows efficiently to reuse allocated memory and avoid re-allocations
        swap(prevRow, currRow);
    }
    
    return maxScore;
}