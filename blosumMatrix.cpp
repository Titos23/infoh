#include "blosumMatrix.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

// ------------------------------------------------------------
// Load BLOSUM matrix from file
// ------------------------------------------------------------
bool BlosumMatrix::load(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open BLOSUM file: " << filepath << endl;
        return false;
    }

    string line;
    vector<char> aminoAcids;

    // --- Read header line (amino acid letters) ---
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        istringstream iss(line);
        char aa;
        while (iss >> aa) {
            aminoAcids.push_back(aa);
            aminoAcidToIndex[aa] = aminoAcids.size() - 1;
        }
        break; // stop after first valid header line
    }

    int n = static_cast<int>(aminoAcids.size());
    matrix.resize(n, vector<int>(n, 0));

    // --- Read matrix values ---
    int row = 0;
    while (getline(file, line) && row < n) {
        if (line.empty() || line[0] == '#') continue;

        istringstream iss(line);
        char rowAA;
        iss >> rowAA;

        for (int col = 0; col < n; ++col) {
            int score;
            if (!(iss >> score)) break;
            matrix[row][col] = score;
        }

        ++row;
    }

    file.close();
    return true;
}

// ------------------------------------------------------------
// Retrieve substitution score between two amino acids
// ------------------------------------------------------------
int BlosumMatrix::getScore(char aa1, char aa2) const {
    if (aminoAcidToIndex.count(aa1) == 0 || aminoAcidToIndex.count(aa2) == 0) {
        cerr << "Unknown amino acid(s): ";
        if (aminoAcidToIndex.count(aa1) == 0) cerr << aa1 << " ";
        if (aminoAcidToIndex.count(aa2) == 0) cerr << aa2 << " ";
        cerr << endl;
        return -23; // obvious error value
    }

    return matrix.at(aminoAcidToIndex.at(aa1))
                 .at(aminoAcidToIndex.at(aa2));
}

