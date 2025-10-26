#include <string>
#include <iostream>
#include "sequence_reader.h"
#include <unordered_map>
 
using namespace std;

int main(int argc, char const *argv[])
{
    string query = readQuery();
    unordered_map<string, string> database;
    string identifier = database[query];

    cout << identifier<< endl;
    return 0;
}
