// used for functions
#include <cstdio>
#include <string>
#include <array>
//#include <list>
//#include <vector>
#include <cmath>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <rclcpp>

// used for ros publishers and servers
#include <chrono>
#include <functional>
#include <memory>
#include <iomanip>

//custom defined interface for use with topics and services
#include "navigation_interfaces/msg/bearing_string.hpp"
#include "navigation_interfaces/msg/position_string.hpp"
#include "navigation_interfaces/srv/get_nav.hpp"
#include "navigation_interfaces/srv/set_nav.hpp"

//interfaces needed to work with ros
#include "rclcpp/rclcpp.hpp"


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
        if (password = this->password || !homeIsSet) {
            setHome(point);
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

/*
    ListNode
    --------
    the core part of my doubly linked list implementation
*/
class ListNode{
    public:
        Waypoint* point;
        ListNode* next;
        ListNode* previous;

        ListNode(ListNode* next, ListNode* previous,Waypoint* newPoint) : point(newPoint), next(next), previous(previous) {}
        
        ~ListNode() { delete point; }
};


/*
    ListOfWaypoints
    ---------------
    a simple pointer based implementation of a doubly linked list
    I couldn't get the standard list and iterator to play nicely, so this is the work around
*/
class ListOfWaypoints{
    private:
        ListNode* head;
        ListNode* tail;

    public:
        ListOfWaypoints() : head(nullptr), tail(nullptr) {}
        
        ~ListOfWaypoints() { clear(); }

        ListNode* getHead(){
            return head;
        }

        ListNode* getTail(){
            return tail;
        }

        ListNode* getPoint(int position){
            if (position < 0) return nullptr;

            ListNode* current = head;
            int index = 0;

            while (current != nullptr && index < position) {
                current = current->next;
                index++;
            }

            return current;
        }

        ListNode* getPoint(Waypoint* point){
            ListNode* current = head;

            while (current != nullptr) {
                if (areEqual(current->point, point)) {
                    return current;
                }
                current = current->next;
            }
            return nullptr;
        }

        ListNode* getPoint(double x, double y){
            ListNode* current = head;

            while (current != nullptr) {
                if (current->point->getX() == x && current->point->getY() == y) {
                    return current;
                }
                current = current->next;
            }
            return nullptr;
        }

        ListNode* getGeodeticPoint(double longitude, double latitude){
            ListNode* current = head;

            while (current != nullptr) {
                if (current->point->getLongitude() == longitude && current->point->getLatitude() == latitude) {
                    return current;
                }
                current = current->next;
            }
            return nullptr;
        }

        ListNode* getEarthCentredPoint(double x, double y, double z){
            ListNode* current = head;
    
            while (current != nullptr) {
                if (current->point->getECEF_x() == x && current->point->getECEF_y() == y && current->point->getECEF_z() == z) {
                    return current;
                }
                current = current->next;
            }
            return nullptr;
        }

        void add(Waypoint* newPoint){
            ListNode* newNode = new ListNode(nullptr, tail, newPoint);
            if (tail != nullptr) {
                tail->next = newNode;
            }
            tail = newNode;
            if (head == nullptr) {
                head = newNode;
            }
        }

        void addFirst(Waypoint* newPoint){
            ListNode* newNode = new ListNode(head, nullptr, newPoint);
            if (head != nullptr) {
                head->previous = newNode;
            }
            head = newNode;
            if (tail == nullptr) {
                tail = newNode;
            }
        }

        void add(Waypoint* newPoint, int position){
            if (position <= 0) {
                addFirst(newPoint);
                return;
            }

            ListNode* current = head;
            int index = 0;

            while (current != nullptr && index < position) {
                current = current->next;
                index++;
            }

            if (current == nullptr) {
                add(newPoint);
            } else {
                ListNode* newNode = new ListNode(current, current->previous, newPoint);
                if (current->previous != nullptr) {
                    current->previous->next = newNode;
                }
                current->previous = newNode;
                if (current == head) {
                    head = newNode;
                }
            }
        }

        void addBefore(ListNode* newNode, ListNode* existingNode){
            
            if (existingNode == head) {
                head = newNode;
                newNode->previous = nullptr;
            }
            else{
                existingNode->previous->next = newNode;
                newNode->previous = existingNode->previous;
            }
            existingNode->previous = newNode;
            newNode->next = existingNode;
        }

        void printList() {
            ListNode* current = head;
            while (current != nullptr) {
                cout << "(" << current->point->getX() << ", " << current->point->getY() << ") -> ";
                current = current->next;
            }
            cout << "End of Route" << endl;
        }

        void clear() {
            ListNode* current = head;
            while (current != nullptr) {
                ListNode* next = current->next;
                delete current;
                current = next;
            }
            head = nullptr;
            tail = nullptr;
        }

        void removeFirst() {
            if (head == nullptr) return;

            ListNode* temp = head;
            head = head->next;
            if (head != nullptr) {
                head->previous = nullptr;
            } else {
                tail = nullptr;
            }
            delete temp;
        }

        void removeLast() {
            if (tail == nullptr) return;

            ListNode* temp = tail;
            tail = tail->previous;
            if (tail != nullptr) {
                tail->next = nullptr;
            } else {
                head = nullptr;
            }
            delete temp;
        }

        void removeAt(int position) {
            if (position < 0) return;

            ListNode* current = head;
            int index = 0;

            while (current != nullptr && index < position) {
                current = current->next;
                index++;
            }

            if (current == nullptr) return;

            if (current->previous != nullptr) {
                current->previous->next = current->next;
            } else {
                head = current->next;
            }

            if (current->next != nullptr) {
                current->next->previous = current->previous;
            } else {
                tail = current->previous;
            }

            delete current;
        }

        void remove(Waypoint* point) {
            ListNode* current = head;

            while (current != nullptr) {
                if (areEqual(current->point, point)) {
                    if (current->previous != nullptr) {
                        current->previous->next = current->next;
                    } else {
                        head = current->next;
                    }

                    if (current->next != nullptr) {
                        current->next->previous = current->previous;
                    } else {
                        tail = current->previous;
                    }

                    delete current;
                    return;
                }
                current = current->next;
            }
        }

        void remove(ListNode* node) {
            if (node == nullptr) return;

            if (node->previous != nullptr) {
                node->previous->next = node->next;
            } else {
                head = node->next;
            }

            if (node->next != nullptr) {
                node->next->previous = node->previous;
            } else {
                tail = node->previous;
            }

            delete node;
        }

        void removePoint(double x, double y) {
            ListNode* current = head;

            while (current != nullptr) {
                if (current->point->getX() == x && current->point->getY() == y) {
                    if (current->previous != nullptr) {
                        current->previous->next = current->next;
                    } else {
                        head = current->next;
                    }

                    if (current->next != nullptr) {
                        current->next->previous = current->previous;
                    } else {
                        tail = current->previous;
                    }

                    delete current;
                    return;
                }
                current = current->next;
            }
        }

        void removeGeodeticPoint(double longitude, double latitude) {
            ListNode* current = head;

            while (current != nullptr) {
                if (current->point->getLongitude() == longitude && current->point->getLatitude() == latitude) {
                    if (current->previous != nullptr) {
                        current->previous->next = current->next;
                    } else {
                        head = current->next;
                    }

                    if (current->next != nullptr) {
                        current->next->previous = current->previous;
                    } else {
                        tail = current->previous;
                    }

                    delete current;
                    return;
                }
                current = current->next;
            }
        }

        void removeEarthCentredPoint(double x, double y, double z) {
            ListNode* current = head;

            while (current != nullptr) {
                if (current->point->getECEF_x() == x && current->point->getECEF_y() == y && current->point->getECEF_z() == z) {
                    if (current->previous != nullptr) {
                        current->previous->next = current->next;
                    } else {
                        head = current->next;
                    }

                    if (current->next != nullptr) {
                        current->next->previous = current->previous;
                    } else {
                        tail = current->previous;
                    }

                    delete current;
                    return;
                }
                current = current->next;
            }
        }

        bool areEqual(Waypoint* point1, Waypoint* point2) {
            return (point1->getX() == point2->getX()) && (point1->getY() == point2->getY());
        }
};

//////////////////////////////////////////////////////////////////////////
// Navigation Class
//////////////////////////////////////////////////////////////////////////

class NavigationSystem : public rclcpp::Node{

    // Attributes
private:
    bool debug;                 // displays added information to the terminal for debugging purposes (false by default)
    bool returnToBase;          // boolean to select the direction of travel along selected route 

    int obstacleCount;          // keeps track of how many obstacles are currently stored in the system 
    int obstacleLimit;          // maximum number of obstacles that can be stored in the system 

    /*
        Map Limits are used to define the available area that this node may reroute within. 
        without this, we could get the situation where the computer decides to reroute through Edmonton to avoid obstacles
        while this would work to avoid the obstacle, it's hardly practical/legal in the eyes of CIRC
    */
    double northernMapLimit; // distance Vertically from home base to the upper edge of the testing area
    double southernMapLimit; // distance Vertically from home base to the lower edge of the testing area
    double westernMapLimit;  // distance Horizontally from home base to the Left-most edge of the testing area
    double easternMapLimit;  // distance Horizontally from home base to the Right-most edge of the testing area
    
    enum coordinates {local, geodetic, earthCentered};  // defines the three coordinate systems available to the pilot 
    enum coordinates preference; // used to select which form of the coordinates gets displayed to the pilot

    enum avoidanceStrategy {reroute, trackCrawling, automatic_Circumnavigation_Off}; // defines the methods of circumnavigating obstacles
    enum avoidanceStrategy circumnavigationStyle; // used to select how circumnavigation of obstacles is tackled

    enum preplannedRoute {Route1, Route2, Route3, Route4, Route5}; // defines up to 5 preplanned routes for the operator to select
    enum preplannedRoute routeSelected; // to allow the operator to select which preplanned route to follow
    
    Waypoint* currentPosition;    // Waypoint to store the current position of the rover (updated often)
    Velocity* currentVelocity;    // Speed and direction of rover (calculated from positional data from GPS)

    ListOfWaypoints pointsVisited;   // Doubly linked list to store the path that the rover has actually taken (whether it was planned or not)
    Obstacle* obstacles[10];         // Array to store Obstacles in the way of the rover. ordered by x coordinate
    ListOfWaypoints routes[5];       // Array to allow us to store multiple preplanned routes
                                     // each route will be a doubly linked list that will hold several waypoints in the order that they should be visited 
    
    // Getters and Setters
public:

    void toggleDebug(){ debug = !debug; }
    
    void toggleReturnToBase(){ returnToBase = !returnToBase; }
    
    void toggleCoordinatePreference(){
        switch(preference){
            case(local): preference = geodetic;
            case(geodetic): preference = earthCentered;
            default: preference = local;
        }
    }

    void toggleCircumnavigationStyle(){
        switch(circumnavigationStyle){
            case(reroute): circumnavigationStyle = trackCrawling;
            case(trackCrawling): circumnavigationStyle = automatic_Circumnavigation_Off;
            default: circumnavigationStyle = reroute;
        }
    }

    void toggleRouteSelected(){
        switch(routeSelected){
            case(Route1): routeSelected = Route2;
            case(Route2): routeSelected = Route3;
            case(Route3): routeSelected = Route4;
            case(Route4): routeSelected = Route5;
            default: routeSelected = Route1;
        }
    }

    void selectRoute(preplannedRoute route){
        routeSelected = route;
    }

    // Other Methods

private:

    /*
        checkWaypointCollisions()
        ---------------------------------
        checks if any of the waypoints in the currently selected route are within the radius of any known obstacles
        if a collision is detected, it automatically calls the waypointCollisionDetected function to handle the collision based on the selected circumnavigation style
    */
    void checkWaypointCollisions(){
        std::sort(obstacles, obstacles + sizeof(obstacles) / sizeof(obstacles[0]));
        ListNode* current = routes[routeSelected].getHead();
        while (current != nullptr) {
            for (int i = 0; i < obstacleCount; i++) {
                double dx = current->point->getX() - obstacles[i]->getX();
                double dy = current->point->getY() - obstacles[i]->getY();
                if (sqrt(dx*dx + dy*dy) < obstacles[i]->getRadius()) {
                    waypointCollisionDetected(current, obstacles[i]);
                    break; // No need to check other obstacles if a collision is detected
                }
            }
            current = current->next;
        }
    }

    /*
        findWaypointCollision(Waypoint* collidingWaypoint)
        --------------------------------------------------
        this method returns the index of the obstacle that is colliding with our waypoint
    */
    int findWaypointCollision(Waypoint* collidingWaypoint){
        for (int i = 0; i < obstacleCount; i++) {
                double dx = collidingWaypoint->getX() - obstacles[i]->getX();
                double dy = collidingWaypoint->getY() - obstacles[i]->getY();
                if (sqrt(dx*dx + dy*dy) < obstacles[i]->getRadius()) {
                    return i;
                }
            }
        return -1;
    }

    /*
        isWaypointColliding(ListNode* node)
        ---------------------------------
        checks if the given node is within the radius of any of the obstacles currently stored in the system.
    */
    bool isWaypointColliding(ListNode* node){
        return isWaypointColliding(node->point);
    }

     /*
        isWaypointColliding(Waypoint* point)
        ---------------------------------
        checks if the given point is within the radius of any of the obstacles currently stored in the system.
    */
    bool isWaypointColliding(Waypoint* point){
        for (int i = 0; i < obstacleCount; i++) {
            double dx = point->getX() - obstacles[i]->getX();
            double dy = point->getY() - obstacles[i]->getY();
            if (sqrt(dx*dx + dy*dy) < obstacles[i]->getRadius()) {
                return true;
            }
        }
        return false;
    }

    void checkTrackCollisions(){
        ListNode* current = routes[routeSelected].getHead();
        current = current->next;
        while (current != nullptr) {
            for (int i = 0; i < obstacleCount; i++) {
                auto pointOfInterest = calculatePointOfInterest(current->previous, current, obstacles[i]);
                double dx = obstacles[i]->getX() - pointOfInterest[0]; 
                double dy = obstacles[i]->getY() - pointOfInterest[1];
                double distanceToCenter = sqrt(dx * dx + dy * dy);
                if (distanceToCenter < obstacles[i]->getRadius()){
                    trackCollisionDetected(current, obstacles[i]);
                }
            }
            current = current->next;
        }
    }

    void checkTrackCollisions(ListNode* start, ListNode* end, Obstacle* obstacle){
        
        auto pointOfInterest = calculatePointOfInterest(start, end, obstacle);
        double dx = obstacle->getX() - pointOfInterest[0]; 
        double dy = obstacle->getY() - pointOfInterest[1];
        double distanceToCenter = sqrt(dx * dx + dy * dy);
        if (distanceToCenter < obstacle->getRadius()){
            generateDetour(pointOfInterest[0], pointOfInterest[1], end, obstacle);
        }
    }
    
    /*
        Description of Circumnavigation Styles:
        ------------------------------------------------
        Reroute: If a collision is detected between our route and an Obstacle, we will create a new waypoint that is 1m outside of the edge of the obstacle to ensure we avoid it,
                 the algorithm will be recursively called to ensure that any changes made to the route do not create new collisions.

                 any waypoints found to be within the radius of the obstacle will be removed when using this circumnavigation style

                 the inspiration for this approach came from the opening and closing angle method of drift correction used in civil aviation.
                 when navigating in the air, pilots would estimate how far they are off track and then use that angle and the distance to the next waypoint to calculate a new heading
                 that will take them from their current position to the next waypoint while also compensating for the wind that pushed them off course.

                 Obviously, we do not need to worry about wind correction, but the idea is still the same.
                 the goal here is to arrive at the destination and not necessarily to follow a specific route.

        Track Crawling: If a collision is detected between our route and an Obstacle, we will add new waypoints along our track that stop short of the obstacles keep out radius,
                        from there, the Rerouting method will be recursively called to chart a path that roughly follows the outline of the obstacle until we intercept our desired track

                        any Waypoints found within the radius of the obstacle will still be removed, but not before new waypoints are added that will preserve the original track
                        as much as possible.

                        the Inspiration for this approach once again came from aviation. The double angle approach is a method of drift correction that is used when the drift is
                        noticed early in the flight. The pilot will estimate the angle between the current track and desired track to determine what angle they need to add to their
                        current heading to compensate for the wind. The pilot will add double this angle to their current heading until they intercept their original route again,
                        at which point they will fly at their calculated heading to fly directly to the next waypoint while still compensating for the wind.

                        that approach involves some math, so sometimes people will cheat and just eyeball their way back to the desired track. of course, since they don't know the
                        heading to fly to compenste for the wind, they will get pushed off track again and will need to continuously correct their heading. 
                        This is known as "Track Crawling" which is generally fround upon since you don't fly directly to your destination and you don't 100% regain your track.
                        However, it is exactly the kind of approximate perfection we need.

                        since we will be trying to stick to our route despite constantly getting forced off course by obstacles, the moniker seemed appropriate.

                        this approach will follow the original path to the best of the rover's ability and will always make redirections that will stray as little as possible.
        
        Automatic Circumnavigation Off: If a collision is detected between our route and an Obstacle, we will simply print a message to the terminal to alert the operator 
                                        of the collision and leave it up to them to decide how to proceed.
    
    */

    /*
        waypointCollisionDetected(ListNode* node, Obstacle* obstacle)
        ---------------------------------
        this method is called when a collision is detected between a waypoint and an obstacle. 
        it takes in the node that is colliding and the obstacle that it is colliding with and then handles the collision based on the selected circumnavigation style.
    */
    void waypointCollisionDetected(ListNode* node, Obstacle* obstacle){
        switch (circumnavigationStyle){
            case(reroute): routes[routeSelected].remove(node); break;
            case(trackCrawling): {
                auto pointOfInterest = calculatePointOfInterest(node->previous, node, obstacle);
                
                /* create a new node to store another waypoint
                   to get this new waypoint, we will move back 1 Radii + 1m from the Point if Interest (POI) towards the previous waypoint
                   if the POI is the centre of the waypoint, this will make a new waypoint 1m from the edge of the obstacle
                   if not, the new waypoint will be further from the edge of the obstacle
                   this was done for simplicity, but it really doesn't matter because the further the POI is from the centre, the less drastic the course correction anyway
                   so we'll still be following the original route reasonably closely
                */ 
                ListNode* newWaypoint = new ListNode(nullptr, nullptr, 
                    new Waypoint(pointOfInterest[0] - (obstacle->getRadius() + 1) * pointOfInterest[2], pointOfInterest[1] - (obstacle->getRadius() + 1) * pointOfInterest[3]));
                
                routes[routeSelected].addBefore(newWaypoint, node);

                /*  for the next waypoint, we will need to calculate the unit vector to the next waypoint as there's a good change the the path turns to some degree at the waypoint
                    therefore, the direction described by the unit vector calculated with the POI will be incorrect

                    once we have that, the process is identical to above
                */
                double dx = node->next->point->getX() - node->point->getX();
                double dy = node->next->point->getY() - node->point->getY();
                double* unitVectorToNextWaypoint = new double[2]{dx / sqrt(dx * dx + dy* dy), dy / sqrt(dx * dx + dy * dy)};
                
                newWaypoint = new ListNode(nullptr, nullptr, 
                    new Waypoint(pointOfInterest[0] + (obstacle->getRadius() + 1) * unitVectorToNextWaypoint[0], pointOfInterest[1] + (obstacle->getRadius() + 1) * unitVectorToNextWaypoint[1]));

                routes[routeSelected].addBefore(newWaypoint, node);

                delete unitVectorToNextWaypoint;
                newWaypoint = nullptr;
            }; break;
            case(automatic_Circumnavigation_Off): cout << "collision Detected at waypoint (" << node->point->getX() << ", " << node->point->getY() << ")" << endl; break;
        };
    }

    /*
        TrackCollisionDetected(ListNode* node, Obstacle* obstacle)
        ---------------------------------
        this method is called when a collision is detected along the route 
        it handles the collision based on the selected circumnavigation style
    */
    void trackCollisionDetected(ListNode* nextNode, Obstacle* obstacle){
        auto pointOfInterest = calculatePointOfInterest(nextNode->previous, nextNode, obstacle);
        switch (circumnavigationStyle){
            case(reroute): generateDetour(pointOfInterest[0], pointOfInterest[1], nextNode, obstacle); break;
            case(trackCrawling): {
                // to start, the process is identical to when a waypoint collision is detected, but it's even simpler since we know that there isn't a turn or a bend
                // in the route midway through the obstacle
                
                /* create a new node to store another waypoint
                   to get this new waypoint, we will move back 1 Radii + 1m from the Point if Interest (POI) towards the previous waypoint
                   if the POI is the centre of the waypoint, this will make a new waypoint 1m from the edge of the obstacle
                   if not, the new waypoint will be further from the edge of the obstacle
                   this was done for simplicity, but it really doesn't matter because the further the POI is from the centre, the less drastic the course correction anyway
                   so we'll still be following the original route reasonably closely
                */ 
                ListNode* newWaypoint = new ListNode(nullptr, nullptr, 
                    new Waypoint(pointOfInterest[0] - (obstacle->getRadius() + 1) * pointOfInterest[2], pointOfInterest[1] - (obstacle->getRadius() + 1) * pointOfInterest[3]));
                
                routes[routeSelected].addBefore(newWaypoint, nextNode);

                newWaypoint = new ListNode(nullptr, nullptr, 
                    new Waypoint(pointOfInterest[0] + (obstacle->getRadius() + 1) * pointOfInterest[2], pointOfInterest[1] + (obstacle->getRadius() + 1) * pointOfInterest[3]));
                
                routes[routeSelected].addBefore(newWaypoint, nextNode);

                newWaypoint = nullptr;
            }; break;
            case(automatic_Circumnavigation_Off): cout << "collision Detected with Obstacle (" << obstacle->getX() << ", " << obstacle->getY() << ")" << endl; break;
        };
    }

    /*
        generateDetour(double pointOfInterest_X, double pointOfInterest_Y, ListNode* nextNode, Obstacle* obstacle)
        ---------------------------------------------------------------------------------------------------
        this method adds a new waypoint 1m past the edge of the obstacle
        If the new waypoint that it creates is inside of another obstacle, it will try placing it at the nearest apex

        note: by the "nearest apex" I am talking about the point on the edge of the obstacle's radius that lines up with the POI and the centre
        it's the nearest one to the original path an thus will be on the same side of the centre mark as the POI
        Check the design guild for my drawings further discussing this
    */
    void generateDetour(double pointOfInterest_X, double pointOfInterest_Y, ListNode* nextNode, Obstacle* obstacle){
        
        // define unit vector going from POI to centre
        double dx = obstacle->getX() - pointOfInterest_X;
        double dy = obstacle->getY() - pointOfInterest_Y;
        double length = sqrt(dx * dx + dy * dy);
        double* unitVector = new double[2]{dx / length, dy / length};

        // following obstacles are used in the worst case scenario of chaining obstacles
        int obstacle1_index;
        int obstacle2_index;
    
        // try to put new waypoint 1m past the nearest apex
        ListNode* newWaypoint = new ListNode(nullptr, nullptr, 
                    new Waypoint(pointOfInterest_X - (obstacle->getRadius() + 1) * unitVector[0], pointOfInterest_Y - (obstacle->getRadius() + 1) * unitVector[1]));
        
        if (!isWaypointColliding(newWaypoint)){
            // putting it 1m past the nearest apex works fine
            routes[routeSelected].addBefore(newWaypoint, nextNode);\

            // check if the new waypoint created other track collisions. 
            // Next two lines will recursively check for problems and fix them until there are no more collisions resulting from our rerouting
            checkTrackCollisions(nextNode->previous, newWaypoint, obstacle);
            checkTrackCollisions(newWaypoint, nextNode, obstacle);
        }
        else {
            // try to put the new waypoint at the nearest apex
            obstacle1_index = findWaypointCollision(newWaypoint->point);
            delete newWaypoint;
            newWaypoint = new ListNode(nullptr, nullptr, 
                    new Waypoint(pointOfInterest_X - (obstacle->getRadius()) * unitVector[0], pointOfInterest_Y - (obstacle->getRadius()) * unitVector[1]));
            
            if (!isWaypointColliding(newWaypoint)){
                // putting it at the nearest apex works fine
                routes[routeSelected].addBefore(newWaypoint, nextNode);
                
                // check if the new waypoint created other track collisions. 
                // Next two lines will recursively check for problems and fix them until there are no more collisions resulting from our rerouting
                checkTrackCollisions(nextNode->previous, newWaypoint, obstacle);
                checkTrackCollisions(newWaypoint, nextNode, obstacle);
            }
            else {
                // try putting it 1m past the far apex
                delete newWaypoint;
                newWaypoint = new ListNode(nullptr, nullptr, 
                    new Waypoint(pointOfInterest_X + (obstacle->getRadius() + 1) * unitVector[0], pointOfInterest_Y + (obstacle->getRadius() + 1) * unitVector[1]));
                
                if (!isWaypointColliding(newWaypoint)){
                    // putting it 1m past the far apex works fine
                    routes[routeSelected].addBefore(newWaypoint, nextNode);
                    
                    // check if the new waypoint created other track collisions. 
                    // Next two lines will recursively check for problems and fix them until there are no more collisions resulting from our rerouting
                    checkTrackCollisions(nextNode->previous, newWaypoint, obstacle);
                    checkTrackCollisions(newWaypoint, nextNode, obstacle);
                }
                else {
                    // try putting it at the far apex
                    obstacle2_index = findWaypointCollision(newWaypoint->point);
                    delete newWaypoint;
                    newWaypoint = new ListNode(nullptr, nullptr, 
                        new Waypoint(pointOfInterest_X + (obstacle->getRadius()) * unitVector[0], pointOfInterest_Y + (obstacle->getRadius()) * unitVector[1]));
                
                    if (!isWaypointColliding(newWaypoint)){
                        // putting it at the far apex works fine
                        routes[routeSelected].addBefore(newWaypoint, nextNode);
                        
                        // check if the new waypoint created other track collisions. 
                        // Next two lines will recursively check for problems and fix them until there are no more collisions resulting from our rerouting
                        checkTrackCollisions(nextNode->previous, newWaypoint, obstacle);
                        checkTrackCollisions(newWaypoint, nextNode, obstacle);
                    }
                    else {
                        /*
                            At this point, we are dealing with a chain of obstacles where the first obstacle we looked at is not at the end
                            This means that we must change our strategy
                            now, we will leap frog along to find the ends of the chain and we will create 2 new waypointsd at either end
                            we will then compare these to the POI to find out which one is the shortest detour
                            the waypoint that takes us the least off track will be added and the other will be removed
                        */
                        int originalObstacleIndex = getIndexOfObstacle(obstacle);
                        Waypoint* alternative1 = findAlternateRoute(originalObstacleIndex, obstacle1_index);
                        Waypoint* alternative2 = findAlternateRoute(originalObstacleIndex, obstacle2_index);
                        delete newWaypoint;
                        
                        // calculate the distance to alternative 1
                        dx = alternative1->getX() - pointOfInterest_X;
                        dy = alternative1->getY() - pointOfInterest_Y;
                        double length1 = sqrt(dx * dx + dy * dy);

                        // calculate the distancec to alternative 2
                        dx = alternative2->getX() - pointOfInterest_X;
                        dy = alternative2->getY() - pointOfInterest_Y;
                        double length2 = sqrt(dx * dx + dy * dy);

                        // compare and keep the nearest one
                        if(length1 <= length2){
                            newWaypoint = new ListNode(nullptr, nullptr, alternative1);
                            delete alternative2;
                        }
                        else {
                            newWaypoint = new ListNode(nullptr, nullptr, alternative2);
                            delete alternative1;
                        }
                        routes[routeSelected].addBefore(newWaypoint, nextNode);
                        
                        // check if the new waypoint created other track collisions. 
                        // Next two lines will recursively check for problems and fix them until there are no more collisions resulting from our rerouting
                        checkTrackCollisions(nextNode->previous, newWaypoint, obstacle);
                        checkTrackCollisions(newWaypoint, nextNode, obstacle);
                    }
                }
            }

        }
        delete[] unitVector;
    }

    Waypoint* findAlternateRoute(int first, int second){
        // define unit vector going from the centre of the original obstacle to the next one
        double dx = obstacles[second]->getX() - obstacles[first]->getX();
        double dy = obstacles[second]->getY() - obstacles[first]->getY();
        double length = sqrt(dx * dx + dy * dy);
        double* unitVector = new double[2]{dx / length, dy / length};

        Waypoint* possibleWaypoint = new Waypoint(obstacles[second]->getX() + (obstacles[second]->getRadius() + 1) * unitVector[0], obstacles[second]->getY() + (obstacles[second]->getRadius() + 1) * unitVector[2]);

        if(!isWaypointColliding(possibleWaypoint)){
            // We found a potential waypoint
            delete unitVector;
            return possibleWaypoint;
        }
        else {
            //try the same trick as before where we put the new point right on the edge of the obstacle to thread the nedle between 2 adjacent obstacles
            delete possibleWaypoint;
            possibleWaypoint = new Waypoint(obstacles[second]->getX() + obstacles[second]->getRadius() * unitVector[0], obstacles[second]->getY() + obstacles[second]->getRadius() * unitVector[2]);
            
            if(!isWaypointColliding(possibleWaypoint)){
                // we found a potential waypoint
                delete unitVector;
                return possibleWaypoint;
            }

            else {
                //the chain continues. if this is the case, we recursively call this function until we get to the end
                delete unitVector;
                int indexOfNextObstacle = findWaypointCollision(possibleWaypoint);
                delete possibleWaypoint;
                return findAlternateRoute(second, indexOfNextObstacle);
            }
        }
    }



    /*
        calculatePointOfInterest(ListNode* start, ListNode* end, Obstacle* obstacle)
        ------------------------------------------------------  
        this method calculates the "point of interest" which is a point that lies exacly halfway between the 2 intersection points of the route with the obstacle.

        this method also returns the unit vector representing the direction of the route at this point so that we can use it in futur calculations

        this is done via some vector math. we get the unit vector for the direction of the route and then project the vector going from the first waypoint to the centre
        of the obstacle. The tip of this projection marks our POI.

        this POI will be used when the "track crawling" circumnavigation style is selected to find waypoints that are barely past the edge of the obstacle and still on the original path
        It will also be used to detect track collisions as if this POI is within 1 radii of the obstacle's center, then we have a collision

        Check the design guild for the associated drawing further breaking down the math
    */
    array<double, 4> calculatePointOfInterest(ListNode* start, ListNode* end, Obstacle* obstacle){
        double x, y;

        // calculate vectors to the next waypoint and to the obstacle
        double* vectorToNextWaypoint = new double[2]{end->point->getX() - start->point->getX(), end->point->getY() - start->point->getY()};
        double* vectorToObstacle = new double[2]{obstacle->getX() - start->point->getX(), obstacle->getY() - start->point->getY()};

        // calculate unit vector representing the direction of the route
        double* unitVectorToNextWaypoint = new double [2]{vectorToNextWaypoint[0] / sqrt(vectorToNextWaypoint[0]*vectorToNextWaypoint[0] + vectorToNextWaypoint[1]*vectorToNextWaypoint[1]), 
                                            vectorToNextWaypoint[1] / sqrt(vectorToNextWaypoint[0]*vectorToNextWaypoint[0] + vectorToNextWaypoint[1]*vectorToNextWaypoint[1])};
        
        // project one vector onto the other to find out how far along the route the POI is
        double projectionLength = vectorToObstacle[0] * unitVectorToNextWaypoint[0] + vectorToObstacle[1] * unitVectorToNextWaypoint[1];
        
        // add use the projection length and the direction to get the coordinates of the POI
        x = start->point->getX() + projectionLength * unitVectorToNextWaypoint[0];
        y = start->point->getY() + projectionLength * unitVectorToNextWaypoint[1];

        // free up memory
        delete vectorToNextWaypoint;
        delete vectorToObstacle;
        delete unitVectorToNextWaypoint;

        return {x, y, unitVectorToNextWaypoint[0], unitVectorToNextWaypoint[1]};
    }

    int getIndexOfObstacle(Obstacle* thing){
        for(int i = 0; i < obstacleCount; i++){
            if(thing->getX() == obstacles[i]->getX() && thing->getY() == obstacles[i]->getY())
                return i;
        }
        return -1;
    }

    // Constructor
public:
    NavigationSystem() : Node("NAVnode"), count_(0), debug(false), returnToBase(false), obstacleCount(0), obstacleLimit(10),
                         northernMapLimit(100), southernMapLimit(-100), westernMapLimit(-100), easternMapLimit(100),
                         preference(local), circumnavigationStyle(reroute), routeSelected(Route1) {
        currentPosition = new Waypoint(0.0, 0.0);
        currentVelocity = new Velocity();
    }

    // Define Topics

    // Define Services

    // Define Actions
};

//////////////////////////////////////////////////////////////////////////
// TODO
//////////////////////////////////////////////////////////////////////////

/*
    - finish logic behind the Velocity class

    - finish logic behind the Navsystem class -> add checks for track craling method to ensure any new waypoints created are not colliding with obstacles

    - define our ros related stuff

    - add logic to the main function to create the node and spin it

    - add a serial bridge to communicate with the GNSS module

    - add functionality to allow the array of obstacles to be dynamically resized (currently set to 10 for simplicity)

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