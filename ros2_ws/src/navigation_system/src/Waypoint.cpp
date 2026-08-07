#ifndef WAYPOINT_CPP
#define WAYPOINT_CPP

#ifndef WAYPOINT_CPP
#define WAYPOINT_CPP

#include <cmath>
#include "Waypoint.hpp"

// // Definitions for static members
//  Waypoint::a = 6378137.0;                // Semi-major axis (meters)
//  Waypoint::f = 1.0 / 298.257223565;      // Flattening
//  Waypoint::e2 = f * (2.0 - f);           // Eccentricity squared
//  Waypoint::pi = 3.14159265358979323846;  // pi

//  Waypoint::home_x = 0.0;          // local coordinates of the home base
//  Waypoint::home_y = 0.0;          // local coordinates of the home base
#include "Waypoint.hpp"

// // Definitions for static members
//  Waypoint::a = 6378137.0;                // Semi-major axis (meters)
//  Waypoint::f = 1.0 / 298.257223565;      // Flattening
//  Waypoint::e2 = f * (2.0 - f);           // Eccentricity squared
//  Waypoint::pi = 3.14159265358979323846;  // pi

//  Waypoint::home_x = 0.0;          // local coordinates of the home base
//  Waypoint::home_y = 0.0;          // local coordinates of the home base

//  Waypoint::home_ECEF_x = 0.0;
//  Waypoint::home_ECEF_y = 0.0;
//  Waypoint::home_ECEF_z = 0.0;

//  Waypoint::home_latitude = 0.0;
//  Waypoint::home_longitude = 0.0;
//  Waypoint::home_altitude = 792.18; //altitude of Dumheller airport (acording to CFS from August 2023)

//  Waypoint::home_ECEF_x = 0.0;
//  Waypoint::home_ECEF_y = 0.0;
//  Waypoint::home_ECEF_z = 0.0;

//  Waypoint::home_latitude = 0.0;
//  Waypoint::home_longitude = 0.0;
//  Waypoint::home_altitude = 792.18; //altitude of Dumheller airport (acording to CFS from August 2023)


// constructor for when using local coordinates
Waypoint::Waypoint(double newX, double newY) : x(newX), y(newY) {
    local_To_EarthCentred();
    EarthCentred_To_GeodeticApprox();
}
// constructor for when using local coordinates
Waypoint::Waypoint(double newX, double newY) : x(newX), y(newY) {
    local_To_EarthCentred();
    EarthCentred_To_GeodeticApprox();
}

// constructor for when using geodetic or Earth Centred coordinates
Waypoint::Waypoint(double coordinateA, double coordinateB, double coordinateC, bool geodeticCoordinate) {
    if (geodeticCoordinate){
        longitude = coordinateA;
        latitude = coordinateB;
        altitude = coordinateC;
// constructor for when using geodetic or Earth Centred coordinates
Waypoint::Waypoint(double coordinateA, double coordinateB, double coordinateC, bool geodeticCoordinate) {
    if (geodeticCoordinate){
        longitude = coordinateA;
        latitude = coordinateB;
        altitude = coordinateC;

        geodetic_To_EarthCentred();
        EarthCentred_To_Local();
    }
        geodetic_To_EarthCentred();
        EarthCentred_To_Local();
    }

    else {
        ECEF_x = coordinateA;
        ECEF_y = coordinateB;
        ECEF_z = coordinateC;
    else {
        ECEF_x = coordinateA;
        ECEF_y = coordinateB;
        ECEF_z = coordinateC;

        EarthCentred_To_Local();
        EarthCentred_To_GeodeticApprox();
    }
}
        EarthCentred_To_Local();
        EarthCentred_To_GeodeticApprox();
    }
}

// Conversion Methods
// Conversion Methods

void Waypoint::geodetic_To_EarthCentred() {
    double N = a / sqrt(1.0 - e2 * sin(latitude) * sin(latitude));
void Waypoint::geodetic_To_EarthCentred() {
    double N = a / sqrt(1.0 - e2 * sin(latitude) * sin(latitude));

    ECEF_x = (N + altitude) * cos(latitude) * cos(longitude);
    ECEF_y = (N + altitude) * cos(latitude) * sin(longitude);
    ECEF_z = (N * (1.0 - e2) + altitude) * sin(latitude);
}
    ECEF_x = (N + altitude) * cos(latitude) * cos(longitude);
    ECEF_y = (N + altitude) * cos(latitude) * sin(longitude);
    ECEF_z = (N * (1.0 - e2) + altitude) * sin(latitude);
}

void Waypoint::EarthCentred_To_Local() {
    double dX = ECEF_x - home_ECEF_x;
    double dY = ECEF_y - home_ECEF_y;
    double dZ = ECEF_z - home_ECEF_z;
    
    x = -sin(home_longitude) * dX 
        +cos(home_latitude) * dY;
void Waypoint::EarthCentred_To_Local() {
    double dX = ECEF_x - home_ECEF_x;
    double dY = ECEF_y - home_ECEF_y;
    double dZ = ECEF_z - home_ECEF_z;
    
    x = -sin(home_longitude) * dX 
        +cos(home_latitude) * dY;

    y = -sin(home_latitude) * cos(home_longitude) * dX 
        -sin(home_latitude) * sin(home_longitude) * dY 
        +cos(home_latitude) * dZ;
}
    y = -sin(home_latitude) * cos(home_longitude) * dX 
        -sin(home_latitude) * sin(home_longitude) * dY 
        +cos(home_latitude) * dZ;
}

void Waypoint::local_To_EarthCentred() {
    double dX =
        -sin(home_longitude) * x
        -sin(home_latitude) * cos(home_longitude) * y;
void Waypoint::local_To_EarthCentred() {
    double dX =
        -sin(home_longitude) * x
        -sin(home_latitude) * cos(home_longitude) * y;

    double dY =
            cos(home_longitude) * x
        -sin(home_latitude) * sin(home_longitude) * y;
    double dY =
            cos(home_longitude) * x
        -sin(home_latitude) * sin(home_longitude) * y;

    double dZ =
        cos(home_latitude) * y;
    double dZ =
        cos(home_latitude) * y;

    ECEF_x = home_ECEF_x + dX;
    ECEF_y = home_ECEF_y + dY;
    ECEF_z = home_ECEF_z + dZ;
}

void Waypoint::EarthCentred_To_GeodeticApprox() {
    double p = sqrt(ECEF_x * ECEF_x + ECEF_y * ECEF_y);
    if (p < 1e-9 && std::abs(ECEF_z) < 1e-9) {
        latitude = 0.0;
        longitude = 0.0;
        return;
    }

    double lonRad = atan2(ECEF_y, ECEF_x);
    double latRad = atan2(ECEF_z, p * (1.0 - e2));
    double h = 0.0;

    for (int i = 0; i < 5; i++) {
        double N = calcPrimeVerticalRadius(latRad);
        double denom = N + h;
        if (std::abs(denom) < 1e-9) break;
        h = p / std::cos(latRad) - N;
        latRad = atan2(ECEF_z, p * (1.0 - e2 * N / denom));
    }

    latitude = latRad;
    longitude = lonRad;
}

#endif // WAYPOINT_CPP
