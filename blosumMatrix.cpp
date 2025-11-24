#include "blosumMatrix.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

using namespace std;

bool BlosumMatrix::load(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open BLOSUM file: " << filepath << endl;
        return false;
    }

    string line;
    vector<char> aminoAcids;

    // Read header line (amino acid letters)
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
    
    // Initialize precomputed lookup table for fast access to substitution scores
    initializeLookupTable();
    
    return true;
}

void BlosumMatrix::initializeLookupTable() {
    // Initialize all entries with default penalty
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            lookupTable[i][j] = -4;  // Default penalty for unknown amino acids
        }
    }
    
    // Fill in known amino acid scores from the matrix
    for (const auto& p1 : aminoAcidToIndex) {
        for (const auto& p2 : aminoAcidToIndex) {
            unsigned char c1 = static_cast<unsigned char>(p1.first);
            unsigned char c2 = static_cast<unsigned char>(p2.first);
            lookupTable[c1][c2] = matrix[p1.second][p2.second];
        }
    }
    
    // Handle rare or ambiguous amino acids using BLAST-style mappings
    // U (Selenocysteine) -> C (Cysteine) - chemically similar
    // O (Pyrrolysine) -> K (Lysine) - structurally similar
    // J (Leu/Ile ambiguity) -> L (Leucine)
    auto copyScores = [this](char from, char to) {
        unsigned char ufrom = static_cast<unsigned char>(from);
        unsigned char uto = static_cast<unsigned char>(to);
        
        for (int i = 0; i < 256; i++) {
            lookupTable[ufrom][i] = lookupTable[uto][i];
            lookupTable[i][ufrom] = lookupTable[i][uto];
        }
        lookupTable[ufrom][ufrom] = lookupTable[uto][uto];
    };
    
    if (aminoAcidToIndex.count('C')) copyScores('U', 'C');
    if (aminoAcidToIndex.count('K')) copyScores('O', 'K');
    if (aminoAcidToIndex.count('L')) copyScores('J', 'L');
}