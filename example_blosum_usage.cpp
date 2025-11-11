// Example: How to use BlosumMatrix class
#include "blosumMatrix.h"
#include <iostream>

using namespace std;

int main() {
    // Create a BlosumMatrix instance
    BlosumMatrix blosum;
    
    // Load the BLOSUM62 matrix from file
    if (!blosum.load("blosum/BLOSUM62")) {
        cerr << "Failed to load BLOSUM matrix!" << endl;
        return 1;
    }
    
    // Get substitution scores between amino acids
    int score1 = blosum.getScore('A', 'A');  // Match: should be positive
    int score2 = blosum.getScore('A', 'R');  // Mismatch: usually lower
    int score3 = blosum.getScore('A', 'X');  // Unknown: returns -23
    
    cout << "Score(A, A) = " << score1 << endl;
    cout << "Score(A, R) = " << score2 << endl;
    cout << "Score(A, X) = " << score3 << endl;
    
    return 0;
}

