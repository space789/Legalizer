///////////////////////////
// File: Row.h           //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#ifndef ROW_H
#define ROW_H

#include <vector>
#include <memory>

class Site;

class Row {
public:
    Row(double originX, double originY, double siteWidth, int siteCount);

    double originX;
    double originY;
    double height;
    double siteWidth;
    int siteCount;

    std::vector<std::shared_ptr<Site>> sites;
};

#endif // ROW_H
