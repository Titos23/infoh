#ifndef QUERY_READER_H
#define QUERY_READER_H

#include <string>

/**
 * QueryReader class for reading query sequences from FASTA files
 */
class QueryReader {
public:
    /**
     * Reads a query sequence from a FASTA file
     * @param queryPath Path to the FASTA file
     * @return The sequence as a string (empty if failed)
     */
    static std::string readQuery(const std::string& queryPath);
};

#endif