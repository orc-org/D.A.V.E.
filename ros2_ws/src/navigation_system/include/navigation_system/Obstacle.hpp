#pragma once

#include "Waypoint.hpp"
    
/*
    Obstacle
    ---------
    represents areas that the rover is to avoid which will be approximated as circular areas to simplify calculations
    it's just a way point with a "keep out" radius
    for example, if we knew there was a big hole at (73,89) on the map, we could use an obstacle to ensure our planned route doesn't take us within 5m of it
*/
class Obstacle : Waypoint{

    // Attributes
public:
    static const int defaultRadius = 5;
    static int largestObstacle;

private:
    int radius; // specifies how far we must stay away from the obstacle's location    

    // Getters and Setters

public:
    void setRadius(int radius);
    int getRadius();
   
    int getLargestObstacle();

    double getX();
    double getY();

    double getLatitude();
    double getLongitude();
    double getAltitude();

    double getECEF_x();
    double getECEF_y();
    double getECEF_z();

    // constructors
    Obstacle();

    Obstacle(double newX, double newY, int radius);

    Obstacle(double coordinateA, double coordinateB, double coordinateC, int radius, bool geodeticCoordinate);

    Obstacle(double newX, double newY);

    Obstacle(double coordinateA, double coordinateB, double coordinateC, bool geodeticCoordinate);

    bool operator<(const Obstacle& other) const;
};
