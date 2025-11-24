#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
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
        return score > other.score;  // Descending order
    }
};

// Global progress tracking
atomic<uint32_t> processedCount(0);
atomic<uint64_t> totalCells(0);

// Progress bar (only if verbose)
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
    // Check for verbose flag
    bool verbose = false;
    if (argc >= 2 && string(argv[argc - 1]) == "-v") {
        verbose = true;
        argc--;  // Remove -v from argument count
    }

    // Check arguments
    if (argc != 6 && argc != 7) {
        cerr << "Usage: ./projet <query_fasta> <database_fasta> <blosum_matrix> <gap_open> <gap_extend> [num_threads] [-v]" << endl;
        return 1;
    }

    string queryPath = argv[1];
    string dbBasePath = argv[2];
    string blosumPath = argv[3];
    int gapOpen = stoi(argv[4]);
    int gapExtend = stoi(argv[5]);
    
    int numThreads = (argc == 7) ? stoi(argv[6]) : thread::hardware_concurrency();
    if (numThreads < 1) numThreads = 1;
    
    // Verbose header
    if (verbose) {
        cerr << "\n";
        cerr << "╔════════════════════════════════════════════════════════╗\n";
        cerr << "║     Smith-Waterman Sequence Alignment (Optimized)     ║\n";
        cerr << "╚════════════════════════════════════════════════════════╝\n";
        cerr << "\n";
        
        cerr << "Configuration:\n";
        cerr << "  Query file:       " << queryPath << "\n";
        cerr << "  Database:         " << dbBasePath << "\n";
        cerr << "  Score matrix:     " << blosumPath << "\n";
        cerr << "  Gap penalties:    Open=" << gapOpen << ", Extend=" << gapExtend << "\n";
        cerr << "  Threads:          " << numThreads << "\n";
        cerr << "\n";
    }

    // Load database
    if (verbose) cerr << "Loading database..." << flush;
    BlastDatabase database(dbBasePath);
    if (!database.loadPinFile()) {
        cerr << "\n❌ Failed to read .pin file!" << endl;
        return 1;
    }
    uint32_t totalSeqs = database.getNumSequences();
    if (verbose) {
        cerr << " ✓\n";
        cerr << "  Sequences: " << totalSeqs << "\n\n";
    }

    // Load query
    if (verbose) cerr << "Loading query sequence..." << flush;
    string querySequence = QueryReader::readQuery(queryPath);
    if (querySequence.empty()) {
        cerr << "\n❌ Failed to read query sequence!" << endl;
        return 1;
    }
    if (verbose) {
        cerr << " ✓\n";
        cerr << "  Query length: " << querySequence.length() << " amino acids\n\n";
    }

    // Load BLOSUM matrix
    if (verbose) cerr << "Loading BLOSUM matrix..." << flush;
    BlosumMatrix blosum;
    if (!blosum.load(blosumPath)) {
        cerr << "\n❌ Failed to load BLOSUM matrix!" << endl;
        return 1;
    }
    if (verbose) cerr << " ✓\n\n";

    // Calculate estimated work
    if (verbose) {
        cerr << "Calculating workload..." << flush;
        uint64_t estimatedCells = 0;
        for (uint32_t i = 0; i < min(totalSeqs, 1000u); i++) {
            estimatedCells += querySequence.length() * database.readSequence(i).length();
        }
        uint64_t totalEstimatedCells = (estimatedCells / 1000) * totalSeqs;
        cerr << " ✓\n";
        cerr << "  Estimated cells: " << fixed << setprecision(1) 
             << (totalEstimatedCells / 1e9) << " billion\n\n";
    }

    // Pre-allocate results
    vector<AlignmentResult> results(totalSeqs);
    
    // Worker function
    auto alignChunk = [&](uint32_t start, uint32_t end) {
        for (uint32_t i = start; i < end; i++) {
            string targetSeq = database.readSequence(i);
            
            int score = SmithWaterman::align(querySequence, targetSeq, blosum, gapOpen, gapExtend);
            
            results[i].identifier = database.readHeader(i);
            results[i].score = score;
            
            processedCount++;
            if (verbose) {
                totalCells += querySequence.length() * targetSeq.length();
            }
        }
    };
    
    // Start alignment
    if (verbose) cerr << "Starting alignment...\n\n";
    auto startTime = chrono::high_resolution_clock::now();
    
    // Launch threads
    vector<thread> threads;
    uint32_t chunkSize = (totalSeqs + numThreads - 1) / numThreads;
    
    for (int t = 0; t < numThreads; t++) {
        uint32_t start = t * chunkSize;
        uint32_t end = min(start + chunkSize, totalSeqs);
        
        if (start < totalSeqs) {
            threads.emplace_back(alignChunk, start, end);
        }
    }
    
    // Progress monitoring thread (only if verbose)
    thread* progressThread = nullptr;
    if (verbose) {
        progressThread = new thread([&]() {
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
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    if (progressThread) {
        progressThread->join();
        delete progressThread;
    }
    
    auto endTime = chrono::high_resolution_clock::now();
    double totalTime = chrono::duration<double>(endTime - startTime).count();
    
    if (verbose) {
        double finalGCUPS = (totalCells.load() / 1e9) / totalTime;
        
        cerr << "\n\n";
        cerr << "Alignment complete!\n\n";
        
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
    }

    // Sort results
    if (verbose) cerr << "Sorting results..." << flush;
    sort(results.begin(), results.end());
    if (verbose) cerr << " ✓\n\n";
    
    // Score distribution (only if verbose)
    if (verbose) {
        cerr << "Score distribution:\n";
        int scoreRanges[6] = {0};
        for (const auto& r : results) {
            if (r.score == 0) scoreRanges[0]++;
            else if (r.score <= 10) scoreRanges[1]++;
            else if (r.score <= 50) scoreRanges[2]++;
            else if (r.score <= 100) scoreRanges[3]++;
            else if (r.score <= 500) scoreRanges[4]++;
            else scoreRanges[5]++;
        }
        
        cerr << "  Score = 0:        " << scoreRanges[0] << " (" 
             << fixed << setprecision(1) << (100.0 * scoreRanges[0] / totalSeqs) << "%)\n";
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
        
        cerr << "╔════════════════════════════════════════════════════════╗\n";
        cerr << "║                    Top 20 Results                      ║\n";
        cerr << "╚════════════════════════════════════════════════════════╝\n";
        cerr << "\n";
    }
    
    // Output top 20 results to stdout
    int numResults = min(20, (int)results.size());
    
    if (verbose) {
        // Pretty format to stderr
        for (int i = 0; i < numResults; i++) {
            cerr << setw(2) << (i + 1) << ". " 
                 << left << setw(50) << results[i].identifier 
                 << " Score: " << right << setw(6) << results[i].score << "\n";
        }
        cerr << "\n";
    }
    
    // Simple format to stdout (always, for grading)
    for (int i = 0; i < numResults; i++) {
        cout << results[i].identifier << " " << results[i].score << "\n";
    }

    return 0;
}