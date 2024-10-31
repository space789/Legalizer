///////////////////////////
// File: Utilities.h     //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <vector>
#include <memory>

class Cell;
class Row;

namespace Utilities {
    void writeOutput(const std::string& inputPath, const std::string& inputFilePrefix, const std::string& outputPath, const std::string& outputFilePrefix,
                     const std::vector<std::shared_ptr<Cell>>& cells,
                     const std::vector<std::shared_ptr<Row>>& rows);
}

#endif // UTILITIES_H