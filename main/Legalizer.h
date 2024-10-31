///////////////////////////
// File: Legalizer.h     //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#ifndef LEGALIZER_H
#define LEGALIZER_H

#include <vector>
#include <memory>
#include <random>

// #define DEBUG_LEGALIZER

class Cell;
class Row;
class Site;

class Legalizer {
public:
    Legalizer(std::vector<std::shared_ptr<Cell>>& cells, std::vector<std::shared_ptr<Row>>& rows, double siteWidth);
    void computeDensity(double epsilon);
    void sortAndCluster();
    void placeCells();
    void simulatedAnnealing(double maxDurationMinutes);
    void calculateDisplacement();
    void checkOverlap();

    double getTotalDisplacement() const;
    double getMaxDisplacement() const;

private:
    void attemptSwap(std::vector<std::shared_ptr<Cell>>& cluster, double temperature,
                     std::default_random_engine& generator,
                     std::uniform_real_distribution<double>& probability);
    void attemptSwapGlobal(double temperature,
                           std::default_random_engine& generator,
                           std::uniform_real_distribution<double>& probability);
    bool isSwapLegal(const std::shared_ptr<Cell>& cell1, const std::shared_ptr<Cell>& cell2);
    bool cellsOverlap(const std::shared_ptr<Cell>& cell1, const std::shared_ptr<Cell>& cell2);
    bool hasOverlapAfterSwap(const std::shared_ptr<Cell>& cell1, const std::shared_ptr<Cell>& cell2,
                                        const std::vector<std::shared_ptr<Cell>>& cellList);
    double displacement(const std::shared_ptr<Cell>& cell);
    bool acceptMove(double oldDistance, double newDistance, double temperature, double randomValue);
    double calculateTotalDisplacement(const std::vector<std::shared_ptr<Cell>>& cellList);

    std::vector<std::shared_ptr<Cell>>& cells;
    std::vector<std::shared_ptr<Row>>& rows;
    double siteWidth;

    std::vector<std::vector<std::shared_ptr<Cell>>> clusters;

    double totalDisplacement;
    double maxDisplacement;
};

#endif // LEGALIZER_H
