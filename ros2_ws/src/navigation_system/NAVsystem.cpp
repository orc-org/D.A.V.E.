// used for functions
#include <cstdio>
#include <string>
#include <array>
#include <list>
#include <vector>
#include <cmath>
#include <sstream>
#include <iostream>
//#include <rclcpp>

// used for ros publishers and servers
#include <chrono>
#include <functional>
#include <memory>
#include <iomanip>

//custom defined interface for use with topics and services
//#include "navigation_interfaces/msg/bearing_string.hpp"
//#include "navigation_interfaces/msg/position_string.hpp"
//#include "navigation_interfaces/srv/get_nav.hpp"
//#include "navigation_interfaces/srv/set_nav.hpp"

//interfaces needed to work with ros
//#include "rclcpp/rclcpp.hpp"


using namespace std;
using namespace std::chrono_literals;

//////////////////////////////////////////////////////////////////////////
// BASIC DATA STRUCTURES
//////////////////////////////////////////////////////////////////////////

/*
    Waypoint
    --------
    Represents a LOCAL 2D waypoint in meters relative to home.
    
    Convention:
    - +ve x = East
    - +ve y = North
*/
class Waypoint {

    // Attributes

private:
    double x;           // Local east/west coordinate (east is in the +ve direction)
    double y;           // Local north/south coordinate (north is in the +ve direction)
    
    double longitude;   // Geodetic Longitude
    double latitude;    // Geodetic Latitude
    double altitude;    // Geodetic Altitude

    double ECEF_x;      // Earth Centered Earth Fixed X coordinate
    double ECEF_y;      // Earth Centered Earth Fixed Y coordinate
    double ECEF_z;      // Earth Centered Earth Fixed Z coordinate

    static double home_x;          // local coordinates of the home base
    static double home_y;          // local coordinates of the home base

    static double home_longitude;  // geodetic coordinates of the home base
    static double home_latitude;   // geodetic coordinates of the home base
    static double home_altitude;   // geodetic coordinates of the home base

    static double home_ECEF_x;      // Earth Centered Earth Fixed X coordinate of home base
    static double home_ECEF_y;      // Earth Centered Earth Fixed Y coordinate of home base
    static double home_ECEF_z;      // Earth Centered Earth Fixed Z coordinate of home base

    static bool homeIsSet;         // boolean to stop home position from being accidentally reset


    // Constants

    const double a = 6378137.0;               // Semi-major axis (meters)
    const double f = 1.0 / 298.257223565;     // Flattening
    const double e2 = f * (2.0 - f);          // Eccentricity squared
    const double pi = 3.14159265358979323846; // pi
    static const int password = 741021; // Password for resetting home waypoint

    // Constructors

public:
    Waypoint() : x(0.0), y(0.0) {}

    Waypoint(double newX, double newY) : x(newX), y(newY) {
        // TODO: rewrite to calculate other coordinate representations from the 2 provided
    }

    // Getters and Setters

    void set(Waypoint* point) {
        x = point->getX();
        y = point->getY();
        
        longitude = point->getLongitude();
        latitude = point->getLatitude();
        altitude = point->getAltitude();
        
        ECEF_x = point->getECEF_x();
        ECEF_y = point->getECEF_y();
        ECEF_z = point->getECEF_z();
    }

    void setX(double newX) { x = newX; } 
    void setY(double newY) { y = newY; }

    void setLongitude(double newLongitude) { y = newLongitude; }
    void setLatitude(double newLatitude) { y = newLatitude; }
    void setAltitude(double newAltitude) { y = newAltitude; }

    void setECEF_x(double newECEF_x) { ECEF_x = newECEF_x;}
    void setECEF_y(double newECEF_y) { ECEF_x = newECEF_y;}
    void setECEF_z(double newECEF_z) { ECEF_x = newECEF_z;}

    double getX() { return x; }
    double getY() { return y; }

    double getLongitude() { return longitude; }
    double getLatitude() { return latitude; }
    double getAltitude() { return altitude; }

    double getECEF_x() { return ECEF_x; }
    double getECEF_y() { return ECEF_y; }
    double getECEF_z() { return ECEF_z; }

    /*
        setHome(Waypoint point, int password)
        ------------------------------------
        method to reset home base position
        it checks the password provided password to prevent to prevent it being used accidentally (no need for correct password if home is not set)

        returns false if home was not reset
    */
    bool setHome(Waypoint* point, int password){
        if (password = this->password || !homeIsSet) {
            setHome(point);
            return true;
        }
        return false;
    }

private: 
    void setHome(Waypoint* point){
        home_x = point->x;
        home_y = point->y;

        home_longitude = point->longitude;
        home_latitude  = point->latitude;
        home_altitude  = point->altitude;

        home_ECEF_x = point->ECEF_x;
        home_ECEF_y = point->ECEF_y;
        home_ECEF_z = point->ECEF_z;
    }

    // Conversion Methods

public:
    double degToRad(double degrees) {
        return degrees * pi / 180.0;
    }

    double radToDeg(double radians) {
        return radians * 180.0 / pi;
    }

    double calcPrimeVerticalRadius(double latRad) {
        return a / sqrt(1.0 - e2 * sin(latRad) * sin(latRad));
    }

    /*
        geodetic_To_EarthCentered
        ----------------
        Converts latitude, longitude, altitude to ECEF coordinates.
    */
    void geodetic_To_EarthCentered() {
        double N = a / sqrt(1.0 - e2 * sin(latitude) * sin(latitude));

        ECEF_x = (N + altitude) * cos(latitude) * cos(longitude);
        ECEF_y = (N + altitude) * cos(latitude) * sin(longitude);
        ECEF_z = (N * (1.0 - e2) + altitude) * sin(latitude);
    }

    /*
        EarthCentered_To_Local
        ------------------
        Converts an ECEF point into local ENU coordinates relative to home.
    */
    void EarthCentered_To_Local() {
        double dX = ECEF_x - home_ECEF_x;
        double dY = ECEF_y - home_ECEF_y;
        double dZ = ECEF_z - home_ECEF_z;
        
        x = -sin(home_longitude) * dX 
            +cos(home_latitude) * dY;

        y = -sin(home_latitude) * cos(home_longitude) * dX 
            -sin(home_latitude) * sin(home_longitude) * dY 
            +cos(home_latitude) * dZ;
    }

    /*
        local_To_EarthCentered
        ------------------
        Converts a local 2D waypoint (East, North) back into an ECEF point.
    */
    void local_To_EarthCentered() {
        double dX =
            -sin(home_longitude) * x
            -sin(home_latitude) * cos(home_longitude) * y;

        double dY =
             cos(home_longitude) * x
            -sin(home_latitude) * sin(home_longitude) * y;

        double dZ =
            cos(home_latitude) * y;

        ECEF_x = home_ECEF_x + dX;
        ECEF_y = home_ECEF_y + dY;
        ECEF_z = home_ECEF_z + dZ;
    }
    
    /*
    EarthCentered_To_Geodetic()
    ----------------------
    Converts ECEF back to approximate geodetic coordinates.

    This uses an iterative latitude update.
    Good enough for this project skeleton.
    */
    void ecef_To_GeodeticApprox() {
        double lonRad = atan2(ECEF_y, ECEF_x);

        double p = sqrt(ECEF_x * ECEF_x + ECEF_y * ECEF_y);
        double latRad = atan2(ECEF_z, p * (1.0 - e2));
        double h;

        for (int i = 0; i < 5; i++) {
           double N = calcPrimeVerticalRadius(latRad);
          h = p / cos(latRad) - N;
           latRad = atan2(ECEF_z, p * (1.0 - e2 * N / (N + h)));
        }
    }
};

/*
    Obstacle
    ---------
    represents areas that the rover is to avoid which will be approximated as circular areas to simplify calculations
    it's just a way point with a "keep out" radius
    for example, if we knew there was a big hole at (73,89) on the map, we could use an obstacle to ensure our planned route doesn't take us within 5m of it
*/
class Obstacle : Waypoint{

    // Attributes

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

    // constructors

    //TODO write constructors for obstacle class

};

/*
    Velocity
    --------
    Stores the most recent speed and direction of travel of the rover
    This will be calculated via a Finite difference approximation using the GPS position data

*/
struct Velocity {
private: 
    double speed;     // stored in m/s
    double direction; // cardinal direction that the rover is facing
};

//////////////////////////////////////////////////////////////////////////
// Navigation Class
//////////////////////////////////////////////////////////////////////////

class NavigationSystem {

    // Attributes
private:
    bool debug;                 // displays added information to the terminal for debugging purposes (false by default)
    bool returnToBase;          // boolean to select the direction of travel along selected route 
    
    enum coordinates {local, geodetic, earthCentered};  // defines the three coordinate systems available to the pilot 
    enum coordinates preference; // used to select which form of the coordinates gets displayed to the pilot

    int routeSelected;           // to allow the operator to select to follow a specific preplanned route (if we make multiple)
    
    Waypoint currentPosition;    // Waypoint to store the current position of the rover (updated often)
    Velocity currentVelocity;    // Speed and direction of rover (calculated from positional data from GPS)

    list<Waypoint> pointsVisited;   // Doubly linked list to store the path that the rover has actually taken (whether it was planned or not)
    vector<Obstacle> obstacles;     // vector to store Obstacles in the way of the rover
    vector<list<Waypoint>> routes;  // Vector to allow us to store multiple preplanned routes
                                    // each route will be a doubly linked list that will hold several waypoints in the order that they should be visited 
    
    // Getters and Setters
public:
    void incrementRouteSelected(){ routeSelected++; }
    int getRouteSelected(){ return routeSelected; }

    void toggleDebug(){ debug = !debug; }
    void toggleReturnToBase(){ returnToBase = !returnToBase; }
    void toggleCoordinatePreference(){
        switch(preference){
            case(local): preference = geodetic;
            case(geodetic): preference = earthCentered;
            default: preference = local;
        }
    }





    // Other Methods

    // Constructor

    // Define Topics

    // Define Services

    // Define Actions
};

//////////////////////////////////////////////////////////////////////////
// TODO
//////////////////////////////////////////////////////////////////////////

/*
    - fleshout waypoint constructors to take different sets of arguments and to automatically make the conversions necessary to have 
    all 3 coordinate forms defined in each waypoint object

    - finish logic behind the Velocity class

    - finish logic behind the Navsystem class

    - define our ros related stuff

    - update documentation
*/

//////////////////////////////////////////////////////////////////////////
// END TODO
//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    cout<<"Hello World!!"<<endl;

    return 0;
}