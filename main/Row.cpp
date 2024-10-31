///////////////////////////
// File: Row.cpp         //
// Author: Shiina        //
// Date: 2024/10/31      //
// Version: 1.0          //
// copiright 2024        //
///////////////////////////

#include "Row.h"
#include "Site.h"

Row::Row(double originX, double originY, double siteWidth, int siteCount)
    : originX(originX), originY(originY), height(0), siteWidth(siteWidth), siteCount(siteCount) {
    // Initialize sites
    for (int i = 0; i < siteCount; ++i) {
        sites.push_back(std::make_shared<Site>(originX + i * siteWidth, originY));
    }
}
