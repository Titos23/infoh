#include <iostream>
#include "queryReader.h"
#include "blastReader.h"

using namespace std;

int main() {
    // Read the .pin file (BLAST database index)
    PinData pinData;
    if (!readPinFile(pinData)) {
        cerr << "Failed to read .pin file!" << endl;
        return 1;
    }
    
    // Print database information
    cout << "=== Database Information ===" << endl;
    cout << "Version: " << pinData.version << endl;
    cout << "Type: " << pinData.type << endl;
    cout << "Number of Sequences: " << pinData.numSequences << endl;
    cout << "Number of Residues: " << pinData.numResidues << endl;
    cout << "Maximum Sequence Length: " << pinData.maxSeqLength << endl;
    cout << endl;

    cout << "=== Debug: Sequence Offsets ===" << endl;
    cout << "sequenceOffsets[0]: " << pinData.sequenceOffsets[0] << endl;
    cout << "sequenceOffsets[1]: " << pinData.sequenceOffsets[1] << endl;
    cout << "sequenceOffsets[2]: " << pinData.sequenceOffsets[2] << endl;
    cout << "sequenceOffsets[3]: " << pinData.sequenceOffsets[3] << endl;
    cout << "Calculated length for seq 0: " << (pinData.sequenceOffsets[1] - pinData.sequenceOffsets[0]) << endl;
    cout << endl;

    // Read the query sequence
    string querySequence = readQuery();
    
    // Check if reading was successful
    if (querySequence.empty()) {
        cerr << "Failed to read query sequence!" << endl;
        return 1;
    }
    
    // Print query information
    cout << "=== Query Information ===" << endl;
    cout << "Query Sequence Length: " << querySequence.length() << endl;
    cout << "First 50 characters: " << querySequence.substr(0, 50) << endl;
    cout << endl;
    
    // Test reading sequences from .psq
    cout << "=== Testing Sequence Reading ===" << endl;
    string firstSeq = readSequenceFromPsq(0, pinData);
    cout << "First sequence length: " << firstSeq.length() << endl;
    cout << "First 50 characters: " << firstSeq.substr(0, 50) << endl;
    cout << endl;
    
    string secondSeq = readSequenceFromPsq(573660, pinData);
    cout << "Second sequence length: " << secondSeq.length() << endl;
    cout << "First 50 characters: " << secondSeq.substr(0, 50) << endl;
    cout << endl;
    
    // TODO: Next steps
    // 1. Loop through all sequences
    // 2. Compare each with query
    // 3. Find exact match
    // 4. Read identifier from .phr
    // 5. Output the identifier
    
    return 0;
}