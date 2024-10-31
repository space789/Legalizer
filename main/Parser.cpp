///////////////////////////
// File: Parser.cpp      //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#include "Parser.h"
#include "Cell.h"
#include "Row.h"
#include "Site.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>

Parser::Parser(const std::string& inputPath, const std::string& filePrefix)
    : siteWidth(1.0), siteHeight(1.0), inputPath(inputPath), filePrefix(filePrefix) {
    nodesFile = filePrefix + ".nodes";
    plFile = filePrefix + ".pl";
    sclFile = filePrefix + ".scl";
    // Initialize other file names if necessary
}

void Parser::parse() {
    parseAux();
    parseNodes();
    parsePl();
    parseScl();
}

void Parser::parseAux() {
    std::ifstream file(inputPath + filePrefix + ".aux");
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filePrefix << ".aux file." << std::endl;
        // Proceed with default filenames
        return;
    }

    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword; // Skip the first keyword
        std::string filename;
        while (iss >> filename) {
            if (filename.find(".nodes") != std::string::npos) {
                nodesFile = filename;
            } else if (filename.find(".pl") != std::string::npos) {
                plFile = filename;
            } else if (filename.find(".scl") != std::string::npos) {
                sclFile = filename;
            }
            // Add handling for other file types if necessary
        }
    }
    file.close();
}

void Parser::parseNodes() {
    std::ifstream file(inputPath + nodesFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << nodesFile << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip the header lines
        if (line.find("UCLA nodes 1.0") != std::string::npos ||
            line.find("NumNodes") != std::string::npos ||
            line.find("NumTerminals") != std::string::npos) {
            continue;
        }

        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string name;
        double width, height;
        iss >> name >> width >> height;
        auto cell = std::make_shared<Cell>(name, width, height);
        cells.push_back(cell);
    }
    file.close();
}

void Parser::parsePl() {
    std::ifstream file(inputPath + plFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << plFile << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line.find("UCLA") == 0) continue;

        // Remove ':' from the line
        line.erase(std::remove(line.begin(), line.end(), ':'), line.end());
        
        std::istringstream iss(line);
        std::string name;
        double x, y;
        std::string orientation;
        iss >> name >> x >> y >> orientation;
        for (auto& cell : cells) {
            if (cell->name == name) {
                cell->x = x;
                cell->y = y;
                cell->originalX = x;
                cell->originalY = y;
                cell->orientation = orientation;
                break;
            }
        }
    }
    file.close();
#ifdef DEBUG_PARSER
    std::cout << "Parsed " << cells.size() << " cells." << std::endl;
    for (const auto& cell : cells) {
        std::cout << cell->name << " " << cell->width << " " << cell->height << " " << cell->x << " " << cell->y
                  << " " << cell->isFixed << " " << cell->orientation << std::endl;
    }
#endif
}

void Parser::parseScl() {
    std::ifstream file(inputPath + sclFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << sclFile << std::endl;
        return;
    }

    std::string line;
    bool inCoreRow = false;
    std::shared_ptr<Row> currentRow = nullptr;
    while (std::getline(file, line)) {
        // Trim leading and trailing whitespace from the line
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), line.end());
        
        if (line.find("CoreRow") != std::string::npos) {
            inCoreRow = true;
            currentRow = std::make_shared<Row>(0, 0, siteWidth, 0);
            continue;
        }
        if (inCoreRow) {
            if (line.find("End") != std::string::npos) {
                rows.push_back(currentRow);
                inCoreRow = false;
                continue;
            }
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string keyword = line.substr(0, colonPos);
                std::string valueStr = line.substr(colonPos + 1);
                keyword.erase(std::remove_if(keyword.begin(), keyword.end(), ::isspace), keyword.end());
                valueStr.erase(std::remove_if(valueStr.begin(), valueStr.end(), ::isspace), valueStr.end());

                try {
                    if (keyword == "Coordinate") {
                        currentRow->originY = std::stod(valueStr);
                    } else if (keyword == "Height") {
                        currentRow->height = std::stod(valueStr);
                        siteHeight = currentRow->height;
                    } else if (keyword == "Sitewidth") {
                        currentRow->siteWidth = std::stod(valueStr);
                        siteWidth = currentRow->siteWidth;
                    } else if (keyword == "SubrowOrigin") {
                        double x = 0;
                        int siteCount = 0;

                        // Use regex to extract SubrowOrigin and NumSites (case-insensitive)
                        std::regex re("\\s*SubrowOrigin\\s*:\\s*(-?\\d+)\\s*Num[Ss]ites\\s*:\\s*(-?\\d+)");
                        std::smatch match;
                        if (std::regex_search(line, match, re)) {
                            if (match.size() == 3) { // match[0] is the full match, match[1] is x, match[2] is siteCount
                                x = std::stod(match[1].str());
                                siteCount = std::stoi(match[2].str());
                            } else {
                                throw std::invalid_argument("Failed to extract SubrowOrigin and NumSites.");
                            }
                        } else {
                            throw std::invalid_argument("Invalid format for SubrowOrigin line.");
                        }

                        currentRow->originX = x;
                        currentRow->siteCount = siteCount;
                        // Initialize sites in the row
                        currentRow->sites.clear();
                        for (int i = 0; i < siteCount; ++i) {
                            currentRow->sites.push_back(std::make_shared<Site>(x + i * siteWidth, currentRow->originY));
                        }
                    }
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Error parsing line: " << line << "\nReason: " << e.what() << std::endl;
                    continue;
                }
            }
        }
    }
    file.close();
#ifdef DEBUG_PARSER
    std::cout << "Parsed " << rows.size() << " rows." << std::endl;
    for (const auto& row : rows) {
        std::cout << row->originX << " " << row->originY << " " << row->height << " " << row->siteWidth << " "
                  << row->siteCount << std::endl;
    }
#endif
}
