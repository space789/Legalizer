///////////////////////////
// File: Cell.h          //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#ifndef CELL_H
#define CELL_H

#include <string>
#include <memory>

class Cell {
public:
    Cell(const std::string& name, double width, double height);

    std::string name;
    double width;
    double height;
    double x; // Left-bottom x-coordinate
    double y; // Left-bottom y-coordinate
    int density;
    bool isFixed;
    std::string orientation;

    // Original global placement coordinates
    double originalX;
    double originalY;
};

#endif // CELL_H
