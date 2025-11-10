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

    // Dynamically configure database paths
    setDatabasePaths(dbBasePath);

    // Read .pin file
    PinData pinData;
    if (!readPinFile(pinData)) {
        cerr << "Failed to read .pin file!" << endl;
        return 1;
    }

    // Read query
    string querySequence = readQuery(queryPath);
    if (querySequence.empty()) {
        cerr << "Failed to read query sequence!" << endl;
        return 1;
    }


    bool found = false;
    uint32_t foundIndex = 0;

    for (uint32_t i = 0; i < pinData.numSequences; i++) {
        string seq = readSequenceFromPsq(i, pinData);
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

    string header = readHeaderFromPhr(foundIndex, pinData);
    size_t spacePos = header.find(' ');
    string identifier = (spacePos == string::npos) ? header : header.substr(0, spacePos);

    cout << identifier << endl;
    return 0;
}
