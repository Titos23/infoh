#include "blastDatabase.h"
#include <fstream>
#include <iostream>
#include <cctype>

using namespace std;

BlastDatabase::BlastDatabase(const std::string& dbBasePath) 
    : databasePinPath(dbBasePath + ".pin"),
      databasePsqPath(dbBasePath + ".psq"),
      databasePhrPath(dbBasePath + ".phr"),
      isLoaded(false) {
}

bool BlastDatabase::loadPinFile() {
    // Open .pin file
    ifstream file(databasePinPath, ios::binary);
    
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open file: " << databasePinPath << endl;
        isLoaded = false;
        return false;
    }

    // Read version (4 bytes, big-endian)
    file.read(reinterpret_cast<char*>(&pinData.version), 4);
    pinData.version = __builtin_bswap32(pinData.version);

    // Read type (4 bytes, big-endian)
    file.read(reinterpret_cast<char*>(&pinData.type), 4);
    pinData.type = __builtin_bswap32(pinData.type);

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
    file.read(reinterpret_cast<char*>(&pinData.numSequences), 4);
    pinData.numSequences = __builtin_bswap32(pinData.numSequences);

    // Read number of residues (8 bytes, little-endian - no swap!)
    file.read(reinterpret_cast<char*>(&pinData.numResidues), 8);

    // Read maximum sequence length (4 bytes, big-endian)
    file.read(reinterpret_cast<char*>(&pinData.maxSeqLength), 4);
    pinData.maxSeqLength = __builtin_bswap32(pinData.maxSeqLength);

    // Resize vectors to hold offsets
    pinData.headerOffsets.resize(pinData.numSequences);
    pinData.sequenceOffsets.resize(pinData.numSequences);

    // Read header offset table
    for (uint32_t i = 0; i < pinData.numSequences; i++) {
        file.read(reinterpret_cast<char*>(&pinData.headerOffsets[i]), 4);
        if (file.fail()) {
            cerr << "ERROR: Failed to read header offset " << i << endl;
            isLoaded = false;
            return false;
        }
        pinData.headerOffsets[i] = __builtin_bswap32(pinData.headerOffsets[i]);
    }

    // Skip last offset value
    file.seekg(4, ios::cur);

    // Read sequence offset table
    for (uint32_t i = 0; i < pinData.numSequences; i++) {
        file.read(reinterpret_cast<char*>(&pinData.sequenceOffsets[i]), 4);
        if (file.fail()) {
            cerr << "ERROR: Failed to read sequence offset " << i << endl;
            isLoaded = false;
            return false;
        }
        pinData.sequenceOffsets[i] = __builtin_bswap32(pinData.sequenceOffsets[i]);
    }
    
    file.close();
    isLoaded = true;
    return true;
}

string BlastDatabase::readSequence(uint32_t index) const {
    ifstream file(databasePsqPath, ios::binary);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open file: " << databasePsqPath << endl;
        return "";
    }
    
    uint32_t startOffset = pinData.sequenceOffsets[index];
    uint32_t endOffset;
    
    if (index == pinData.numSequences - 1) {
        file.seekg(0, ios::end);
        endOffset = file.tellg();
    } else {
        endOffset = pinData.sequenceOffsets[index + 1];
    }
    
    // Subtract 1 for the NUL separator byte!
    uint32_t length = endOffset - startOffset - 1;
    
    file.seekg(startOffset, ios::beg);
    
    vector<uint8_t> encodedSeq(length);
    file.read(reinterpret_cast<char*>(encodedSeq.data()), length);
    
    // Skip the NUL separator byte
    file.seekg(1, ios::cur);
    
    string decodedSeq;
    decodedSeq.reserve(length);
    
    for (uint8_t byte : encodedSeq) {
        if (byte < 28) {  
            decodedSeq += AMINO_ACID_TABLE[byte];
        }
    }
    
    file.close();
    return decodedSeq;
}

string BlastDatabase::readHeader(uint32_t index) const {
    ifstream file(databasePhrPath, ios::binary);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open file: " << databasePhrPath << endl;
        return "";
    }

    uint32_t start = pinData.headerOffsets[index];
    uint32_t end;

    if (index == pinData.numSequences - 1) {
        file.seekg(0, ios::end);
        end = static_cast<uint32_t>(file.tellg());
    } else {
        end = pinData.headerOffsets[index + 1];
    }

    uint32_t length = end - start;
    file.seekg(start, ios::beg);

    vector<char> buffer(length);
    file.read(buffer.data(), length);
    file.close();

    // --- Step 1: Keep only printable characters ---
    string result;
    for (char c : buffer) {
        if (isprint((unsigned char)c))
            result += c;
    }

    // --- Step 2: Find where the true header starts ("sp|") ---
    size_t startPos = result.find("sp|");
    if (startPos == string::npos)
        return "";  // no header found, return empty

    result = result.substr(startPos);

    // --- Step 3: Trim everything after the first space (keep only ID part) ---
    size_t spacePos = result.find(' ');
    if (spacePos != string::npos)
        result = result.substr(0, spacePos);

    return result;
}
