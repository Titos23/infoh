#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iomanip>
#include "blastDatabase.h"
#include "queryReader.h"
#include "blosumMatrix.h"
#include "smithWaterman.h"

using namespace std;

struct AlignmentResult {
    string identifier;
    int score;
    
    bool operator<(const AlignmentResult& other) const {
        return score > other.score;
    }
};

// Global progress tracking
atomic<uint32_t> processedCount(0);
atomic<uint64_t> totalCells(0);

// Function to draw progress bar
void drawProgressBar(uint32_t current, uint32_t total, 
                     double elapsed, double gcups) {
    const int barWidth = 50;
    float progress = (float)current / total;
    int pos = barWidth * progress;
    
    cerr << "\r[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) cerr << "=";
        else if (i == pos) cerr << ">";
        else cerr << " ";
    }
    cerr << "] " << int(progress * 100.0) << "% ";
    cerr << current << "/" << total << " seqs ";
    cerr << "| Time: " << fixed << setprecision(1) << elapsed << "s ";
    cerr << "| Speed: " << fixed << setprecision(2) << gcups << " GCUPS";
    cerr << flush;
}

int main(int argc, char* argv[]) {
    cerr << "\n";
    cerr << "╔════════════════════════════════════════════════════════╗\n";
    cerr << "║     Smith-Waterman Sequence Alignment (Optimized)     ║\n";
    cerr << "╚════════════════════════════════════════════════════════╝\n";
    cerr << "\n";

    // Check arguments
    if (argc != 6 && argc != 7) {
        cerr << "Usage: ./projet <query_fasta> <database_fasta> <blosum_matrix> <gap_open> <gap_extend> [num_threads]" << endl;
        return 1;
    }

    string queryPath = argv[1];
    string dbBasePath = argv[2];
    string blosumPath = argv[3];
    int gapOpen = stoi(argv[4]);
    int gapExtend = stoi(argv[5]);
    
    int numThreads = (argc == 7) ? stoi(argv[6]) : thread::hardware_concurrency();
    if (numThreads < 1) numThreads = 1;
    
    // Display configuration
    cerr << "Configuration:\n";
    cerr << "  Query file:       " << queryPath << "\n";
    cerr << "  Database:         " << dbBasePath << "\n";
    cerr << "  Score matrix:     " << blosumPath << "\n";
    cerr << "  Gap penalties:    Open=" << gapOpen << ", Extend=" << gapExtend << "\n";
    cerr << "  Threads:          " << numThreads << "\n";
    cerr << "\n";

    // Load database
    cerr << "Loading database..." << flush;
    BlastDatabase database(dbBasePath);
    if (!database.loadPinFile()) {
        cerr << "\n❌ Failed to read .pin file!" << endl;
        return 1;
    }
    uint32_t totalSeqs = database.getNumSequences();
    cerr << " ✓\n";
    cerr << "  Sequences: " << totalSeqs << "\n\n";

    // Load query
    cerr << "Loading query sequence..." << flush;
    string querySequence = QueryReader::readQuery(queryPath);
    if (querySequence.empty()) {
        cerr << "\n❌ Failed to read query sequence!" << endl;
        return 1;
    }
    cerr << " ✓\n";
    cerr << "  Query length: " << querySequence.length() << " amino acids\n\n";

    // Load BLOSUM matrix
    cerr << "Loading BLOSUM matrix..." << flush;
    BlosumMatrix blosum;
    if (!blosum.load(blosumPath)) {
        cerr << "\n❌ Failed to load BLOSUM matrix!" << endl;
        return 1;
    }
    cerr << " ✓\n\n";

    // Calculate estimated work
    cerr << "Calculating workload..." << flush;
    uint64_t estimatedCells = 0;
    for (uint32_t i = 0; i < min(totalSeqs, 1000u); i++) {
        estimatedCells += querySequence.length() * database.readSequence(i).length();
    }
    uint64_t totalEstimatedCells = (estimatedCells / 1000) * totalSeqs;
    cerr << " ✓\n";
    cerr << "  Estimated cells: " << (totalEstimatedCells / 1e9) << " billion\n\n";

    // Pre-allocate results
    vector<AlignmentResult> results(totalSeqs);
    
    // Worker function
    auto alignChunk = [&](uint32_t start, uint32_t end, int threadId) {
        for (uint32_t i = start; i < end; i++) {
            string targetSeq = database.readSequence(i);
            
            // Calculate score
            int score = SmithWaterman::align(querySequence, targetSeq, blosum, gapOpen, gapExtend);
            
            // Store result
            results[i].identifier = database.readHeader(i);
            results[i].score = score;
            
            // Update counters
            processedCount++;
            totalCells += querySequence.length() * targetSeq.length();
        }
    };
    
    // Start alignment
    cerr << "Starting alignment...\n\n";
    auto startTime = chrono::high_resolution_clock::now();
    
    // Launch threads
    vector<thread> threads;
    uint32_t chunkSize = (totalSeqs + numThreads - 1) / numThreads;
    
    for (int t = 0; t < numThreads; t++) {
        uint32_t start = t * chunkSize;
        uint32_t end = min(start + chunkSize, totalSeqs);
        
        if (start < totalSeqs) {
            threads.emplace_back(alignChunk, start, end, t);
        }
    }
    
    // Progress monitoring thread
    thread progressThread([&]() {
        while (processedCount < totalSeqs) {
            auto now = chrono::high_resolution_clock::now();
            double elapsed = chrono::duration<double>(now - startTime).count();
            double gcups = (totalCells.load() / 1e9) / max(elapsed, 0.001);
            
            drawProgressBar(processedCount, totalSeqs, elapsed, gcups);
            
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        
        // Final update
        auto now = chrono::high_resolution_clock::now();
        double elapsed = chrono::duration<double>(now - startTime).count();
        double gcups = (totalCells.load() / 1e9) / max(elapsed, 0.001);
        drawProgressBar(totalSeqs, totalSeqs, elapsed, gcups);
    });
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    progressThread.join();
    
    auto endTime = chrono::high_resolution_clock::now();
    double totalTime = chrono::duration<double>(endTime - startTime).count();
    double finalGCUPS = (totalCells.load() / 1e9) / totalTime;
    
    cerr << "\n\n";
    cerr << "Alignment complete!\n\n";
    
    // Display statistics
    cerr << "Statistics:\n";
    cerr << "  Total sequences:  " << totalSeqs << "\n";
    cerr << "  Total cells:      " << fixed << setprecision(2) 
         << (totalCells.load() / 1e9) << " billion\n";
    cerr << "  Total time:       " << fixed << setprecision(2) 
         << totalTime << " seconds\n";
    cerr << "  Performance:      " << fixed << setprecision(2) 
         << finalGCUPS << " GCUPS\n";
    cerr << "  Speed per core:   " << fixed << setprecision(2) 
         << (finalGCUPS / numThreads) << " GCUPS/core\n";
    cerr << "\n";

    // Sort results
    cerr << "Sorting results..." << flush;
    sort(results.begin(), results.end());
    cerr << " ✓\n\n";
    
    // Analyze score distribution
    cerr << "Score distribution:\n";
    int scoreRanges[6] = {0}; // 0, 1-10, 11-50, 51-100, 101-500, 500+
    for (const auto& r : results) {
        if (r.score == 0) scoreRanges[0]++;
        else if (r.score <= 10) scoreRanges[1]++;
        else if (r.score <= 50) scoreRanges[2]++;
        else if (r.score <= 100) scoreRanges[3]++;
        else if (r.score <= 500) scoreRanges[4]++;
        else scoreRanges[5]++;
    }
    
    cerr << "  Score = 0:        " << scoreRanges[0] << " (" 
         << (100.0 * scoreRanges[0] / totalSeqs) << "%)\n";
    cerr << "  Score 1-10:       " << scoreRanges[1] << " (" 
         << (100.0 * scoreRanges[1] / totalSeqs) << "%)\n";
    cerr << "  Score 11-50:      " << scoreRanges[2] << " (" 
         << (100.0 * scoreRanges[2] / totalSeqs) << "%)\n";
    cerr << "  Score 51-100:     " << scoreRanges[3] << " (" 
         << (100.0 * scoreRanges[3] / totalSeqs) << "%)\n";
    cerr << "  Score 101-500:    " << scoreRanges[4] << " (" 
         << (100.0 * scoreRanges[4] / totalSeqs) << "%)\n";
    cerr << "  Score > 500:      " << scoreRanges[5] << " (" 
         << (100.0 * scoreRanges[5] / totalSeqs) << "%)\n";
    cerr << "\n";

    // Output top 20 to stderr (for display)
    cerr << "╔════════════════════════════════════════════════════════╗\n";
    cerr << "║                    Top 20 Results                      ║\n";
    cerr << "╚════════════════════════════════════════════════════════╝\n";
    cerr << "\n";
    
    int numResults = min(20, (int)results.size());
    for (int i = 0; i < numResults; i++) {
        cerr << setw(2) << (i + 1) << ". " 
             << left << setw(50) << results[i].identifier 
             << " Score: " << right << setw(6) << results[i].score << "\n";
    }
    
    cerr << "\n";

    // Output top 20 to stdout (required format for grading)
    for (int i = 0; i < numResults; i++) {
        cout << results[i].identifier << " " << results[i].score << "\n";
    }

    return 0;
}

