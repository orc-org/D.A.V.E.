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

//custom defined interface for use with topics and services (defined in GPS node implementation)
#include "navigation_interfaces/msg/bearing_string.hpp"
#include "navigation_interfaces/msg/position_string.hpp"
#include "navigation_interfaces/srv/get_nav.hpp"
#include "navigation_interfaces/srv/set_nav.hpp"

//All Data Structures and objects we've defined
#include "ListOfWaypoints.cpp"
#include "Waypoint.cpp"
#include "Obstacle.cpp"


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

    rclcpp::Service<navigation_interfaces::srv::VoidService>::SharedPtr StopNavigatingServer;
    rclcpp::Service<navigation_interfaces::srv::SelectRoute>::SharedPtr SelectRouteServer;
    rclcpp::Service<navigation_interfaces::srv::ResetHome>::SharedPtr ResetHomeServer;
    rclcpp::Service<navigation_interfaces::srv::VoidService>::SharedPtr ClearRouteServer;
        
    rclcpp::Service<navigation_interfaces::srv::VoidService>::SharedPtr ToggleDebugServer;
    rclcpp::Service<navigation_interfaces::srv::VoidService>::SharedPtr TogglePreferenceServer;
    rclcpp::Service<navigation_interfaces::srv::VoidService>::SharedPtr ToggleRouteServer;
    rclcpp::Service<navigation_interfaces::srv::VoidService>::SharedPtr ToggleCircumnavigationStyleServer;
    rclcpp::Service<navigation_interfaces::srv::VoidService>::SharedPtr ToggleDirectionServer;

    rclcpp::Service<navigation_interfaces::srv::AddLocalWaypoint>::SharedPtr AddLocalWaypointServer;
    rclcpp::Service<navigation_interfaces::srv::AddLocalWaypointAtIndex>::SharedPtr AddLocalWaypointAtIndexServer;
    rclcpp::Service<navigation_interfaces::srv::AddLocalObstacle>::SharedPtr AddLocalObstacleServer;

    rclcpp::Service<navigation_interfaces::srv::AddGeodeticWaypoint>::SharedPtr AddGeodeticWaypointServer;
    rclcpp::Service<navigation_interfaces::srv::AddGeodeticWaypointAtIndex>::SharedPtr AddGeodeticWaypointAtIndexServer;
    rclcpp::Service<navigation_interfaces::srv::AddGeodeticObstacle>::SharedPtr AddGeodeticObstacleServer;

    rclcpp::Service<navigation_interfaces::srv::AddEarthCentredWaypoint>::SharedPtr AddEarthCentredWaypointServer;
    rclcpp::Service<navigation_interfaces::srv::AddEarthCentredWaypointAtIndex>::SharedPtr AddEarthCentredWaypointAtIndexServer;
    rclcpp::Service<navigation_interfaces::srv::AddEarthCentredObstacle>::SharedPtr AddEarthCentredObstacleServer;

    // Actions (work in progress)

    //FollowRoute -> will begin the route (like hitting start on google maps)
    //ConfirmRoute -> will check the route for collisions with obstacles

    // Timers
    rclcpp::TimerBase::SharedPtr timer;

    ////////////////////////////////////////////
    // End Ros stuff Definition
    ////////////////////////////////////////////

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
                   to get this new waypoint, we will move back 1 Radii + 1m from the Point of Interest (POI) towards the previous waypoint
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
            routes[routeSelected].addBefore(newWaypoint, nextNode);

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
        double x, y, x_hat, y_hat;

        // calculate vectors to the next waypoint and to the obstacle
        double* vectorToNextWaypoint = new double[2]{end->point->getX() - start->point->getX(), end->point->getY() - start->point->getY()};
        double* vectorToObstacle = new double[2]{obstacle->getX() - start->point->getX(), obstacle->getY() - start->point->getY()};

        // calculate unit vector representing the direction of the route
        double* unitVectorToNextWaypoint = new double [2]{vectorToNextWaypoint[0] / sqrt(vectorToNextWaypoint[0]*vectorToNextWaypoint[0] + vectorToNextWaypoint[1]*vectorToNextWaypoint[1]), 
                                            vectorToNextWaypoint[1] / sqrt(vectorToNextWaypoint[0]*vectorToNextWaypoint[0] + vectorToNextWaypoint[1]*vectorToNextWaypoint[1])};
        
        // project one vector onto the other to find out how far along the route the POI is
        double projectionLength = vectorToObstacle[0] * unitVectorToNextWaypoint[0] + vectorToObstacle[1] * unitVectorToNextWaypoint[1];
        
        // use the projection length and the direction to get the coordinates of the POI
        x = start->point->getX() + projectionLength * unitVectorToNextWaypoint[0];
        y = start->point->getY() + projectionLength * unitVectorToNextWaypoint[1];
        
        x_hat = unitVectorToNextWaypoint[0];
        y_hat = unitVectorToNextWaypoint[1];

        // free up memory
        delete vectorToNextWaypoint;
        delete vectorToObstacle;
        delete unitVectorToNextWaypoint;

        return {x, y, x_hat, y_hat};
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
    NAVnode() : Node("NAVnode") {
        
        currentPosition = new Waypoint(0.0, 0.0);
        currentVelocity.speed = 0;
        currentVelocity.heading = 0;

        debug = false;
        returnToBase = false;
        
        obstacleCount = 0;
        obstacleLimit = 10;
        waypointCount = 0;
        
        northernMapLimit = 100;
        southernMapLimit = 100;
        westernMapLimit = 100;
        easternMapLimit = 100;
        
        preference = local;
        circumnavigationStyle = reroute;
        routeSelected = Route1;

        // create topics
        // here, we're using a queue size of 1 because it is more desireable to lose some data
        // than to be using outdated position data
        VelocityPublisher = this->create_publisher<navigation_interfaces::msg::Velocity>("Velocity", 1);


        // create services
        StopNavigatingServer = this->create_service<navigation_interfaces::srv::VoidService>("StopNavigating", &stopNavigating);
        SelectRouteServer = this->create_service<navigation_interfaces::srv::SelectRoute>("SelectRoute", &selectRoute);
        ResetHomeServer = this->create_service<navigation_interfaces::srv::ResetHome>("ResetHome", &resetHome);
        ClearRouteServer = this->create_service<navigation_interfaces::srv::VoidService>("clearRoute", &clearRoute);
        
        ToggleDebugServer = this->create_service<navigation_interfaces::srv::VoidService>("ToggleDebug", &toggleDebug);
        TogglePreferenceServer = this->create_service<navigation_interfaces::srv::VoidService>("TogglePreference", &togglePreference);
        ToggleRouteServer = this->create_service<navigation_interfaces::srv::VoidService>("ToggleDebug", &togglePreference);
        ToggleCircumnavigationStyleServer = this->create_service<navigation_interfaces::srv::VoidService>("ToggleDebug", &toggleCircumnavigationStyle);
        ToggleDirectionServer = this->create_service<navigation_interfaces::srv::VoidService>("ToggleDebug", &toggleDirection);

        AddLocalWaypointServer = this->create_service<navigation_interfaces::srv::AddLocalWaypoint>("AddLocalWaypoint", &addLocalWaypoint);
        AddLocalWaypointAtIndexServer = this->create_service<navigation_interfaces::srv::AddLocalWaypointAtIndex>("AddLocalWaypointAtIndex", &addLocalWaypointAtIndex);
        AddLocalObstacleServer = this->create_service<navigation_interfaces::srv::AddLocalObstacle>("AddLocalObstacle", &addLocalObstacle);

        AddGeodeticWaypointServer = this->create_service<navigation_interfaces::srv::AddGeodeticWaypoint>("AddGeodeticWaypoint", &addGeodeticWaypoint);
        AddGeodeticWaypointAtIndexServer = this->create_service<navigation_interfaces::srv::AddGeodeticWaypointAtIndex>("AddGeodeticWaypointAtIndex", &addGeodeticWaypointAtIndex);
        AddGeodeticObstacleServer = this->create_service<navigation_interfaces::srv::AddGeodeticObstacle>("AddGeodeticObstacle", &addGeodeticObstacle);

        AddEarthCentredWaypointServer = this->create_service<navigation_interfaces::srv::AddEarthCentredWaypoint>("AddEarthCentredWaypoint", &addEarthCentredWaypoint);
        AddEarthCentredWaypointAtIndexServer = this->create_service<navigation_interfaces::srv::AddEarthCentredWaypointAtIndex>("AddEarthCentredWaypointAtIndex", &addEarthCentredWaypointAtIndex);
        AddEarthCentredObstacleServer = this->create_service<navigation_interfaces::srv::AddEarthCentredObstacle>("AddEarthCentredObstacle", &addEarthCentredObstacle);
        
        // main timer will process gnss data once per second
        timer = this->create_wall_timer(1s, std::bind(&NAVnode::mainTimer, this));

        // velocity timer will calculate and publish current velocity vector every 5 seconds
        //auto secondaryTimer = this->create_wall_timer(5s, std::bind(&NavigationSystem::velocityTimer, this));
    }

    // timers
    void mainTimer(){
        updatePositonData();
        pointsVisited.add(currentPosition);

        if (debug){
            // debug mode will print the current value for all our variables to the ros terminal thatt the navigation node is active in
            RCLCPP_INFO(this->getLogger(), 
                "Current coordinates: %f N %f W \n\r" + 
                "Current velocity: %f m/s @ %f degrees \n\r" +
                "Route Selected: %s Coordinate Preference: %s Circumnavigation style: %s \n\r" +
                "Number of Waypoints: %d Number of Obstacles: %d Current Maximun number of obstacles: %d \n\n\n", 
                currentPosition->getLatitude(), currentPosition->getLatitude(),
                currentVelocity->speed, currentVelocity->direction,
                routeSelected, preference, circumnavigationStyle,
                waypointCount, obstacleCount, obstacleLimit);
        }

        lastFiveWaypoints[timeSinceLastVelocityPublishing] = currentPosition;

        if (timeSinceLastVelocityPublishing >= 5){
            updateVelocity();
            VelocityPublisher->publish(currentVelocity);
        }
    }

    //void velocityTimer(){}

    /*
        updatePositionData()
        --------------------
        method that will get the current position of the rover from the gnss module and update the "currentPosition" Waypoint
    */
    void updatePositonData(){
        
    }

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
    void updateVelocity(){
        double x,y,dX,dY;

        for(int i = 1; i < 5; i++){
            dX += lastFiveWaypoints[i]->getX() - lastFiveWaypoints[0]->getX();
            dY += lastFiveWaypoints[i]->getY() - lastFiveWaypoints[0]->getY();
        }

        x = abs(dX/4); 
        y = abs(dY/4);

        // calculate speed
        currentVelocity.speed = sqrt(x*x + y*y)/4;

        //calculate heading
        if (dX == 0 && dY == 0);
        else if (dX > 0 && dY > 0) // 1st quadrant
            currentVelocity.heading = 90  - currentPosition->radToDeg(std::asin(y/x));
        else if (dX < 0 && dY > 0) // 2nd quadrant
            currentVelocity.heading = 270 + currentPosition->radToDeg(std::asin(y/x));
        else if (dX < 0 && dY < 0) // 3rd quadrant
            currentVelocity.heading = 270 - currentPosition->radToDeg(std::asin(y/x));
        else if (dX > 0 && dY < 0) // 4th quadrant
            currentVelocity.heading = 90  + currentPosition->radToDeg(std::asin(y/x));
        
    }

    // service callback functions
    void stopNavigating(const std::shared_ptr<navigation_interfaces::srv::VoidService::Request> request, std::shared_ptr<navigation_interfaces::srv::VoidService::Response> response){

    }

    void togglePreference(const std::shared_ptr<navigation_interfaces::srv::VoidService::Request> request, std::shared_ptr<navigation_interfaces::srv::VoidService::Response> response){
        switch(preference){
            case(local): preference = geodetic;
            case(geodetic): preference = earthCentered;
            default: preference = local;
        }
    }

    void toggleRoute(const std::shared_ptr<navigation_interfaces::srv::VoidService::Request> request, std::shared_ptr<navigation_interfaces::srv::VoidService::Response> response){
        switch(routeSelected){
            case(Route1): routeSelected = Route2;
            case(Route2): routeSelected = Route3;
            case(Route3): routeSelected = Route4;
            case(Route4): routeSelected = Route5;
            default: routeSelected = Route1;
        }
    }

    void toggleDebug(const std::shared_ptr<navigation_interfaces::srv::VoidService::Request> request, std::shared_ptr<navigation_interfaces::srv::VoidService::Response> response){
        debug = !debug;
    }

    void toggleDirection(const std::shared_ptr<navigation_interfaces::srv::VoidService::Request> request, std::shared_ptr<navigation_interfaces::srv::VoidService::Response> response){
        returnToBase = !returnToBase;
    }

    void toggleCircumnavigationStyle(const std::shared_ptr<navigation_interfaces::srv::VoidService::Request> request, std::shared_ptr<navigation_interfaces::srv::VoidService::Response> response){
        switch(circumnavigationStyle){
            case(reroute): circumnavigationStyle = trackCrawling;
            case(trackCrawling): circumnavigationStyle = automatic_Circumnavigation_Off;
            default: circumnavigationStyle = reroute;
        }
    }

    void selectRoute(const std::shared_ptr<navigation_interfaces::srv::SelectRoute::Request> request, std::shared_ptr<navigation_interfaces::srv::SelectRoute::Response> response){
        switch(request->route){
            case Route1: routeSelected = Route1; break;
            case Route2: routeSelected = Route2; break;
            case Route3: routeSelected = Route3; break;
            case Route4: routeSelected = Route4; break;
            case Route5: routeSelected = Route5; break;
            default: cout << "Error: invalid Route selection" << endl;
        }
    }

    void resetHome(const std::shared_ptr<navigation_interfaces::srv::ResetHome::Request> request, std::shared_ptr<navigation_interfaces::srv::ResetHome::Response> response){
        if (Waypoint::checkPassword(request->password))
            response->success = true;
        else 
            response->success = false;
    }

    void clearRoute(const std::shared_ptr<navigation_interfaces::srv::VoidService::Request> request, std::shared_ptr<navigation_interfaces::srv::VoidService::Response> response){
        routes[routeSelected].clear();
    }

    void addLocalWaypoint(const std::shared_ptr<navigation_interfaces::srv::AddLocalWaypoint::Request> request, std::shared_ptr<navigation_interfaces::srv::AddLocalWaypoint::Response> response){
        routes[routeSelected].add(new Waypoint(request->x, request->y));
    }

    void addLocalWaypointAtIndex(const std::shared_ptr<navigation_interfaces::srv::AddLocalWaypointAtIndex::Request> request, std::shared_ptr<navigation_interfaces::srv::AddLocalWaypointAtIndex::Response> response){
        routes[routeSelected].add(new Waypoint(request->x, request->y), request->index);
    }

    void addLocalObstacle(const std::shared_ptr<navigation_interfaces::srv::AddLocalObstacle::Request> request, std::shared_ptr<navigation_interfaces::srv::AddLocalObstacle::Response> response){
        // check if the array of obstacles is full
        if (obstacleCount == obstacleLimit){
            // increase make a larger array to store obstacles
            obstacleLimit += 10;
            Obstacle* biggerArray[obstacleLimit] = {};

            // copy over existing obstacles (by iterating through existing array)
            for(int i = 0; i < obstacleLimit - 10; i++){
                biggerArray[i] = obstacles[i];
                obstacles[i] = nullptr; // this is so we don't delete our obstacles when we purge the old array from memory
            }

            // clean up memory and rename our new array (by reusing the old pointer);
            delete obstacles;
            obstacles = biggerArray;
        }

        // add new obstacle. post incrementing to handle indexing differences
        if (request->radius <= 0)
            obstacles[obstacleCount++] = new Obstacle(request->x, request->y); // use default radius if no radius is entered
        else
            obstacles[obstacleCount++] = new Obstacle(request->x, request->y, request->radius);
    }

    void addGeodeticWaypoint(const std::shared_ptr<navigation_interfaces::srv::AddGeodeticWaypoint::Request> request, std::shared_ptr<navigation_interfaces::srv::AddGeodeticWaypoint::Response> response){
        routes[routeSelected].add(new Waypoint(request->longitude, request->latitude, request->altitude, true));
    }

    void addGeodeticWaypointAtIndex(const std::shared_ptr<navigation_interfaces::srv::AddGeodeticWaypointAtIndex::Request> request, std::shared_ptr<navigation_interfaces::srv::AddGeodeticWaypointAtIndex::Response> response){
        routes[routeSelected].add(new Waypoint(request->longitude, request->latitude, request->altitude, true), request->index);
    }

    void addGeodeticObstacle(const std::shared_ptr<navigation_interfaces::srv::AddGeodeticObstacle::Request> request, std::shared_ptr<navigation_interfaces::srv::AddGeodeticObstacle::Response> response){
        // check if the array of obstacles is full
        if (obstacleCount == obstacleLimit){
            // increase make a larger array to store obstacles
            obstacleLimit += 10;
            Obstacle* biggerArray[obstacleLimit] = {};

            // copy over existing obstacles (by iterating through existing array)
            for(int i = 0; i < obstacleLimit - 10; i++){
                biggerArray[i] = obstacles[i];
                obstacles[i] = nullptr; // this is so we don't delete our obstacles when we purge the old array from memory
            }

            // clean up memory and rename our new array (by reusing the old pointer);
            delete obstacles;
            obstacles = biggerArray;
        }

        // add new obstacle. post incrementing to handle indexing differences
        if (request->radius <= 0)
            obstacles[obstacleCount++] = new Obstacle(request->longitude, request->latitude, request->altitude, true); // use default radius if no radius is entered
        else
            obstacles[obstacleCount++] = new Obstacle(request->longitude, request->latitude, request->altitude, request->radius, true);
    }

    void addEarthCentredWaypoint(const std::shared_ptr<navigation_interfaces::srv::AddEarthCentredWaypoint::Request> request, std::shared_ptr<navigation_interfaces::srv::AddEarthCentredWaypoint::Response> response){
        routes[routeSelected].add(new Waypoint(request->x, request->y, request->z, false));
    }

    void addEarthCentredWaypointAtIndex(const std::shared_ptr<navigation_interfaces::srv::AddEarthCentredWaypointAtIndex::Request> request, std::shared_ptr<navigation_interfaces::srv::AddEarthCentredWaypointAtIndex::Response> response){
        routes[routeSelected].add(new Waypoint(request->x, request->y, request->z, false), request->index);
    }

    void addEarthCentredObstacle(const std::shared_ptr<navigation_interfaces::srv::AddEarthCentredObstacle::Request> request, std::shared_ptr<navigation_interfaces::srv::AddEarthCentredObstacle::Response> response){
        // check if the array of obstacles is full
        if (obstacleCount == obstacleLimit){
            // increase make a larger array to store obstacles
            obstacleLimit += 10;
            Obstacle* biggerArray[obstacleLimit] = {};

            // copy over existing obstacles (by iterating through existing array)
            for(int i = 0; i < obstacleLimit - 10; i++){
                biggerArray[i] = obstacles[i];
                obstacles[i] = nullptr; // this is so we don't delete our obstacles when we purge the old array from memory
            }

            // clean up memory and rename our new array (by reusing the old pointer);
            delete obstacles;
            obstacles = biggerArray;
        }

        // add new obstacle. post incrementing to handle indexing differences
        if (request->radius <= 0)
            obstacles[obstacleCount++] = new Obstacle(request->x, request->y, request->z, false); // use default radius if no radius is entered
        else
            obstacles[obstacleCount++] = new Obstacle(request->x, request->y, request->z, request->radius, false);
    }






    // action callback functions
};

//////////////////////////////////////////////////////////////////////////
// TODO
//////////////////////////////////////////////////////////////////////////

/*
    - finish logic behind the Navsystem class -> add checks for track craling method to ensure any new waypoints created are not colliding with obstacles

    - define our ros related stuff

    - add logic to the main function to create the node and spin it

    - add a serial bridge to communicate with the GNSS module

    - update documentation
*/

//////////////////////////////////////////////////////////////////////////
// END TODO
//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<NAVnode>());
    rclcpp::shutdown();

    return 0;
}