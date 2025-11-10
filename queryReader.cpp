#include <fstream>
#include <string>
#include <sstream>
using namespace std;

string readQuery(const string& queryPath) {
    ifstream file(queryPath);
    if (!file.is_open()) return "";

    string line, sequence;
    while (getline(file, line)) {
        if (!line.empty() && line[0] != '>')
            sequence += line;
    }
    return sequence;
}
