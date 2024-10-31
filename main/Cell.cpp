///////////////////////////
// File: Cell.cpp        //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#include "Cell.h"

Cell::Cell(const std::string& name, double width, double height)
    : name(name), width(width), height(height), x(0), y(0), density(0), isFixed(false), orientation("N"), originalX(0), originalY(0) {}
