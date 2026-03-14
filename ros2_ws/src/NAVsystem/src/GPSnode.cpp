// used for functions
#include <cstdio>
#include <string>
#include <array>
#include <cmath>
#include <sstream>
#include <iostream>

// used for ros publishers and servers
#include <chrono>
#include <functional>
#include <memory>

//#include "rclcpp/rclcpp.hpp"
//#include "std_msgs/msg/string.hpp"

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
    - x = East
    - y = North
*/


/*
    ECEFPoint
    ---------
    Stores Earth-Centered, Earth-Fixed Cartesian coordinates.
*/

struct Waypoint {
private:
    double x;
    double y;

public:
    Waypoint() : x(0.0), y(0.0) {}

    Waypoint(double newX, double newY) : x(newX), y(newY) {}

    void set(Waypoint point) {
        x = point.getX();
        y = point.getY();
    }

    void setX(double newX) {
        x = newX;
    }

    void setY(double newY) {
        y = newY;
    }

    double getX() const {
        return x;
    }

    double getY() const {
        return y;
    }
};

struct ECEFPoint {
    double x;
    double y;
    double z;
};

//class GPSnode : public rclcpp::Node {

//////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
//////////////////////////////////////////////////////////////////////////

string translate(Waypoint point);
Waypoint translate(string point);
string translate(Waypoint point, bool localCoordinates);
Waypoint translate(string point, bool localCoordinates);


//////////////////////////////////////////////////////////////////////////
// GLOBAL NAVIGATION STATE
//////////////////////////////////////////////////////////////////////////

Waypoint g_home(0.0, 0.0);                 // Home waypoint in local coordinates
Waypoint g_target(0.0, 0.0);               // Destination waypoint in local coordinates
Waypoint g_currentPosition(0.0, 0.0);      // Current rover position in local coordinates

bool g_goingHome = false;                  // Whether rover should navigate home
bool g_useLocalCoordinates = false;        // False = GPS strings, True = local x,y strings
bool g_homeIsSet = false;                  // Prevent accidental reset of home

const int g_password = 741021;             // Password override for resetting home

//////////////////////////////////////////////////////////////////////////
// EARTH / GEODESY CONSTANTS
//////////////////////////////////////////////////////////////////////////

/*
    
*/
const double g_a = 6378137.0;                  // Semi-major axis (meters)
const double g_f = 1.0 / 298.257223565;        // Flattening
const double g_e2 = g_f * (2.0 - g_f);         // Eccentricity squared
const double g_pi = 3.14159265358979323846;

//////////////////////////////////////////////////////////////////////////
// HOME GPS / ORIGIN STATE
//////////////////////////////////////////////////////////////////////////

/*
    These define the local tangent plane origin.

    homeLatRad  = home latitude in radians
    homeLonRad  = home longitude in radians

    homeECEF    = home point converted into ECEF coordinates
*/
double g_homeLatRad = 0.0;
double g_homeLonRad = 0.0;
double g_homeAlt = 0.0;   // for now assume zero if GNSS altitude is unavailable

ECEFPoint g_homeECEF{0.0, 0.0, 0.0};

//////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS: ANGLE CONVERSION
//////////////////////////////////////////////////////////////////////////

double degToRad(double degrees) {
    return degrees * g_pi / 180.0;
}

double radToDeg(double radians) {
    return radians * 180.0 / g_pi;
}

//////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS: STRING PARSING / FORMATTING
//////////////////////////////////////////////////////////////////////////

/*
    parseCoordinateString()
    -----------------------
    Parses strings like:
    "49.8880,-119.4960"
    or
    "12.5,7.2"

    Returns true if parsing succeeds.
*/
bool parseCoordinateString(const string& input, double& first, double& second) {
    stringstream ss(input);
    char comma;

    if (!(ss >> first)) return false;
    if (!(ss >> comma) || comma != ',') return false;
    if (!(ss >> second)) return false;

    return true;
}

/*
    formatTwoDoubles()
    ------------------
    Formats two doubles as "a,b"
*/
string formatTwoDoubles(double a, double b) {
    return to_string(a) + "," + to_string(b);
}

//////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS: ECEF / GEODETIC MATH
//////////////////////////////////////////////////////////////////////////

/*
    calcPrimeVerticalRadius()
    -------------------------
    Computes N(phi), the prime vertical radius of curvature.
*/
double calcPrimeVerticalRadius(double latRad) {
    return g_a / sqrt(1.0 - g_e2 * sin(latRad) * sin(latRad));
}

/*
    geodeticToECEF()
    ----------------
    Converts latitude, longitude, altitude to ECEF coordinates.

    Inputs:
    - latRad = latitude in radians
    - lonRad = longitude in radians
    - h      = altitude in meters

    Outputs:
    - ECEFPoint {X, Y, Z}
*/
ECEFPoint geodeticToECEF(double latRad, double lonRad, double h) {
    double N = calcPrimeVerticalRadius(latRad);

    ECEFPoint point;
    point.x = (N + h) * cos(latRad) * cos(lonRad);
    point.y = (N + h) * cos(latRad) * sin(lonRad);
    point.z = (N * (1.0 - g_e2) + h) * sin(latRad);

    return point;
}

/*
    ecefToLocalENU2D()
    ------------------
    Converts an ECEF point into local ENU coordinates relative to home.

    Since rover is 2D, we only return:
    - x = East
    - y = North
*/
Waypoint ecefToLocalENU2D(const ECEFPoint& currentECEF) {
    double dX = currentECEF.x - g_homeECEF.x;
    double dY = currentECEF.y - g_homeECEF.y;
    double dZ = currentECEF.z - g_homeECEF.z;

    double east =
        -sin(g_homeLonRad) * dX
        + cos(g_homeLonRad) * dY;

    double north =
        -sin(g_homeLatRad) * cos(g_homeLonRad) * dX
        -sin(g_homeLatRad) * sin(g_homeLonRad) * dY
        +cos(g_homeLatRad) * dZ;

    return Waypoint(east, north);
}

/*
    localENU2DToECEF()
    ------------------
    Converts a local 2D waypoint (East, North) back into an ECEF point.

    We assume Up = 0 because rover navigation is 2D.
*/
ECEFPoint localENU2DToECEF(const Waypoint& point) {
    double E = point.getX();
    double N = point.getY();

    double dX =
        -sin(g_homeLonRad) * E
        -sin(g_homeLatRad) * cos(g_homeLonRad) * N;

    double dY =
         cos(g_homeLonRad) * E
        -sin(g_homeLatRad) * sin(g_homeLonRad) * N;

    double dZ =
         cos(g_homeLatRad) * N;

    ECEFPoint result;
    result.x = g_homeECEF.x + dX;
    result.y = g_homeECEF.y + dY;
    result.z = g_homeECEF.z + dZ;

    return result;
}

/*
    ecefToGeodeticApprox()
    ----------------------
    Converts ECEF back to approximate geodetic coordinates.

    This uses an iterative latitude update.
    Good enough for this project skeleton.
*/
void ecefToGeodeticApprox(const ECEFPoint& point, double& latRad, double& lonRad, double& h) {
    lonRad = atan2(point.y, point.x);

    double p = sqrt(point.x * point.x + point.y * point.y);
    latRad = atan2(point.z, p * (1.0 - g_e2));

    for (int i = 0; i < 5; i++) {
        double N = calcPrimeVerticalRadius(latRad);
        h = p / cos(latRad) - N;
        latRad = atan2(point.z, p * (1.0 - g_e2 * N / (N + h)));
    }
}

//////////////////////////////////////////////////////////////////////////
// HOME / ORIGIN SETUP
//////////////////////////////////////////////////////////////////////////

/*
    initializeHomeFromGPS()
    -----------------------
    Sets the global home origin using GPS coordinates in DEGREES.

    This method:
    1. Converts degrees -> radians
    2. Stores home latitude / longitude
    3. Computes home ECEF
    4. Sets local home waypoint to (0,0)
*/
bool initializeHomeFromGPS(double latDeg, double lonDeg, double altMeters = 0.0) {
    g_homeLatRad = degToRad(latDeg);
    g_homeLonRad = degToRad(lonDeg);
    g_homeAlt = altMeters;

    g_homeECEF = geodeticToECEF(g_homeLatRad, g_homeLonRad, g_homeAlt);

    g_home = Waypoint(0.0, 0.0);
    g_homeIsSet = true;

    return true;
}

//////////////////////////////////////////////////////////////////////////
// GETTERS
//////////////////////////////////////////////////////////////////////////

string getTarget() {
    return translate(g_target);
}

string getHome() {
    return translate(g_home);
}

//////////////////////////////////////////////////////////////////////////
// SETTERS
//////////////////////////////////////////////////////////////////////////

void setTarget(string coordinates) {
    // Sets target from either local string or GPS string depending on preference
    g_target = translate(coordinates);
}

void setHome(string coordinates) {
    /*
        Intended meaning:
        - input is GPS string "lat,lon"
        - set home origin using that GPS point
        - set local home waypoint to (0,0)
    */
    if (!g_homeIsSet) {
        double latDeg, lonDeg;
        if (parseCoordinateString(coordinates, latDeg, lonDeg)) {
            initializeHomeFromGPS(latDeg, lonDeg, 0.0);
        }
    }
}

void setHome() {
    /*
        Sets home to current LOCAL position.
        This only changes local waypoint, not GPS origin.
        If you want to redefine the GPS origin too, you need actual current GNSS lat/lon.
    */
    if (!g_homeIsSet) {
        g_home.set(g_currentPosition);
        g_homeIsSet = true;
    }
}

void setHome(int password) {
    if (password == g_password) {
        g_home.set(g_currentPosition);
        g_homeIsSet = true;
    }
}

void setHome(string coordinates, int password) {
    if (password == g_password) {
        double latDeg, lonDeg;
        if (parseCoordinateString(coordinates, latDeg, lonDeg)) {
            initializeHomeFromGPS(latDeg, lonDeg, 0.0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// NAVIGATION / BEARING
//////////////////////////////////////////////////////////////////////////

/*
    getBearing()
    ------------
    Returns a 2D direction vector from current position toward the active goal.

    If g_goingHome == true:
        goal = g_home
    else:
        goal = g_target

    Returns:
    [dx, dy]
*/
std::array<double, 2> getBearing() {
    Waypoint destination = g_goingHome ? g_home : g_target;

    double dx = destination.getX() - g_currentPosition.getX();
    double dy = destination.getY() - g_currentPosition.getY();

    return {dx, dy};
}

void goHome() {
    g_goingHome = true;
}

//////////////////////////////////////////////////////////////////////////
// TRANSLATION HELPERS
//////////////////////////////////////////////////////////////////////////

string translate(Waypoint point) {
    return translate(point, g_useLocalCoordinates);
}

Waypoint translate(string point) {
    return translate(point, g_useLocalCoordinates);
}

/*
    translate(Waypoint, bool)
    -------------------------
    If localCoordinates == true:
        return "x,y"

    If localCoordinates == false:
        convert local waypoint -> GPS string "lat,lon"
*/
string translate(Waypoint point, bool localCoordinates) {
    if (localCoordinates) {
        return formatTwoDoubles(point.getX(), point.getY());
    } else {
        if (!g_homeIsSet) {
            return "HOME_NOT_SET";
        }

        ECEFPoint ecefPoint = localENU2DToECEF(point);

        double latRad = 0.0;
        double lonRad = 0.0;
        double h = 0.0;

        ecefToGeodeticApprox(ecefPoint, latRad, lonRad, h);

        double latDeg = radToDeg(latRad);
        double lonDeg = radToDeg(lonRad);

        return formatTwoDoubles(latDeg, lonDeg);
    }
}

/*
    translate(string, bool)
    -----------------------
    If localCoordinates == true:
        parse string as "x,y" local meters

    If localCoordinates == false:
        parse string as "lat,lon" in degrees
        then convert GPS -> local waypoint
*/
Waypoint translate(string point, bool localCoordinates) {
    double first, second;
    if (!parseCoordinateString(point, first, second)) {
        return Waypoint(0.0, 0.0);
    }

    if (localCoordinates) {
        return Waypoint(first, second);
    } else {
        if (!g_homeIsSet) {
            // If home isn't set, we cannot convert GPS to local safely.
            // For now return origin.
            return Waypoint(0.0, 0.0);
        }

        double latRad = degToRad(first);
        double lonRad = degToRad(second);
        double alt = 0.0;

        ECEFPoint currentECEF = geodeticToECEF(latRad, lonRad, alt);
        return ecefToLocalENU2D(currentECEF);
    }
}

//////////////////////////////////////////////////////////////////////////
// GNSS INPUT PLACEHOLDER
//////////////////////////////////////////////////////////////////////////

/*
    These variables simulate future GNSS module output.

    WHEN WE GET THE MODULE, replace values parsed from:
    - serial input
    - ROS topic
    - GNSS driver message
*/
bool g_gnssHasFix = false;
double g_gnssLatDeg = 0.0;
double g_gnssLonDeg = 0.0;
double g_gnssAltMeters = 0.0;

//////////////////////////////////////////////////////////////////////////
// SERIAL / GNSS PROCESSING
//////////////////////////////////////////////////////////////////////////

/*
    serialDecode()
    --------------
    Placeholder GNSS pipeline.

    What this should do:
    1. Read current GNSS lat/lon/alt from module
    2. If home not set, initialize home using first valid fix
    3. Convert current GNSS fix -> ECEF
    4. Convert ECEF -> local ENU
    5. Update g_currentPosition
*/
void serialDecode() {
    if (!g_gnssHasFix) {
        return;
    }

    if (!g_homeIsSet) {
        initializeHomeFromGPS(g_gnssLatDeg, g_gnssLonDeg, g_gnssAltMeters);
    }

    double latRad = degToRad(g_gnssLatDeg);
    double lonRad = degToRad(g_gnssLonDeg);

    ECEFPoint currentECEF = geodeticToECEF(latRad, lonRad, g_gnssAltMeters);
    g_currentPosition = ecefToLocalENU2D(currentECEF);
}

void serialSend(string message) {
    // Placeholder for sending configuration commands to GNSS module
    cout << "Sending to GNSS module: " << message << endl;
}

void togglePreference() {
    g_useLocalCoordinates = !g_useLocalCoordinates;
}

//////////////////////////////////////////////////////////////////////////
// Define Ros Related stuff
//////////////////////////////////////////////////////////////////////////

//class GPSnode : public rclcpp::Node {
  //  public:

//};




//}; // end of class GPSnode

//////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("GPS processor node prototype running.\n");

    /*
        Example simulation:
        1. Simulate GNSS first fix
        2. serialDecode() sets home and current position
        3. Set target using GPS coordinates
        4. Print target in current preference format
    

    g_gnssHasFix = true;
    g_gnssLatDeg = 49.8880;
    g_gnssLonDeg = -119.4960;
    g_gnssAltMeters = 0.0;

    serialDecode();

    setTarget("49.8885,-119.4955");

    cout << "Current position: " << translate(g_currentPosition, true) << endl;
    cout << "Target: " << getTarget() << endl;
    */

    return 0;
}
