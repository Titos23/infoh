#ifndef BLAST_READER_H
#define BLAST_READER_H

#include <string>
#include <vector>
#include <cstdint>

// Database binary file paths
#define DATABASE_PIN "database/uniprot_sprot.fasta.pin"
#define DATABASE_PSQ "database/uniprot_sprot.fasta.psq"
#define DATABASE_PHR "database/uniprot_sprot.fasta.phr"

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
const char AMINO_ACID_TABLE[25] = {
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
    'Y',  // 21
    'Z',  // 22
    'X',  // 23
    '*'   // 24
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

#endif