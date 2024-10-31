///////////////////////////
// File: Parser.h        //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <memory>

// #define DEBUG_PARSER

class Cell;
class Row;

class Parser {
public:
    Parser(const std::string& inputPath, const std::string& filePrefix);
    void parse();

    std::vector<std::shared_ptr<Cell>> cells;
    std::vector<std::shared_ptr<Row>> rows;
    double siteWidth;
    double siteHeight;

private:
    std::string inputPath;
    std::string filePrefix;
    void parseAux();
    void parseNodes();
    void parsePl();
    void parseScl();

    // Filenames
    std::string nodesFile;
    std::string plFile;
    std::string sclFile;
    // Add other file types if necessary
};

#endif // PARSER_H
