///////////////////////////
// File: Utilities.cpp   //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#include "Utilities.h"
#include "Cell.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <sys/stat.h> // For mkdir
#include <cstring>    // For strerror

void Utilities::writeOutput(const std::string& inputPath, const std::string& inputFilePrefix, const std::string& outputPath, const std::string& outputFilePrefix,
                            const std::vector<std::shared_ptr<Cell>>& cells,
                            const std::vector<std::shared_ptr<Row>>& rows) {
    // Create output directory if it doesn't exist
    struct stat info;
    if (stat(outputPath.c_str(), &info) != 0) {
        // Directory does not exist, attempt to create it
        if (mkdir(outputPath.c_str(), 0755) != 0) {
            std::cerr << "Failed to create output directory: " << strerror(errno) << std::endl;
            return;
        }
    }

    // Write .pl file
    std::ofstream plFile(outputPath + outputFilePrefix + ".pl");
    if (!plFile.is_open()) {
        std::cerr << "Failed to open output .pl file." << std::endl;
        return;
    }

    plFile << "UCLA pl 1.0 \n" << std::endl;
    for (auto& cell : cells) {
        plFile << cell->name << "\t" << cell->x << "\t" << cell->y << " : " << cell->orientation << std::endl;
    }
    plFile.close();

    // List of files to copy
    std::vector<std::string> filesToCopy = {".nodes", ".scl", ".nets", ".wts"};

    for (const auto& extension : filesToCopy) {
        std::ifstream inFile(inputPath + inputFilePrefix + extension, std::ios::binary);
        if (!inFile.is_open()) {
            std::cerr << "Failed to open input file: " << inputPath + inputFilePrefix + extension << std::endl;
            continue;
        }

        std::ofstream outFile(outputPath + outputFilePrefix + extension, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Failed to open output file: " << outputPath + outputFilePrefix + extension << std::endl;
            inFile.close();
            continue;
        }

        outFile << inFile.rdbuf();

        inFile.close();
        outFile.close();
    }

    // Write .aux file with updated content
    std::ifstream inAuxFile(inputPath + inputFilePrefix + ".aux");
    if (!inAuxFile.is_open()) {
        std::cerr << "Failed to open input .aux file: " << inputPath + inputFilePrefix + ".aux" << std::endl;
    } else {
        std::ofstream outAuxFile(outputPath + outputFilePrefix + ".aux");
        if (!outAuxFile.is_open()) {
            std::cerr << "Failed to open output .aux file: " << outputPath + outputFilePrefix + ".aux" << std::endl;
            inAuxFile.close();
        } else {
            std::string line;
            while (std::getline(inAuxFile, line)) {
                size_t pos = line.find(inputFilePrefix);
                while (pos != std::string::npos) {
                    line.replace(pos, inputFilePrefix.length(), outputFilePrefix);
                    pos = line.find(inputFilePrefix, pos + outputFilePrefix.length());
                }
                outAuxFile << line << std::endl;
            }
            inAuxFile.close();
            outAuxFile.close();
        }
    }
}
