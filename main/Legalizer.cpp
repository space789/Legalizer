///////////////////////////
// File: Legalizer.cpp   //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#include "Legalizer.h"
#include "Cell.h"
#include "Row.h"
#include "Site.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>
#include <iostream>

Legalizer::Legalizer(std::vector<std::shared_ptr<Cell>>& cells, std::vector<std::shared_ptr<Row>>& rows, double siteWidth)
    : cells(cells), rows(rows), siteWidth(siteWidth), totalDisplacement(0), maxDisplacement(0) {}

void Legalizer::computeDensity(double epsilon) {
    // Simple density calculation without hash tables
    for (auto& cell : cells) {
        cell->density = 0;
        for (auto& otherCell : cells) {
            if (cell == otherCell) continue;
            double dx = cell->x - otherCell->x;
            double dy = cell->y - otherCell->y;
            double distance = std::sqrt(dx * dx + dy * dy);
            if (distance <= epsilon * siteWidth) {
                cell->density++;
            }
        }
    }
#ifdef DEBUG_LEGALIZER
    for (const auto& cell : cells) {
        std::cout << cell->name << " " << cell->density << std::endl;
    }
#endif
}

void Legalizer::sortAndCluster() {
    // Separate cells that are outside the chip boundary
    std::vector<std::shared_ptr<Cell>> outOfBoundsCells;

    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();

    for (const auto& row : rows) {
        double rowMaxX = row->originX + row->siteWidth * row->siteCount;
        double rowMaxY = row->originY + row->height;

        if (row->originX < minX) minX = row->originX;
        if (row->originY < minY) minY = row->originY;
        if (rowMaxX > maxX) maxX = rowMaxX;
        if (rowMaxY > maxY) maxY = rowMaxY;
    }

#ifdef DEBUG_LEGALIZER
    std::cout << "Chip boundary: " << minX << " " << minY << " " << maxX << " " << maxY << std::endl;
#endif

    for (auto& cell : cells) {
        bool outOfBounds = false;
        if (cell->originalX < minX || cell->originalX + cell->width > maxX ||
            cell->originalY < minY || cell->originalY + cell->height > maxY) {
            outOfBounds = true;
#ifdef DEBUG_LEGALIZER
            std::cout << "Cell " << cell->name << " is out of bounds" << std::endl;
            // std::cout << "Cell: " << cell->originalX << " " << cell->originalY << " " << cell->width << " " << cell->height << std::endl;
#endif
        }
        if (outOfBounds) {
            outOfBoundsCells.push_back(cell);
        }
    }

    // Create a special cluster for out-of-bounds cells
    if (!outOfBoundsCells.empty()) {
        clusters.push_back(outOfBoundsCells);
    }

    // Sort remaining cells by density descending
    std::sort(cells.begin(), cells.end(), [](const std::shared_ptr<Cell>& a, const std::shared_ptr<Cell>& b) {
        return a->density > b->density;
    });

    // Cluster remaining cells with similar density and positions
    const int densityThreshold = 3; // Define a suitable threshold
    for (auto& cell : cells) {
        if (std::find(outOfBoundsCells.begin(), outOfBoundsCells.end(), cell) != outOfBoundsCells.end()) continue;

        bool added = false;
        for (auto& cluster : clusters) {
            if (!cluster.empty()) {
                double dx = cell->x - cluster.front()->x;
                double dy = cell->y - cluster.front()->y;
                double distance = std::sqrt(dx * dx + dy * dy);
                if (std::abs(cell->density - cluster.front()->density) <= densityThreshold && distance <= 2 * siteWidth) {
                    cluster.push_back(cell);
                    added = true;
                    break;
                }
            }
        }
        if (!added) {
            clusters.push_back({cell});
        }
    }

    // Within each cluster, sort cells by width ascending
    for (auto& cluster : clusters) {
        std::sort(cluster.begin(), cluster.end(), [](const std::shared_ptr<Cell>& a, const std::shared_ptr<Cell>& b) {
            return a->width < b->width;
        });
    }

    // print first cluster
    // for (auto& cell : clusters[0]) {
    //     std::cout << cell->name << " " << cell->density << std::endl;
    // }

#ifdef DEBUG_LEGALIZER
    std::cout << "Number of clusters: " << clusters.size() << std::endl;
#endif
}

void Legalizer::placeCells() {
    // Place cells starting from highest density cluster
    for (auto& cluster : clusters) {
        for (auto& cell : cluster) {
            if (cell->isFixed) continue;
            double minDistance = std::numeric_limits<double>::max();
            std::shared_ptr<Site> bestSite = nullptr;

            // Scan all rows and sites
            for (auto& row : rows) {
                for (auto& site : row->sites) {
                    if (site->isOccupied) continue;
                    if ((site->x + cell->width) > (row->originX + row->siteWidth * row->siteCount)) continue;

                    // Checking cell width don't is not Occupied
                    bool isOccupied = false;

                    for (int i = 0; i < static_cast<int>(std::ceil(cell->width / siteWidth)); ++i) {
                        if (row->sites[static_cast<int>((site->x - row->originX) / siteWidth) + i]->isOccupied) {
                            isOccupied = true;
                            break;
                        }
                    }

                    if (isOccupied) continue;

                    double distance = std::abs(site->x - cell->originalX) + std::abs(site->y - cell->originalY);
                    if (distance < minDistance) {
                        minDistance = distance;
                        bestSite = site;
                    }
                }
            }

            if (bestSite) {
                cell->x = bestSite->x;
                cell->y = bestSite->y;
                bestSite->isOccupied = true;
                bestSite->cell = cell;
                // Occupy subsequent sites if cell width > site width
                int sitesNeeded = static_cast<int>(std::ceil(cell->width / siteWidth));

                // Find the correct row for the cell
                std::shared_ptr<Row> row = nullptr;
                for (auto& r : rows) {
                    if (r->originY == bestSite->y) {
                        row = r;
                        break;
                    }
                }

                int siteIndex = static_cast<int>((bestSite->x - row->originX) / siteWidth);
                for (int i = 0; i < sitesNeeded; ++i) {
                    if (siteIndex + i < static_cast<int>(row->sites.size())) {
                        row->sites[siteIndex + i]->isOccupied = true;
                        row->sites[siteIndex + i]->cell = cell;
                    }
                }
            } else {
                std::cerr << "Failed to find placement for cell: " << cell->name << std::endl;
            }
        }
    }
}

void Legalizer::simulatedAnnealing(double maxDurationMinutes) {
    // Convert maxDurationMinutes to milliseconds
    auto maxDuration = std::chrono::milliseconds(static_cast<int>(maxDurationMinutes * 60 * 1000));
    auto startTime = std::chrono::steady_clock::now();

    // Initialize best global solution
    std::vector<std::pair<double, double>> bestPositionsGlobal;
    double bestTotalDisplacementGlobal = calculateTotalDisplacement(cells);
    for (const auto& cell : cells) {
        bestPositionsGlobal.emplace_back(cell->x, cell->y);
    }

    int totalProgress = static_cast<int>(maxDurationMinutes * 60);
    int currentProgress = 0;

    while (true) {
        // Check if time limit has been reached
        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime - startTime >= maxDuration) {
            break;
        }

        // Progress bar
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        if (elapsedTime > currentProgress) {
            currentProgress = static_cast<int>(elapsedTime);
            int width = 50;
            int pos = (currentProgress * width) / totalProgress;
            std::cout << "[";
            for (int i = 0; i < width; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << (currentProgress * 100) / totalProgress << "%\r";
            std::cout.flush();
        }

        // Simulated annealing within each cluster
        for (auto& cluster : clusters) {
            double temperature = 1000.0;
            double coolingRate = 0.99; // Adjusted cooling rate for better convergence

            std::default_random_engine generator(std::random_device{}());
            std::uniform_real_distribution<double> probability(0.0, 1.0);

            // Initialize best solution for the cluster
            std::vector<std::pair<double, double>> bestPositionsCluster;
            double bestTotalDisplacementCluster = calculateTotalDisplacement(cluster);
            for (const auto& cell : cluster) {
                bestPositionsCluster.emplace_back(cell->x, cell->y);
            }

            while (temperature > 1) {
                for (int iter = 0; iter < 1000; ++iter) {
                    attemptSwap(cluster, temperature, generator, probability);

                    // Calculate current total displacement
                    double currentTotalDisplacement = calculateTotalDisplacement(cluster);
                    if (currentTotalDisplacement < bestTotalDisplacementCluster) {
                        // Update best solution for the cluster
                        bestTotalDisplacementCluster = currentTotalDisplacement;
                        for (size_t i = 0; i < cluster.size(); ++i) {
                            bestPositionsCluster[i].first = cluster[i]->x;
                            bestPositionsCluster[i].second = cluster[i]->y;
                        }
                    }
                }
                temperature *= coolingRate;
            }

            // Restore best solution for the cluster
            for (size_t i = 0; i < cluster.size(); ++i) {
                cluster[i]->x = bestPositionsCluster[i].first;
                cluster[i]->y = bestPositionsCluster[i].second;
            }
        }

        // Global simulated annealing
        double temperature = 1000.0;
        double coolingRate = 0.99;

        std::default_random_engine generator(std::random_device{}());
        std::uniform_real_distribution<double> probability(0.0, 1.0);

        // Initialize best solution for global SA
        std::vector<std::pair<double, double>> bestPositions;
        double bestTotalDisplacement = calculateTotalDisplacement(cells);
        for (const auto& cell : cells) {
            bestPositions.emplace_back(cell->x, cell->y);
        }

        while (temperature > 1) {
            for (int iter = 0; iter < 1000; ++iter) {
                attemptSwapGlobal(temperature, generator, probability);

                // Calculate current total displacement
                double currentTotalDisplacement = calculateTotalDisplacement(cells);
                if (currentTotalDisplacement < bestTotalDisplacement) {
                    // Update best global solution
                    bestTotalDisplacement = currentTotalDisplacement;
                    for (size_t i = 0; i < cells.size(); ++i) {
                        bestPositions[i].first = cells[i]->x;
                        bestPositions[i].second = cells[i]->y;
                    }
                }
            }
            temperature *= coolingRate;
        }

        // Restore best global solution
        for (size_t i = 0; i < cells.size(); ++i) {
            cells[i]->x = bestPositions[i].first;
            cells[i]->y = bestPositions[i].second;
        }

        // Update the best solution across all iterations
        double currentTotalDisplacementGlobal = calculateTotalDisplacement(cells);
        if (currentTotalDisplacementGlobal < bestTotalDisplacementGlobal) {
            bestTotalDisplacementGlobal = currentTotalDisplacementGlobal;
            for (size_t i = 0; i < cells.size(); ++i) {
                bestPositionsGlobal[i].first = cells[i]->x;
                bestPositionsGlobal[i].second = cells[i]->y;
            }
        } else {
            // Restore the best global solution
            for (size_t i = 0; i < cells.size(); ++i) {
                cells[i]->x = bestPositionsGlobal[i].first;
                cells[i]->y = bestPositionsGlobal[i].second;
            }
        }
    }

    // After time limit, ensure the best global solution is restored
    for (size_t i = 0; i < cells.size(); ++i) {
        cells[i]->x = bestPositionsGlobal[i].first;
        cells[i]->y = bestPositionsGlobal[i].second;
    }

    std::cout << "[==================================================] 100%" << std::endl;
}

void Legalizer::attemptSwap(std::vector<std::shared_ptr<Cell>>& cluster, double temperature,
                            std::default_random_engine& generator,
                            std::uniform_real_distribution<double>& probability) {
    std::uniform_int_distribution<int> cell_distribution(0, cluster.size() - 1);

    int idx1 = cell_distribution(generator);
    int idx2 = cell_distribution(generator);
    if (idx1 == idx2) return;

    auto& cell1 = cluster[idx1];
    auto& cell2 = cluster[idx2];

    // Only allow swapping cells if widths are equal
    if (std::abs(cell1->width - cell2->width) > siteWidth * 0.1) {
        return;
    }

    // Check if swap is legal (no overlap)
    if (isSwapLegal(cell1, cell2)) {
        double oldDistance = displacement(cell1) + displacement(cell2);

        // Swap positions
        std::swap(cell1->x, cell2->x);
        std::swap(cell1->y, cell2->y);

        // Check for overlaps after swapping
        if (!hasOverlapAfterSwap(cell1, cell2, cluster)) {
            double newDistance = displacement(cell1) + displacement(cell2);

            if (acceptMove(oldDistance, newDistance, temperature, probability(generator))) {
                // Accept swap
            } else {
                // Revert swap
                std::swap(cell1->x, cell2->x);
                std::swap(cell1->y, cell2->y);
            }
        } else {
            // Revert swap due to overlap
            std::swap(cell1->x, cell2->x);
            std::swap(cell1->y, cell2->y);
        }
    }
}

void Legalizer::attemptSwapGlobal(double temperature,
                                  std::default_random_engine& generator,
                                  std::uniform_real_distribution<double>& probability) {
    std::uniform_int_distribution<int> cell_distribution(0, cells.size() - 1);

    int idx1 = cell_distribution(generator);
    int idx2 = cell_distribution(generator);
    if (idx1 == idx2) return;

    auto& cell1 = cells[idx1];
    auto& cell2 = cells[idx2];

    // Only allow swapping cells if widths are equal
    if (std::abs(cell1->width - cell2->width) > siteWidth * 0.1) {
        return;
    }

    // Check if swap is legal
    if (isSwapLegal(cell1, cell2)) {
        double oldDistance = displacement(cell1) + displacement(cell2);

        // Swap positions
        std::swap(cell1->x, cell2->x);
        std::swap(cell1->y, cell2->y);

        // Check for overlaps after swapping
        if (!hasOverlapAfterSwap(cell1, cell2, cells)) {
            double newDistance = displacement(cell1) + displacement(cell2);

            if (acceptMove(oldDistance, newDistance, temperature, probability(generator))) {
                // Accept swap
            } else {
                // Revert swap
                std::swap(cell1->x, cell2->x);
                std::swap(cell1->y, cell2->y);
            }
        } else {
            // Revert swap due to overlap
            std::swap(cell1->x, cell2->x);
            std::swap(cell1->y, cell2->y);
        }
    }
}

bool Legalizer::hasOverlapAfterSwap(const std::shared_ptr<Cell>& cell1, const std::shared_ptr<Cell>& cell2,
                                    const std::vector<std::shared_ptr<Cell>>& cellList) {
    // Check for overlaps between swapped cells and other cells
    for (const auto& otherCell : cellList) {
        if (otherCell == cell1 || otherCell == cell2) continue;
        if (cellsOverlap(cell1, otherCell) || cellsOverlap(cell2, otherCell)) {
            return true;
        }
    }
    // Also check overlap between cell1 and cell2 (should not overlap since they swapped positions)
    return false;
}


bool Legalizer::isSwapLegal(const std::shared_ptr<Cell>& cell1, const std::shared_ptr<Cell>& cell2) {
    // Ensure cells will not overlap after swapping
    // Check if the target positions are available
    // For simplicity, assume positions are legal if they don't cause overlap
    return !cellsOverlap(cell1, cell2);
}

bool Legalizer::cellsOverlap(const std::shared_ptr<Cell>& cell1, const std::shared_ptr<Cell>& cell2) {
    // Check if two cells overlap
    double x1_min = cell1->x;
    double x1_max = cell1->x + cell1->width;
    double y1_min = cell1->y;
    double y1_max = cell1->y + cell1->height;

    double x2_min = cell2->x;
    double x2_max = cell2->x + cell2->width;
    double y2_min = cell2->y;
    double y2_max = cell2->y + cell2->height;

    bool overlapX = x1_min < x2_max && x1_max > x2_min;
    bool overlapY = y1_min < y2_max && y1_max > y2_min;

    return overlapX && overlapY;
}

double Legalizer::displacement(const std::shared_ptr<Cell>& cell) {
    return std::abs(cell->x - cell->originalX) + std::abs(cell->y - cell->originalY);
}

bool Legalizer::acceptMove(double oldDistance, double newDistance, double temperature, double randomValue) {
    if (newDistance < oldDistance) {
        return true;
    } else {
        double acceptanceProbability = std::exp((oldDistance - newDistance) / temperature);
        return randomValue < acceptanceProbability;
    }
}

void Legalizer::calculateDisplacement() {
    totalDisplacement = 0;
    maxDisplacement = 0;
    std::shared_ptr<Cell> maxDisplacementCell = nullptr;

    for (auto& cell : cells) {
        double displacement = std::abs(cell->x - cell->originalX) + std::abs(cell->y - cell->originalY);
        totalDisplacement += displacement;
        if (displacement > maxDisplacement) {
            maxDisplacement = displacement;
            maxDisplacementCell = cell;
        }
    }

#ifdef DEBUG_LEGALIZER
    if (maxDisplacementCell) {
        std::cout << "Cell with maximum displacement: " << maxDisplacementCell->name << std::endl;
        std::cout << "Original position: (" << maxDisplacementCell->originalX << ", " << maxDisplacementCell->originalY << ")" << std::endl;
        std::cout << "Placed position: (" << maxDisplacementCell->x << ", " << maxDisplacementCell->y << ")" << std::endl;
        std::cout << "Displacement: " << maxDisplacement << std::endl;
    }
#endif
}


double Legalizer::getTotalDisplacement() const {
    return totalDisplacement;
}

double Legalizer::getMaxDisplacement() const {
    return maxDisplacement;
}


double Legalizer::calculateTotalDisplacement(const std::vector<std::shared_ptr<Cell>>& cellList) {
    double totalDisplacement = 0.0;
    for (const auto& cell : cellList) {
        totalDisplacement += displacement(cell);
    }
    return totalDisplacement;
}

void Legalizer::checkOverlap() {
    for (size_t i = 0; i < cells.size(); ++i) {
        for (size_t j = i + 1; j < cells.size(); ++j) {
            if (cellsOverlap(cells[i], cells[j])) {
                std::cerr << "Overlap detected between cells: " << cells[i]->name << " and " << cells[j]->name << std::endl;
            }
        }
    }
}