#ifndef WAYPOINT_CPP
#define WAYPOINT_CPP

#include <cmath>


/*
    Waypoint
    ---------
    Class to store the location of a specific point on the map thast we wish to visit
    has the options for 3 separate coordinate systems and comes with all the required conversion methods
*/
class Waypoint {

    // Attributes

protected:
    double x;           // Local east/west coordinate (east is in the +ve direction)
    double y;           // Local north/south coordinate (north is in the +ve direction)
    
    double longitude;   // Geodetic Longitude
    double latitude;    // Geodetic Latitude
    double altitude;    // Geodetic Altitude

    double ECEF_x;      // Earth Centered Earth Fixed X coordinate
    double ECEF_y;      // Earth Centered Earth Fixed Y coordinate
    double ECEF_z;      // Earth Centered Earth Fixed Z coordinate

    static constexpr double home_x = 0.0;          // local coordinates of the home base
    static constexpr double home_y = 0.0;          // local coordinates of the home base

    static double home_longitude;  // geodetic coordinates of the home base
    static double home_latitude;   // geodetic coordinates of the home base
    static double home_altitude;   // geodetic coordinates of the home base

    static double home_ECEF_x;      // Earth Centered Earth Fixed X coordinate of home base
    static double home_ECEF_y;      // Earth Centered Earth Fixed Y coordinate of home base
    static double home_ECEF_z;      // Earth Centered Earth Fixed Z coordinate of home base

    static bool homeIsSet;          // boolean to stop home position from being accidentally reset


    // Constants

    static constexpr double a = 6378137.0;               // Semi-major axis (meters)
    static constexpr double f = 1.0 / 298.257223565;     // Flattening
    static constexpr double e2 = f * (2.0 - f);          // Eccentricity squared
    static constexpr double pi = 3.14159265358979323846; // pi
    static const int password = 741021;                  // Password for resetting home waypoint

    // Constructors

public:
    //Waypoint() : x(0.0), y(0.0) {}

    // constructor for when using local coordinates
    Waypoint(double newX, double newY) : x(newX), y(newY) {
        local_To_EarthCentred();
        EarthCentred_To_GeodeticApprox();
    }

    // constructor for when using geodetic or Earth Centred coordinates
    Waypoint(double coordinateA, double coordinateB, double coordinateC, bool geodeticCoordinate) {
        if (geodeticCoordinate){
            longitude = coordinateA;
            latitude = coordinateB;
            altitude = coordinateC;

            geodetic_To_EarthCentred();
            EarthCentred_To_Local();
        }

        else {
            ECEF_x = coordinateA;
            ECEF_y = coordinateB;
            ECEF_z = coordinateC;

            EarthCentred_To_Local();
            EarthCentred_To_GeodeticApprox();
        }
    }

    // Getters and Setters
public:
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

    void setLongitude(double newLongitude) { longitude = newLongitude; }
    void setLatitude(double newLatitude) { latitude = newLatitude; }
    void setAltitude(double newAltitude) { altitude = newAltitude; }

    void setECEF_x(double newECEF_x) { ECEF_x = newECEF_x;}
    void setECEF_y(double newECEF_y) { ECEF_y = newECEF_y;}
    void setECEF_z(double newECEF_z) { ECEF_z = newECEF_z;}

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
        if (password == this->password || !homeIsSet) {
            setHome(point);
            return true;
        }
        return false;
    }

    static bool checkPassword(int password){
        if(Waypoint::password == password){
            return true;
        }
        return false;
    }

private: 
    void setHome(Waypoint* point){
        home_longitude = point->longitude;
        home_latitude  = point->latitude;
        home_altitude  = point->altitude;

        home_ECEF_x = point->ECEF_x;
        home_ECEF_y = point->ECEF_y;
        home_ECEF_z = point->ECEF_z;
        
        homeIsSet = true;
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
    void geodetic_To_EarthCentred() {
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
    void EarthCentred_To_Local() {
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
    void local_To_EarthCentred() {
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
    void EarthCentred_To_GeodeticApprox() {
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

// dummy main function for standalone testing
// int main(int argc, char** argv) {
//     (void)argc;
//     (void)argv;
// 
//     return 0;
// }

// define static member variables of waypoint class
double Waypoint::home_longitude = 0.0;
double Waypoint::home_latitude = 0.0;
double Waypoint::home_altitude = 0.0;
double Waypoint::home_ECEF_x = 0.0;
double Waypoint::home_ECEF_y = 0.0;
double Waypoint::home_ECEF_z = 0.0;

#endif