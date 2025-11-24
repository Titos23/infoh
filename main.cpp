#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "blastDatabase.h"
#include "queryReader.h"
#include "blosumMatrix.h"
#include "smithWaterman.h"

using namespace std;

// Structure to hold alignment results
struct AlignmentResult {
    string identifier;
    int score;
};

int main(int argc, char* argv[]) {
    // Check arguments
    if (argc != 6) {
        cerr << "Usage: ./projet <query_fasta> <database_fasta> <blosum_matrix> <gap_open> <gap_extend>" << endl;
        return 1;
    }

    string queryPath = argv[1];
    string dbBasePath = argv[2];
    string blosumPath = argv[3];
    int gapOpen = stoi(argv[4]);
    int gapExtend = stoi(argv[5]);

    // Load database
    BlastDatabase database(dbBasePath);
    if (!database.loadPinFile()) {
        cerr << "Failed to read .pin file!" << endl;
        return 1;
    }

    // Load query
    string querySequence = QueryReader::readQuery(queryPath);
    if (querySequence.empty()) {
        cerr << "Failed to read query sequence!" << endl;
        return 1;
    }

    // Load BLOSUM matrix
    BlosumMatrix blosum;
    if (!blosum.load(blosumPath)) {
        cerr << "Failed to load BLOSUM matrix!" << endl;
        return 1;
    }

    // Align query with all sequences in database
    vector<AlignmentResult> results;
    
    for (uint32_t i = 0; i < database.getNumSequences(); i++) {
        string targetSeq = database.readSequence(i);
        
        // Calculate alignment score
        int score = SmithWaterman::align(querySequence, targetSeq, blosum, gapOpen, gapExtend);
        
        // Store result
        AlignmentResult result;
        result.identifier = database.readHeader(i);
        result.score = score;
        results.push_back(result);
    }

    // Sort by score (descending)
    sort(results.begin(), results.end(), 
         [](const AlignmentResult& a, const AlignmentResult& b) {
             return a.score > b.score;
         });

    // Output top 20 results
    int numResults = min(20, (int)results.size());
    for (int i = 0; i < numResults; i++) {
        cout << results[i].identifier << " " << results[i].score << endl;
    }

    return 0;
}