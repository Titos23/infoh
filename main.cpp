#include <iostream>
#include <string>
#include "blastReader.h"
#include "queryReader.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./projetprelim <query_fasta> <database_fasta>" << endl;
        return 1;
    }

    string queryPath = argv[1];
    string dbBasePath = argv[2];

    // Create BlastDatabase instance
    BlastDatabase database(dbBasePath);
    
    // Load .pin file
    if (!database.loadPinFile()) {
        cerr << "Failed to read .pin file!" << endl;
        return 1;
    }

    // Read query using QueryReader class
    string querySequence = QueryReader::readQuery(queryPath);
    if (querySequence.empty()) {
        cerr << "Failed to read query sequence!" << endl;
        return 1;
    }

    // Search for exact match
    bool found = false;
    uint32_t foundIndex = 0;

    for (uint32_t i = 0; i < database.getNumSequences(); i++) {
        string seq = database.readSequence(i);
        if (seq == querySequence) {
            found = true;
            foundIndex = i;
            break;
        }
    }

    if (!found) {
        cout << "No exact match found in database." << endl;
        return 0;
    }

    // Read header and extract identifier
    string header = database.readHeader(foundIndex);
    size_t spacePos = header.find(' ');
    string identifier = (spacePos == string::npos) ? header : header.substr(0, spacePos);

    cout << identifier << endl;
    return 0;
}
