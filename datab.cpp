#include <fstream>
#include <iostream>
#include <cstdint>
#include <vector>
using namespace std;

int main() {

    /*In order to read binary files, we need to open the file in binary mode.
     This is done by passing ios::binary as a second argument to the ifstream constructor.
    */
    ifstream file("database/uniprot_sprot.fasta.pin", ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "Cannot open file!" << std::endl;
        return 1;
    }


    // Read version (4 bytes)

    /* The read function expects a char pointer and a size in bytes.  We use
       reinterpret_cast to convert the address of version to a char pointer.
    */
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), 4);
    
    // Need to swap from big-endian to little-endian

    /* On computer architectures that use little-endian byte order (like x86 and x86-64),
       multi-byte values are stored with the least significant byte at the lowest memory address.
       The Blast file formats use big-endian byte order, where the most significant byte is stored first.
       
       The __builtin_bswap32 function is a GCC built-in function that swaps the byte order of a 32-bit integer.
       It takes a 32-bit integer as input and returns the integer with its byte order reversed.
    */
    version = __builtin_bswap32(version);  
    cout << "Version: " << version << endl;

    // Read type (4 bytes)  
    uint32_t type;
    file.read(reinterpret_cast<char*>(&type), 4);
    type = __builtin_bswap32(type);
    cout << "Type: " << type << endl;

    // Read title length (4 bytes)
    uint32_t titleLength;
    file.read(reinterpret_cast<char*>(&titleLength), 4);
    titleLength = __builtin_bswap32(titleLength);
    cout << "Title Length: " << titleLength << endl;

    //Skip title (titleLength bytes)
    file.seekg(titleLength, ios::cur);
    
    // Read date length (4 bytes)
    uint32_t dateLength;
    file.read(reinterpret_cast<char*>(&dateLength), 4);
    dateLength = __builtin_bswap32(dateLength);
    cout << "Date Length: " << dateLength << endl;

    // Skip date (dateLength bytes)
    file.seekg(dateLength, ios::cur);

    // Read number of sequences (4 bytes)
    uint32_t numSequences;
    file.read(reinterpret_cast<char*>(&numSequences), 4);
    numSequences = __builtin_bswap32(numSequences);
    cout << "Number of Sequences: " << numSequences << endl;

    // Read number of residues (8 bytes), already in little-endian
    uint64_t numResidues;// 0
    file.read(reinterpret_cast<char*>(&numResidues), 8);
    cout << "Number of Residues: " << numResidues << endl;

    // Read maximum sequence length (4 bytes)
    uint32_t maxSeqLength;
    file.read(reinterpret_cast<char*>(&maxSeqLength), 4);
    maxSeqLength = __builtin_bswap32(maxSeqLength);
    cout << "Maximum Sequence Length: " << maxSeqLength << endl;

    // initialise two vectors to hold the header offsets and sequence offsets
    vector<uint64_t> headerOffsets(numSequences);
    vector<uint64_t> sequenceOffsets(numSequences);


    file.close();
    return 0;
}

