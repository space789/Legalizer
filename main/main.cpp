///////////////////////////
// File: main.cpp        //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#include <iostream>
#include <string>
#include <cstdlib>  // For std::strtod
#include "Parser.h"
#include "Legalizer.h"
#include "Utilities.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: legalizer INPUT_DIR OUTPUT_DIR [-e double] [-t double]" << std::endl;
        return 1;
    }

    // Handle help argument
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") {
            std::cout << "Usage: legalizer INPUT_DIR OUTPUT_DIR [-e double] [-t double]\n";
            std::cout << "\nArguments:\n";
            std::cout << "  INPUT_DIR           Directory containing input files.\n";
            std::cout << "  OUTPUT_DIR          Directory to save output files.\n";
            std::cout << "  -e double           Optional. Set epsilon value (default: 10.0).\n";
            std::cout << "  -t double           Optional. Set timer in minutes (default: 5.0).\n";
            return 0;
        }
    }

    std::string inputPath = argv[1];
    if (inputPath.back() != '/' && inputPath.back() != '\\') {
        inputPath += '/';
    }
    std::string outputPath = argv[2];
    if (outputPath.back() != '/' && outputPath.back() != '\\') {
        outputPath += '/';
    }

    double epsilon = 10.0; // Default epsilon
    double timer = 5.0;   // Default timer in minutes

    // Parse optional arguments
    for (int i = 3; i < argc; ++i) {
        if (std::string(argv[i]) == "-e" && i + 1 < argc) {
            epsilon = std::strtod(argv[++i], nullptr);
        } else if (std::string(argv[i]) == "-t" && i + 1 < argc) {
            timer = std::strtod(argv[++i], nullptr);
        } else {
            std::cout << "Unknown argument: " << argv[i] << std::endl;
            std::cout << "Usage: legalizer INPUT_DIR OUTPUT_DIR [-e double] [-t double]" << std::endl;
            return 1;
        }
    }

    // Extract file prefix from inputPath
    std::string inputFilePrefix;
    // Remove trailing slash if any
    std::string tempPath = inputPath.substr(0, inputPath.length() - 1);
    // Find the last '/' or '\\'
    size_t pos = tempPath.find_last_of("/\\");
    if (pos != std::string::npos) {
        inputFilePrefix = tempPath.substr(pos + 1);
    } else {
        inputFilePrefix = tempPath;
    }

    // Extract output file prefix from outputPath
    std::string outputFilePrefix;
    // Remove trailing slash if any
    tempPath = outputPath.substr(0, outputPath.length() - 1);
    // Find the last '/' or '\\'
    pos = tempPath.find_last_of("/\\");
    if (pos != std::string::npos) {
        outputFilePrefix = tempPath.substr(pos + 1);
    } else {
        outputFilePrefix = tempPath;
    }

    Parser parser(inputPath, inputFilePrefix);
    parser.parse();

    Legalizer legalizer(parser.cells, parser.rows, parser.siteWidth);
    std::cout << "Legalizing..." << std::endl;
    legalizer.computeDensity(epsilon);
    legalizer.sortAndCluster();
    std::cout << "Placing cells..." << std::endl;
    legalizer.placeCells();
    std::cout << "Simulated annealing..." << std::endl;
    legalizer.simulatedAnnealing(timer); // Use the specified timer value
    legalizer.calculateDisplacement();
    legalizer.checkOverlap();

    std::cout << "Total Displacement: " << legalizer.getTotalDisplacement() << std::endl;
    std::cout << "Max Displacement: " << legalizer.getMaxDisplacement() << std::endl;

    Utilities::writeOutput(inputPath, inputFilePrefix, outputPath, outputFilePrefix, parser.cells, parser.rows);

    return 0;
}
