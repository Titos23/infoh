#include "queryReader.h"  
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

string QueryReader::readQuery(const string& queryPath) {
    ifstream file(queryPath);
    if (!file.is_open()) {
        cerr << "ERROR: Could not open query file: " << queryPath << endl;
        return "";
    }

    string line, sequence;
    while (getline(file, line)) {
        if (!line.empty() && line[0] != '>')
            sequence += line;
    }
    file.close();
    return sequence;
}