#include "Obstacle.hpp"

int Obstacle::largestObstacle = 5;

void Obstacle::setRadius(int radius){
    this->radius = radius;
}
int Obstacle::getRadius(){
    return radius;
}
   
int Obstacle::getLargestObstacle(){
    return largestObstacle;
}

double Obstacle::getX() { return x; }
double Obstacle::getY() { return y; }

double Obstacle::getLatitude() { return latitude; }
double Obstacle::getLongitude(){ return longitude; }
double Obstacle::getAltitude() { return altitude; }

double Obstacle::getECEF_x(){ return ECEF_x; }
double Obstacle::getECEF_y(){ return ECEF_y; }
double Obstacle::getECEF_z(){ return ECEF_z; }

    // constructors
Obstacle::Obstacle() : Waypoint(0.0, 0.0), radius(defaultRadius) {
    if(largestObstacle < radius){
        largestObstacle = radius;
    }
}

Obstacle::Obstacle(double newX, double newY, int radius): Waypoint(newX, newY){
    this->radius = radius;
    if(largestObstacle < radius){
        largestObstacle = radius;
    }
}

Obstacle::Obstacle(double coordinateA, double coordinateB, double coordinateC, int radius, bool geodeticCoordinate): Waypoint(coordinateA, coordinateB, coordinateC, geodeticCoordinate){
    this->radius = radius;
    if(largestObstacle < radius){
        largestObstacle = radius;
    }
}

Obstacle::Obstacle(double newX, double newY): Waypoint(newX, newY){
    this->radius = defaultRadius;
    if(largestObstacle < radius){
        largestObstacle = radius;
    }
}

Obstacle::Obstacle(double coordinateA, double coordinateB, double coordinateC, bool geodeticCoordinate): Waypoint(coordinateA, coordinateB, coordinateC, geodeticCoordinate){
    this->radius = defaultRadius;
    if(largestObstacle < radius){
        largestObstacle = radius;
    }
}

bool Obstacle::operator<(const Obstacle& other) const {
    return this->x < other.x;
}

