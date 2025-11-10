#include "blastReader.h"
#include <fstream>
#include <iostream>
#include "blastReader.h"
#include <filesystem>
using namespace std;

string DATABASE_PIN;
string DATABASE_PSQ;
string DATABASE_PHR;

void setDatabasePaths(const std::string& dbBasePath) {
    std::string base = dbBasePath;

    DATABASE_PIN = base + ".pin";
    DATABASE_PSQ = base + ".psq";
    DATABASE_PHR = base + ".phr";
}

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

    // Sip last offset value
    file.seekg(4, ios::cur);


    // Read sequence offset table
    for (uint32_t i = 0; i < data.numSequences; i++) {
        file.read(reinterpret_cast<char*>(&data.sequenceOffsets[i]), 4);
        if (file.fail()) {
            cerr << "ERROR: Failed to read sequence offset " << i << endl;
            return false;
        }
        data.sequenceOffsets[i] = __builtin_bswap32(data.sequenceOffsets[i]);
        }
        file.close();
        return true;
    }

string readSequenceFromPsq(uint32_t index, const PinData& pinData) {
    ifstream file(DATABASE_PSQ, ios::binary);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open file: " << DATABASE_PSQ << endl;
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

string readHeaderFromPhr(uint32_t index, const PinData& pinData) {
    ifstream file(DATABASE_PHR, ios::binary);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open file: " << DATABASE_PHR << endl;
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
