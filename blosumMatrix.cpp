#include "blosumMatrix.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

bool BlosumMatrix::load(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open BLOSUM file: " << filepath << endl;
        return false;
    }

    string line;
    vector<char> aminoAcids;

    // Read header line
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        istringstream iss(line);
        char aa;
        while (iss >> aa) {
            aminoAcids.push_back(aa);
            aminoAcidToIndex[aa] = aminoAcids.size() - 1;
        }
        break;
    }

    int n = static_cast<int>(aminoAcids.size());
    matrix.resize(n, vector<int>(n, 0));

    // Read matrix values
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

int BlosumMatrix::getScore(char aa1, char aa2) const {
    // BLAST-style mapping (industry standard)
    auto normalize = [](char aa) -> char {
        switch(aa) {
            case 'U': return 'C';  // Selenocysteine → Cysteine
            case 'O': return 'K';  // Pyrrolysine → Lysine
            case 'J': return 'L';  // Leu/Ile → Leucine
            default: return aa;
        }
    };
    
    char norm1 = normalize(aa1);
    char norm2 = normalize(aa2);
    
    auto it1 = aminoAcidToIndex.find(norm1);
    auto it2 = aminoAcidToIndex.find(norm2);
    
    // Fallback to X if still unknown
    if (it1 == aminoAcidToIndex.end()) {
        it1 = aminoAcidToIndex.find('X');
    }
    if (it2 == aminoAcidToIndex.end()) {
        it2 = aminoAcidToIndex.find('X');
    }
    
    // Final fallback (should never happen)
    if (it1 == aminoAcidToIndex.end() || it2 == aminoAcidToIndex.end()) {
        return -4;
    }
    
    return matrix[it1->second][it2->second];
}