#include <iostream>
#include <fstream>
using namespace std;

string readQuerry() {
    ifstream queryFile("query/P00533.fasta");
    string sequence;

    // Check if file opened successfully
    if (!queryFile.is_open()) {
        cout << "ERROR: Could not open file 'query/P00533.fasta'" << endl;
        return "";
    }
    
    string firstLine;// first line to skip
    string line; // other lines to read

    getline(queryFile, line);// get the first line that we would skip
    
    // Read other lines starting
    while(getline(queryFile, line)) {
        sequence += line;
    }

    
    queryFile.close();
    return sequence;
}

