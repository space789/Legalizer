///////////////////////////
// File: Site.h          //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#ifndef SITE_H
#define SITE_H

#include <memory>

class Cell;

class Site {
public:
    Site(double x, double y);

    double x;
    double y;
    bool isOccupied;
    std::shared_ptr<Cell> cell;
};

#endif // SITE_H
