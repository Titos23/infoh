#include "queryReader.h"
#include <iostream>
#include <fstream>
using namespace std;

/**
 * Reads a protein sequence from a FASTA file
 *
 * FASTA format consists of:
 * - Header line starting with '>' (contains metadata)
 * - Sequence lines (amino acid codes)
 *
 * @return The protein sequence as a string (without spaces/newlines)
 *         Returns empty string if file cannot be opened
 */
string readQuery() {
    // Open file in text mode (default for ifstream)
    ifstream queryFile(QUERY_FILE);
    
    // Check if file opened successfully
    if (!queryFile.is_open()) {
        cerr << "ERROR: Could not open query file: " << QUERY_FILE << endl;
        return "";
    }
    
    string sequence;
    string line;
    
    // Skip the header line (starts with '>')
    getline(queryFile, line);
    
    // Read all remaining lines and concatenate them
    while (getline(queryFile, line)) {
        sequence += line;
    }
    
    queryFile.close();
    return sequence;
}