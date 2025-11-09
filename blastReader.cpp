#include "blastReader.h"
#include <fstream>
#include <iostream>

using namespace std;

bool readPinFile(PinData& data) {
    // Open .pin file using the define
    ifstream file(DATABASE_PIN, ios::binary);
    
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open file: " << DATABASE_PIN << endl;
        return false;
    }

    // Read version (4 bytes, big-endian)
    file.read(reinterpret_cast<char*>(&data.version), 4);
    data.version = __builtin_bswap32(data.version);

    // Read type (4 bytes, big-endian)
    file.read(reinterpret_cast<char*>(&data.type), 4);
    data.type = __builtin_bswap32(data.type);

    // Read title length (4 bytes, big-endian)
    uint32_t titleLength;
    file.read(reinterpret_cast<char*>(&titleLength), 4);
    titleLength = __builtin_bswap32(titleLength);

    // Skip title
    file.seekg(titleLength, ios::cur);
    
    // Read date length (4 bytes, big-endian)
    uint32_t dateLength;
    file.read(reinterpret_cast<char*>(&dateLength), 4);
    dateLength = __builtin_bswap32(dateLength);

    // Skip date
    file.seekg(dateLength, ios::cur);

    // Read number of sequences (4 bytes, big-endian)
    file.read(reinterpret_cast<char*>(&data.numSequences), 4);
    data.numSequences = __builtin_bswap32(data.numSequences);

    // Read number of residues (8 bytes, little-endian - no swap!)
    file.read(reinterpret_cast<char*>(&data.numResidues), 8);

    // Read maximum sequence length (4 bytes, big-endian)
    file.read(reinterpret_cast<char*>(&data.maxSeqLength), 4);
    data.maxSeqLength = __builtin_bswap32(data.maxSeqLength);

    cout << "[DEBUG] File position before offset tables: " << file.tellg() << endl;

    // Resize vectors to hold offsets
    data.headerOffsets.resize(data.numSequences);
    data.sequenceOffsets.resize(data.numSequences);

    // Read header offset table
for (uint32_t i = 0; i < data.numSequences; i++) {
    file.read(reinterpret_cast<char*>(&data.headerOffsets[i]), 4);
    if (file.fail()) {
        cerr << "ERROR: Failed to read header offset " << i << endl;
        return false;
    }
    data.headerOffsets[i] = __builtin_bswap32(data.headerOffsets[i]);
}

cout << "[DEBUG] File position after header offsets: " << file.tellg() << endl;

// Read sequence offset table
for (uint32_t i = 0; i < data.numSequences; i++) {
    file.read(reinterpret_cast<char*>(&data.sequenceOffsets[i]), 4);
    if (file.fail()) {
        cerr << "ERROR: Failed to read sequence offset " << i << endl;
        return false;
    }
    data.sequenceOffsets[i] = __builtin_bswap32(data.sequenceOffsets[i]);
}

    cout << "[DEBUG] File position after sequence offsets: " << file.tellg() << endl;
    cout << "[DEBUG] First 4 sequence offsets: " 
         << data.sequenceOffsets[0] << ", "
         << data.sequenceOffsets[1] << ", "
         << data.sequenceOffsets[2] << ", "
         << data.sequenceOffsets[3] << endl;

    file.close();
    return true;
}

string readSequenceFromPsq(uint32_t index, const PinData& pinData) {
    // Open .psq file
    ifstream file(DATABASE_PSQ, ios::binary);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open file: " << DATABASE_PSQ << endl;
        return "";
    }
    
    // Calculate sequence length
    uint32_t startOffset = pinData.sequenceOffsets[index];
    uint32_t endOffset;
    
    // Handle last sequence specially (no next offset)
    if (index == pinData.numSequences - 1) {
        // For last sequence, read until end of file
        file.seekg(0, ios::end);
        endOffset = file.tellg();
    } else {
        endOffset = pinData.sequenceOffsets[index + 1];
    }
    
    uint32_t length = endOffset - startOffset;
    
    // Go to start of sequence
    file.seekg(startOffset, ios::beg);
    
    // Read the encoded bytes
    vector<uint8_t> encodedSeq(length);
    file.read(reinterpret_cast<char*>(encodedSeq.data()), length);
    
    // Decode each byte to amino acid
    string decodedSeq;
    decodedSeq.reserve(length);  // Pre-allocate for efficiency
    
    for (uint8_t byte : encodedSeq) {
        if (byte < 25) {  // Valid amino acid code
            decodedSeq += AMINO_ACID_TABLE[byte];
        }
    }
    
    file.close();
    return decodedSeq;
}