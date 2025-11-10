#ifndef BLAST_READER_H
#define BLAST_READER_H

#include <string>
#include <vector>
#include <cstdint>

// Database binary file paths
extern std::string DATABASE_PIN;
extern std::string DATABASE_PSQ;
extern std::string DATABASE_PHR;

void setDatabasePaths(const std::string& dbBasePath);


// Structure to hold data from .pin file
struct PinData {
    uint32_t version;
    uint32_t type;
    uint32_t numSequences;
    uint64_t numResidues;
    uint32_t maxSeqLength;
    std::vector<uint32_t> headerOffsets;
    std::vector<uint32_t> sequenceOffsets;
};

// Amino acid encoding table (index = byte value, value = amino acid letter)
// Based on NCBI BLAST Database Format documentation (Farrar, 2010)
const char AMINO_ACID_TABLE[28] = {
    '-',  // 0
    'A',  // 1
    'B',  // 2
    'C',  // 3
    'D',  // 4
    'E',  // 5
    'F',  // 6
    'G',  // 7
    'H',  // 8
    'I',  // 9
    'K',  // 10
    'L',  // 11
    'M',  // 12
    'N',  // 13
    'P',  // 14
    'Q',  // 15
    'R',  // 16
    'S',  // 17
    'T',  // 18
    'V',  // 19
    'W',  // 20
    'X',  // 21
    'Y',  // 22
    'Z',  // 23
    'U',  // 24
    '*',  // 25
    'O',  // 26
    'J'   // 27
};


/**
 * Reads the .pin file (BLAST index) and extracts metadata and offset tables
 * 
 * @return true if successful, false otherwise
 */
bool readPinFile(PinData& data);

/**
 * Reads a specific sequence from the .psq file
 * 
 * @param index The index of the sequence to read (0 to numSequences-1)
 * @param pinData The PinData containing sequence offsets
 * @return The decoded amino acid sequence as a string
 */
std::string readSequenceFromPsq(uint32_t index, const PinData& pinData);


/**
 * Reads the header (metadata) for a specific sequence from the .phr file
 *
 * @param index   The index of the sequence to read (0 to numSequences-1)
 * @param pinData The PinData containing header offsets
 * @return A human-readable string extracted from the header (approximation)
 */
std::string readHeaderFromPhr(uint32_t index, const PinData& pinData);

#endif