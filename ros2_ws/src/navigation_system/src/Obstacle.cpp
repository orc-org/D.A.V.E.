#ifndef OBSTACLE_CPP
#define OBSTACLE_CPP

#include "Waypoint.cpp"


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
    void setRadius(int radius){
        this->radius = radius;
    }
    int getRadius(){
        return radius;
    }
    double getX() { return x; }
    double getY() { return y; }

    int getLargestObstacle(){
        return largestObstacle;
    }

    // constructors
    Obstacle() : Waypoint(0.0, 0.0), radius(defaultRadius) {
        if(largestObstacle < radius){
            largestObstacle = radius;
        }
    }


    Obstacle(double newX, double newY, int radius): Waypoint(newX, newY){
        this->radius = radius;
        if(largestObstacle < radius){
            largestObstacle = radius;
        }
    }

    Obstacle(double coordinateA, double coordinateB, double coordinateC, int radius, bool geodeticCoordinate): Waypoint(coordinateA, coordinateB, coordinateC, geodeticCoordinate){
        this->radius = radius;
        if(largestObstacle < radius){
            largestObstacle = radius;
        }
    }

    Obstacle(double newX, double newY): Waypoint(newX, newY){
        this->radius = defaultRadius;
        if(largestObstacle < radius){
            largestObstacle = radius;
        }
    }

    Obstacle(double coordinateA, double coordinateB, double coordinateC, bool geodeticCoordinate): Waypoint(coordinateA, coordinateB, coordinateC, geodeticCoordinate){
        this->radius = defaultRadius;
        if(largestObstacle < radius){
            largestObstacle = radius;
        }
    }

    bool operator<(const Obstacle& other) const {
        return this->x < other.x;
    }

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