<<<<<<< HEAD
#include "Obstacle.hpp"
=======
#ifndef OBSTACLE_CPP
#define OBSTACLE_CPP

#include "Waypoint.cpp"
>>>>>>> fa8e302 (commit everything)

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

<<<<<<< HEAD
=======
};

// dummy main function for standalone testing
// int main(int argc, char** argv) {
//     (void)argc;
//     (void)argv;
// 
//     return 0;
// }

// define static member variables of obstacle class
int Obstacle::largestObstacle = 0;

#endif
>>>>>>> fa8e302 (commit everything)
