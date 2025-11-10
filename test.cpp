#include <iostream>
#include <vector>
#include <fstream>
#include "blastReader.h"
#include "queryReader.h"

using namespace std;




int main() {

    setDatabasePaths("database/uniprot_sprot.fasta");
    cout << "========================================" << endl;
    cout << "  BLAST READER COMPREHENSIVE TEST" << endl;
    cout << "========================================" << endl;
    cout << endl;

    // --- Read .pin file ---
    PinData pinData;
    if (!readPinFile(pinData)) {
        cerr << "Failed to read .pin file!" << endl;
        return 1;
    }

     
    // Print database information
    cout << "=== Database Information ===" << endl;
    cout << "Version: " << pinData.version << endl;
    cout << "Type: " << pinData.type << endl;
    cout << "Number of Sequences: " << pinData.numSequences << endl;
    cout << "Number of Residues: " << pinData.numResidues << endl;
    cout << "Maximum Sequence Length: " << pinData.maxSeqLength << endl;
    cout << endl;
    // --- Define test sequence indices ---
    vector<uint32_t> testIndices = {
        0, 1, 2, 3, 10, 50, 100, 500, 1000, 
        5000, 10000, 50000, 100000, 
        200000, 300000, 400000, 500000, 
        pinData.numSequences - 1  // last sequence
    };

    cout << "Testing " << testIndices.size() << " sequences..." << endl;
    cout << "========================================" << endl;
    cout << endl;

    int passCount = 0;
    int failCount = 0;

    // --- Loop through test indices ---
    for (uint32_t idx : testIndices) {
        if (idx >= pinData.numSequences) {
            cout << "Skipping index " << idx << " (out of range)" << endl;
            continue;
        }

        // === Read sequence (.psq) ===
        string seq = readSequenceFromPsq(idx, pinData);

        // Calculate expected length
        uint32_t expectedLength;
        if (idx == pinData.numSequences - 1) {
            ifstream psqFile(DATABASE_PSQ, ios::binary);
            psqFile.seekg(0, ios::end);
            uint32_t fileSize = static_cast<uint32_t>(psqFile.tellg());
            psqFile.close();
            expectedLength = fileSize - pinData.sequenceOffsets[idx] - 1;
        } else {
            expectedLength = pinData.sequenceOffsets[idx + 1] - pinData.sequenceOffsets[idx] - 1;
        }

        // --- Validate ---
        bool lengthMatches = (seq.length() == expectedLength);
        bool hasContent = (seq.length() > 0);
        bool startsWithM = (seq.length() > 0 && seq[0] == 'M');
        bool allValidChars = true;

        for (size_t i = 0; i < min<size_t>(100, seq.length()); i++) {
            char c = seq[i];
            if ((c < 'A' || c > 'Z') && c != '-' && c != '*') {
                allValidChars = false;
                break;
            }
        }

        // === Read header (.phr) ===
        string header = readHeaderFromPhr(idx, pinData);

        // Filter readable header snippet
        string headerPreview = header.empty() ? "(no readable text)" :
                               header.substr(0, min<size_t>(120, header.size()));

        // --- Print result ---
        cout << "Sequence " << idx << ": ";
        if (lengthMatches && hasContent && allValidChars) {
            cout << "✓ PASS ";
            cout << "(len=" << seq.length() 
                 << ", starts with " << seq[0] << ")";
            passCount++;
        } else {
            cout << "✗ FAIL ";
            if (!lengthMatches) cout << "(length mismatch) ";
            if (!hasContent) cout << "(empty) ";
            if (!allValidChars) cout << "(invalid chars) ";
            failCount++;
        }

        cout << endl;

        // Show sequence preview
        cout << "  Seq[0:20] = " << seq.substr(0, min<size_t>(20, seq.length())) << endl;

        // Show header preview
        cout << "  Header: " << headerPreview << endl;
        cout << "----------------------------------------" << endl;
    }

    // === Summary ===
    cout << endl;
    cout << "========================================" << endl;
    cout << "TEST RESULTS:" << endl;
    cout << "  Passed: " << passCount << endl;
    cout << "  Failed: " << failCount << endl;
    cout << "========================================" << endl;

    if (failCount == 0) {
        cout << "✓✓✓ ALL TESTS PASSED! ✓✓✓" << endl;
        return 0;
    } else {
        cout << "✗✗✗ SOME TESTS FAILED ✗✗✗" << endl;
        return 1;
    }
}
