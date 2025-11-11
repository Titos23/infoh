#ifndef BLAST_READER_H
#define BLAST_READER_H

#include <string>
#include <vector>
#include <cstdint>

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

/**
 * BlastDatabase class for reading BLAST database files
 * Encapsulates all database reading operations
 */
class BlastDatabase {
private:
    std::string databasePinPath;
    std::string databasePsqPath;
    std::string databasePhrPath;
    PinData pinData;
    bool isLoaded;

public:
    /**
     * Constructor - sets the database base path
     * @param dbBasePath Base path to the database files (without extension)
     */
    BlastDatabase(const std::string& dbBasePath);

    /**
     * Loads the .pin file and reads metadata
     * @return true if successful, false otherwise
     */
    bool loadPinFile();

    /**
     * Reads a specific sequence from the .psq file
     * @param index The index of the sequence to read (0 to numSequences-1)
     * @return The decoded amino acid sequence as a string
     */
    std::string readSequence(uint32_t index) const;

    /**
     * Reads the header (metadata) for a specific sequence from the .phr file
     * @param index The index of the sequence to read (0 to numSequences-1)
     * @return A human-readable string extracted from the header
     */
    std::string readHeader(uint32_t index) const;

    /**
     * Gets the number of sequences in the database
     * @return Number of sequences
     */
    uint32_t getNumSequences() const { return pinData.numSequences; }

    /**
     * Checks if the database has been loaded
     * @return true if loaded, false otherwise
     */
    bool loaded() const { return isLoaded; }
};

#endif