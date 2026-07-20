#pragma once

// used for functions
#include <cstdio>
#include <string>
#include <array>
//#include <list>
#include <vector>
#include <cmath>
#include <sstream>
#include <iostream>
#include <algorithm>

// used for ros publishers and servers
#include <chrono>
#include <functional>
#include <memory>
#include <iomanip>

//All Data Structures and objects we've defined
#include "ListOfWaypoints.hpp"
#include "Waypoint.hpp"
#include "Obstacle.hpp"
#include "NAVnode.hpp"

//custom defined interfaces for use with topics, services and actions
#include "navigation_interfaces/msg/velocity.hpp"

#include "navigation_interfaces/srv/void_service.hpp"
#include "navigation_interfaces/srv/reset_home.hpp"
#include "navigation_interfaces/srv/select_route.hpp"

#include "navigation_interfaces/srv/add_local_waypoint.hpp"
#include "navigation_interfaces/srv/add_local_waypoint_at_index.hpp"
#include "navigation_interfaces/srv/add_local_obstacle.hpp"

#include "navigation_interfaces/srv/add_geodetic_waypoint.hpp"
#include "navigation_interfaces/srv/add_geodetic_waypoint_at_index.hpp"
#include "navigation_interfaces/srv/add_geodetic_obstacle.hpp"

#include "navigation_interfaces/srv/add_earth_centred_waypoint.hpp"
#include "navigation_interfaces/srv/add_earth_centred_waypoint_at_index.hpp"
#include "navigation_interfaces/srv/add_earth_centred_obstacle.hpp"

#include "navigation_interfaces/action/navigation_action.hpp"

//interfaces needed to work with ros
#include "rclcpp/rclcpp.hpp"

using VoidService = navigation_interfaces::srv::VoidService;
using SelectRoute = navigation_interfaces::srv::SelectRoute;
using ResetHome = navigation_interfaces::srv::ResetHome;

using AddLocalWaypoint = navigation_interfaces::srv::AddLocalWaypoint;
using AddLocalWaypointAtIndex = navigation_interfaces::srv::AddLocalWaypointAtIndex;
using AddLocalObstacle = navigation_interfaces::srv::AddLocalObstacle;

using AddGeodeticWaypoint = navigation_interfaces::srv::AddGeodeticWaypoint;
using AddGeodeticWaypointAtIndex = navigation_interfaces::srv::AddGeodeticWaypointAtIndex;
using AddGeodeticObstacle = navigation_interfaces::srv::AddGeodeticObstacle;

using AddEarthCentredWaypoint = navigation_interfaces::srv::AddEarthCentredWaypoint;
using AddEarthCentredWaypointAtIndex = navigation_interfaces::srv::AddEarthCentredWaypointAtIndex;
using AddEarthCentredObstacle = navigation_interfaces::srv::AddEarthCentredObstacle;

class NAVnode : public rclcpp::Node{

    // Attributes
private:
    bool debug;                 // displays added information to the terminal for debugging purposes (false by default)
    bool returnToBase;          // boolean to select the direction of travel along selected route 

    int obstacleCount;          // keeps track of how many obstacles are currently stored in the system 
    int obstacleLimit;          // maximum number of obstacles that can be stored in the system
    int waypointCount;          // keeps track of the number of waypoints stored in the system accross all routes 

    int timeSinceLastVelocityPublishing; // used in the numerical method for calculating the velocity of the rover


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

    ListOfWaypoints pointsVisited;   // Doubly linked list to store the path that the rover has actually taken (whether it was planned or not)
    Obstacle** obstacles;            // Pointer to dynamic array to store Obstacles in the way of the rover. ordered by x coordinate
    ListOfWaypoints routes[5];       // Array to allow us to store multiple preplanned routes
                                     // each route will be a doubly linked list that will hold several waypoints in the order that they should be visited 
    Waypoint* lastFiveWaypoints[5];  // array to store the 5 most recent waypoints
                                     // this will be used to estimate the rovers velocity

    //////////////////////////////////////////
    // Ros Stuff Definition
    //////////////////////////////////////////

    // Topics
    navigation_interfaces::msg::Velocity currentVelocity = navigation_interfaces::msg::Velocity();
    rclcpp::Publisher<navigation_interfaces::msg::Velocity>::SharedPtr VelocityPublisher; 

    // Services

    rclcpp::Service<VoidService>::SharedPtr StopNavigatingServer;
    rclcpp::Service<SelectRoute>::SharedPtr SelectRouteServer;
    rclcpp::Service<ResetHome>::SharedPtr ResetHomeServer;
    rclcpp::Service<VoidService>::SharedPtr ClearRouteServer;
        
    rclcpp::Service<VoidService>::SharedPtr ToggleDebugServer;
    rclcpp::Service<VoidService>::SharedPtr TogglePreferenceServer;
    rclcpp::Service<VoidService>::SharedPtr ToggleRouteServer;
    rclcpp::Service<VoidService>::SharedPtr ToggleCircumnavigationStyleServer;
    rclcpp::Service<VoidService>::SharedPtr ToggleDirectionServer;

    rclcpp::Service<AddLocalWaypoint>::SharedPtr AddLocalWaypointServer;
    rclcpp::Service<AddLocalWaypointAtIndex>::SharedPtr AddLocalWaypointAtIndexServer;
    rclcpp::Service<AddLocalObstacle>::SharedPtr AddLocalObstacleServer;

    rclcpp::Service<AddGeodeticWaypoint>::SharedPtr AddGeodeticWaypointServer;
    rclcpp::Service<AddGeodeticWaypointAtIndex>::SharedPtr AddGeodeticWaypointAtIndexServer;
    rclcpp::Service<AddGeodeticObstacle>::SharedPtr AddGeodeticObstacleServer;

    rclcpp::Service<AddEarthCentredWaypoint>::SharedPtr AddEarthCentredWaypointServer;
    rclcpp::Service<AddEarthCentredWaypointAtIndex>::SharedPtr AddEarthCentredWaypointAtIndexServer;
    rclcpp::Service<AddEarthCentredObstacle>::SharedPtr AddEarthCentredObstacleServer;

    // Actions (work in progress)

    //FollowRoute -> will begin the route (like hitting start on google maps)
    //ConfirmRoute -> will check the route for collisions with obstacles

    // Timers
    rclcpp::TimerBase::SharedPtr timer;

    //size_t = count_;

    ////////////////////////////////////////////
    // End Ros stuff Definition
    ////////////////////////////////////////////

    public:
    /*
        checkWaypointCollisions()
        ---------------------------------
        checks if any of the waypoints in the currently selected route are within the radius of any known obstacles
        if a collision is detected, it automatically calls the waypointCollisionDetected function to handle the collision based on the selected circumnavigation style
    */
    void checkWaypointCollisions();

    /*
        findWaypointCollision(Waypoint* collidingWaypoint)
        --------------------------------------------------
        this method returns the index of the obstacle that is colliding with our waypoint
    */
    int findWaypointCollision(Waypoint* collidingWaypoint);

    /*
        isWaypointColliding(ListNode* node)
        ---------------------------------
        checks if the given node is within the radius of any of the obstacles currently stored in the system.
    */
    bool isWaypointColliding(ListNode* node);

     /*
        isWaypointColliding(Waypoint* point)
        ---------------------------------
        checks if the given point is within the radius of any of the obstacles currently stored in the system.
    */
    bool isWaypointColliding(Waypoint* point);

    void checkTrackCollisions();

    void checkTrackCollisions(ListNode* start, ListNode* end, Obstacle* obstacle);
    
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
    void waypointCollisionDetected(ListNode* node, Obstacle* obstacle);

    /*
        TrackCollisionDetected(ListNode* node, Obstacle* obstacle)
        ---------------------------------
        this method is called when a collision is detected along the route 
        it handles the collision based on the selected circumnavigation style
    */
    void trackCollisionDetected(ListNode* nextNode, Obstacle* obstacle);

    /*
        generateDetour(double pointOfInterest_X, double pointOfInterest_Y, ListNode* nextNode, Obstacle* obstacle)
        ---------------------------------------------------------------------------------------------------
        this method adds a new waypoint 1m past the edge of the obstacle
        If the new waypoint that it creates is inside of another obstacle, it will try placing it at the nearest apex

        note: by the "nearest apex" I am talking about the point on the edge of the obstacle's radius that lines up with the POI and the centre
        it's the nearest one to the original path an thus will be on the same side of the centre mark as the POI
        Check the design guild for my drawings further discussing this
    */
    void generateDetour(double pointOfInterest_X, double pointOfInterest_Y, ListNode* nextNode, Obstacle* obstacle);

    Waypoint* findAlternateRoute(int first, int second);

    /*
        calculatePointOfInterest(ListNode* start, ListNode* end, Obstacle* obstacle)
        ------------------------------------------------------  
        this method calculates the "point of interest" which is a point that lies exacly halfway between the 2 intersection points of the route with the obstacle.

        in other words: the POI is the point where the a line drawn perpendicular to the path would intersect with the centre of the obstacle

        in other other words: this is the nearest point along the path to the centre of the obstacle

        this method also returns the unit vector representing the direction of the route at this point so that we can use it in futur calculations

        this is done via some vector math. we get the unit vector for the direction of the route and then project the vector going from the first waypoint to the centre
        of the obstacle. The tip of this projection marks our POI.

        this POI will be used when the "track crawling" circumnavigation style is selected to find waypoints that are barely past the edge of the obstacle and still on the original path
        It will also be used to detect track collisions as if this POI is within 1 radii of the obstacle's center, then we have a collision

        Check the design guild for the associated drawing further breaking down the math
    */
    array<double, 4> calculatePointOfInterest(ListNode* start, ListNode* end, Obstacle* obstacle);

    int getIndexOfObstacle(Obstacle* thing);

    std::string getRouteName();

    std::string getPreferenceName();

    std::string getCircumnavigationStyleName();

    // Constructor

    NAVnode();

    // timers
    void mainTimer();

    /*
        updatePositionData()
        --------------------
        method that will get the current position of the rover from the gnss module and update the "currentPosition" Waypoint
    */
    void updatePositonData();

    /*
        updateVelocity()
        --------------------
        method that will estimate the current velocity of the rover
        it will use a vector average over the last 4 waypoints with respect to the position of the 5th to get the velocity

        **NOTE** this calculates the average velocity of the rover over the last 5 seconds
        (or more accurately its an average of the last 4 seconds and there's a 1 second gap between intervals where we calculate the velocity)
        this is aweful, but since we are mainly going to be using the directional data to calibrate the heading indicator that is 
        powered by the IMU, this is good enough
    */
    void updateVelocity();

    // service callback functions
    void stopNavigating(const std::shared_ptr<VoidService::Request> request, std::shared_ptr<VoidService::Response> response);

    void togglePreference(const std::shared_ptr<VoidService::Request> request, std::shared_ptr<VoidService::Response> response);

    void toggleRoute(const std::shared_ptr<VoidService::Request> request, std::shared_ptr<VoidService::Response> response);

    void toggleDebug(const std::shared_ptr<VoidService::Request> request, std::shared_ptr<VoidService::Response> response);

    void toggleDirection(const std::shared_ptr<VoidService::Request> request, std::shared_ptr<VoidService::Response> response);

    void toggleCircumnavigationStyle(const std::shared_ptr<VoidService::Request> request, std::shared_ptr<VoidService::Response> response);

    void selectRoute(const std::shared_ptr<SelectRoute::Request> request, std::shared_ptr<SelectRoute::Response> response);
    
    void resetHome(const std::shared_ptr<ResetHome::Request> request, std::shared_ptr<ResetHome::Response> response);

    void clearRoute(const std::shared_ptr<VoidService::Request> request, std::shared_ptr<VoidService::Response> response);

    void addLocalWaypoint(const std::shared_ptr<AddLocalWaypoint::Request> request, std::shared_ptr<AddLocalWaypoint::Response> response);

    void addLocalWaypointAtIndex(const std::shared_ptr<AddLocalWaypointAtIndex::Request> request, std::shared_ptr<AddLocalWaypointAtIndex::Response> response);

    void addLocalObstacle(const std::shared_ptr<AddLocalObstacle::Request> request, std::shared_ptr<AddLocalObstacle::Response> response);

    void addGeodeticWaypoint(const std::shared_ptr<AddGeodeticWaypoint::Request> request, std::shared_ptr<AddGeodeticWaypoint::Response> response);

    void addGeodeticWaypointAtIndex(const std::shared_ptr<AddGeodeticWaypointAtIndex::Request> request, std::shared_ptr<AddGeodeticWaypointAtIndex::Response> response);

    void addGeodeticObstacle(const std::shared_ptr<AddGeodeticObstacle::Request> request, std::shared_ptr<AddGeodeticObstacle::Response> response);

    void addEarthCentredWaypoint(const std::shared_ptr<AddEarthCentredWaypoint::Request> request, std::shared_ptr<AddEarthCentredWaypoint::Response> response);

    void addEarthCentredWaypointAtIndex(const std::shared_ptr<AddEarthCentredWaypointAtIndex::Request> request, std::shared_ptr<AddEarthCentredWaypointAtIndex::Response> response);

    void addEarthCentredObstacle(const std::shared_ptr<AddEarthCentredObstacle::Request> request, std::shared_ptr<AddEarthCentredObstacle::Response> response);

    // action callback functions
};

