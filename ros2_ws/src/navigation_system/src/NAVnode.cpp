#include "NAVnode.hpp"
#include "Waypoint.hpp"

using std::placeholders::_1;
using std::placeholders::_2;
void NAVnode::checkWaypointCollisions(){
    std::sort(obstacles, obstacles + obstacleCount); 
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

int NAVnode::findWaypointCollision(Waypoint* collidingWaypoint){
    for (int i = 0; i < obstacleCount; i++) {
            double dx = collidingWaypoint->getX() - obstacles[i]->getX();
            double dy = collidingWaypoint->getY() - obstacles[i]->getY();
            if (sqrt(dx*dx + dy*dy) < obstacles[i]->getRadius()) {
                return i;
            }
        }
    return -1;
}

bool NAVnode::isWaypointColliding(ListNode* node){
    return isWaypointColliding(node->point);
}

bool NAVnode::isWaypointColliding(Waypoint* point){
    for (int i = 0; i < obstacleCount; i++) {
        double dx = point->getX() - obstacles[i]->getX();
        double dy = point->getY() - obstacles[i]->getY();
        if (sqrt(dx*dx + dy*dy) < obstacles[i]->getRadius()) {
            return true;
        }
    }
    return false;
}

void NAVnode::checkTrackCollisions(){
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

void NAVnode::checkTrackCollisions(ListNode* start, ListNode* end, Obstacle* obstacle){
        
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

void NAVnode::waypointCollisionDetected(ListNode* node, Obstacle* obstacle){
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

void NAVnode::trackCollisionDetected(ListNode* nextNode, Obstacle* obstacle){
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

void NAVnode::generateDetour(double pointOfInterest_X, double pointOfInterest_Y, ListNode* nextNode, Obstacle* obstacle){
        
    // define unit vector going from POI to centre
    double dx = obstacle->getX() - pointOfInterest_X;
    double dy = obstacle->getY() - pointOfInterest_Y;
    double length = sqrt(dx * dx + dy * dy);
    double* unitVector = new double[2]{dx / length, dy / length}; // only has two places at index 0 and 1

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

Waypoint* NAVnode::findAlternateRoute(int first, int second){
    // define unit vector going from the centre of the original obstacle to the next one
    double dx = obstacles[second]->getX() - obstacles[first]->getX();
    double dy = obstacles[second]->getY() - obstacles[first]->getY();
    double length = sqrt(dx * dx + dy * dy);
    double* unitVector = new double[2]{dx / length, dy / length};

    Waypoint* possibleWaypoint = new Waypoint(obstacles[second]->getX() + (obstacles[second]->getRadius() + 1) * unitVector[0], obstacles[second]->getY() + (obstacles[second]->getRadius() + 1) * unitVector[1]);

    if(!isWaypointColliding(possibleWaypoint)){
        // We found a potential waypoint
        delete unitVector;
        return possibleWaypoint;
    }
    else {
        //try the same trick as before where we put the new point right on the edge of the obstacle to thread the nedle between 2 adjacent obstacles
        delete possibleWaypoint;
        possibleWaypoint = new Waypoint(obstacles[second]->getX() + obstacles[second]->getRadius() * unitVector[0], obstacles[second]->getY() + obstacles[second]->getRadius() * unitVector[1]);
            
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

array<double, 4> NAVnode::calculatePointOfInterest(ListNode* start, ListNode* end, Obstacle* obstacle){
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

int NAVnode::getIndexOfObstacle(Obstacle* thing){
    for(int i = 0; i < obstacleCount; i++){
        if(thing->getX() == obstacles[i]->getX() && thing->getY() == obstacles[i]->getY())
            return i;
    }
    return -1;
}

std::string NAVnode::getRouteName(){
    string name;
    switch(routeSelected){
        case Route1: name = "Route 1"; break;
        case Route2: name = "Route 2"; break;
        case Route3: name = "Route 3"; break;
        case Route4: name = "Route 4"; break;
        case Route5: name = "Route 5"; break;
    }
    return name;
}

std::string NAVnode::getPreferenceName(){
    string name;
    switch(preference){
        case geodetic: name = "Geodetic"; break;
        case local: name = "Local"; break;
        case earthCentered: name = "Earth Centred Earth Fixed"; break;
    }
    return name;
}

std::string NAVnode::getCircumnavigationStyleName(){
    string name;
    switch(circumnavigationStyle){
        case reroute: name = "Reroute"; break;
        case trackCrawling: name = "Track Crawling"; break;
        case automatic_Circumnavigation_Off: name = "Automatic Circumnavigation Off"; break;
    }
    return name;
}

    // Constructor
NAVnode::NAVnode() : Node("NAVnode") {
        
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
    StopNavigatingServer = this->create_service<VoidService>("StopNavigating", std::bind(&NAVnode::stopNavigating, this, _1, _2));
    SelectRouteServer    = this->create_service<SelectRoute>("SelectRoute",    std::bind(&NAVnode::selectRoute, this, _1, _2));
    ResetHomeServer      = this->create_service<ResetHome>  ("ResetHome",      std::bind(&NAVnode::resetHome, this, _1, _2));
    ClearRouteServer     = this->create_service<VoidService>("clearRoute",     std::bind(&NAVnode::clearRoute, this, _1, _2));
        
    ToggleDebugServer                 = this->create_service<VoidService>("ToggleDebug",                 std::bind(&NAVnode::toggleDebug, this, _1, _2));
    TogglePreferenceServer            = this->create_service<VoidService>("TogglePreference",            std::bind(&NAVnode::togglePreference, this, _1, _2));
    ToggleRouteServer                 = this->create_service<VoidService>("TogglePreferenc",             std::bind(&NAVnode::togglePreference, this, _1, _2));
    ToggleCircumnavigationStyleServer = this->create_service<VoidService>("ToggleCircumnavigationStyle", std::bind(&NAVnode::toggleCircumnavigationStyle, this, _1, _2));
    ToggleDirectionServer             = this->create_service<VoidService>("ToggleDirection",             std::bind(&NAVnode::toggleDirection, this, _1, _2));

    AddLocalWaypointServer        = this->create_service<AddLocalWaypoint>       ("AddLocalWaypoint",        std::bind(&NAVnode::addLocalWaypoint, this, _1, _2));
    AddLocalWaypointAtIndexServer = this->create_service<AddLocalWaypointAtIndex>("AddLocalWaypointAtIndex", std::bind(&NAVnode::addLocalWaypointAtIndex, this, _1, _2));
    AddLocalObstacleServer        = this->create_service<AddLocalObstacle>       ("AddLocalObstacle",        std::bind(&NAVnode::addLocalObstacle, this, _1, _2));

    AddGeodeticWaypointServer        = this->create_service<AddGeodeticWaypoint>       ("AddGeodeticWaypoint",        std::bind(&NAVnode::addGeodeticWaypoint, this, _1, _2));
    AddGeodeticWaypointAtIndexServer = this->create_service<AddGeodeticWaypointAtIndex>("AddGeodeticWaypointAtIndex", std::bind(&NAVnode::addGeodeticWaypointAtIndex, this, _1, _2));
    AddGeodeticObstacleServer        = this->create_service<AddGeodeticObstacle>       ("AddGeodeticObstacle",        std::bind(&NAVnode::addGeodeticObstacle, this, _1, _2));

    AddEarthCentredWaypointServer        = this->create_service<AddEarthCentredWaypoint>       ("AddEarthCentredWaypoint",        std::bind(&NAVnode::addEarthCentredWaypoint, this, _1, _2));
    AddEarthCentredWaypointAtIndexServer = this->create_service<AddEarthCentredWaypointAtIndex>("AddEarthCentredWaypointAtIndex", std::bind(&NAVnode::addEarthCentredWaypointAtIndex, this, _1, _2));
    AddEarthCentredObstacleServer        = this->create_service<AddEarthCentredObstacle>       ("AddEarthCentredObstacle",        std::bind(&NAVnode::addEarthCentredObstacle, this, _1, _2));
        
    //main timer will process gnss data once per second
    timer = this->create_wall_timer(1s, std::bind(&NAVnode::mainTimer, this));

    }

    // timers
void NAVnode::mainTimer(){
    updatePositonData();
    pointsVisited.add(currentPosition);

    if (debug){
        // debug mode will print the current value for all our variables to the ros terminal thatt the navigation node is active in
        
        printf("Current coordinates: %f N %f W \n", currentPosition->getLatitude(), currentPosition->getLatitude());
        printf("Current velocity: %f m/s @ %f degrees \n", currentVelocity.speed, currentVelocity.heading);
        printf("Route Selected: %s Coordinate Preference: %s Circumnavigation style: %s \n", getRouteName().c_str(), getPreferenceName().c_str(), getCircumnavigationStyleName().c_str());
        printf("Number of Waypoints: %d Number of Obstacles: %d Current Maximun number of obstacles: %d \n\n\n", waypointCount, obstacleCount, obstacleLimit);
    }

    lastFiveWaypoints[timeSinceLastVelocityPublishing] = currentPosition;

    if (timeSinceLastVelocityPublishing >= 5){
        updateVelocity();
        VelocityPublisher->publish(currentVelocity);
    }
}

void NAVnode::updatePositonData(){
        
    }

void NAVnode::updateVelocity(){
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
void NAVnode::stopNavigating(const std::shared_ptr<VoidService::Request> request, 
                                std::shared_ptr<VoidService::Response> response){

}

void NAVnode::togglePreference(const std::shared_ptr<VoidService::Request> request, 
                                std::shared_ptr<VoidService::Response> response){
    switch(preference){
        case(local): preference = geodetic;          break;
        case(geodetic): preference = earthCentered;  break;
        default: preference = local;                 break;
    }
}

void NAVnode::toggleRoute(const std::shared_ptr<VoidService::Request> request, 
                            std::shared_ptr<VoidService::Response> response){
    switch(routeSelected){
        case(Route1): routeSelected = Route2; break;
        case(Route2): routeSelected = Route3; break;
        case(Route3): routeSelected = Route4; break;
        case(Route4): routeSelected = Route5; break;
        default: routeSelected = Route1;      break;
    }
}

void NAVnode::toggleDebug(const std::shared_ptr<VoidService::Request> request, 
                            std::shared_ptr<VoidService::Response> response){
    debug = !debug;
}

void NAVnode::toggleDirection(const std::shared_ptr<VoidService::Request> request, 
                                std::shared_ptr<VoidService::Response> response){
    returnToBase = !returnToBase;
}

void NAVnode::toggleCircumnavigationStyle(const std::shared_ptr<VoidService::Request> request, 
                                            std::shared_ptr<VoidService::Response> response){
    switch(circumnavigationStyle){
        case(reroute):       circumnavigationStyle = trackCrawling;                  break;
        case(trackCrawling): circumnavigationStyle = automatic_Circumnavigation_Off; break;
        default:             circumnavigationStyle = reroute;                        break;
    }
}

void NAVnode::selectRoute(const std::shared_ptr<SelectRoute::Request> request, 
                            std::shared_ptr<SelectRoute::Response> response){
    switch(request->route){
        case Route1: routeSelected = Route1; break;
        case Route2: routeSelected = Route2; break;
        case Route3: routeSelected = Route3; break;
        case Route4: routeSelected = Route4; break;
        case Route5: routeSelected = Route5; break;
        default: cout << "Error: invalid Route selection" << endl;
    }
}

void NAVnode::resetHome(const std::shared_ptr<ResetHome::Request> request, 
                        std::shared_ptr<ResetHome::Response> response){
    if (Waypoint::checkPassword(request->password))
        response->success = true;
    else 
        response->success = false;
}

void NAVnode::clearRoute(const std::shared_ptr<VoidService::Request> request, 
                            std::shared_ptr<VoidService::Response> response){
    routes[routeSelected].clear();
}

void NAVnode::addLocalWaypoint(const std::shared_ptr<AddLocalWaypoint::Request> request, 
                                std::shared_ptr<AddLocalWaypoint::Response> response){
    routes[routeSelected].add(new Waypoint(request->x, request->y));
}

void NAVnode::addLocalWaypointAtIndex(const std::shared_ptr<AddLocalWaypointAtIndex::Request> request, 
                                        std::shared_ptr<AddLocalWaypointAtIndex::Response> response){
    routes[routeSelected].add(new Waypoint(request->x, request->y), request->index);
}

void NAVnode::addLocalObstacle(const std::shared_ptr<AddLocalObstacle::Request> request, 
                                std::shared_ptr<AddLocalObstacle::Response> response){
    // check if the array of obstacles is full
    if (obstacleCount == obstacleLimit){
        // increase make a larger array to store obstacles
        obstacleLimit += 10;
        Obstacle* biggerArray[obstacleLimit];

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

void NAVnode::addGeodeticWaypoint(const std::shared_ptr<AddGeodeticWaypoint::Request> request, 
                                    std::shared_ptr<AddGeodeticWaypoint::Response> response){
    routes[routeSelected].add(new Waypoint(request->longitude, request->latitude, request->altitude, true));
}

void NAVnode::addGeodeticWaypointAtIndex(const std::shared_ptr<AddGeodeticWaypointAtIndex::Request> request, 
                                            std::shared_ptr<AddGeodeticWaypointAtIndex::Response> response){
    routes[routeSelected].add(new Waypoint(request->longitude, request->latitude, request->altitude, true), request->index);
}

void NAVnode::addGeodeticObstacle(const std::shared_ptr<AddGeodeticObstacle::Request> 
                                    request, std::shared_ptr<AddGeodeticObstacle::Response> response){
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

void NAVnode::addEarthCentredWaypoint(const std::shared_ptr<AddEarthCentredWaypoint::Request> request, 
                                        std::shared_ptr<AddEarthCentredWaypoint::Response> response){
    routes[routeSelected].add(new Waypoint(request->x, request->y, request->z, false));
}

void NAVnode::addEarthCentredWaypointAtIndex(const std::shared_ptr<AddEarthCentredWaypointAtIndex::Request> request, 
                                                std::shared_ptr<AddEarthCentredWaypointAtIndex::Response> response){
    routes[routeSelected].add(new Waypoint(request->x, request->y, request->z, false), request->index);
}

void NAVnode::addEarthCentredObstacle(const std::shared_ptr<AddEarthCentredObstacle::Request> request, 
                                        std::shared_ptr<AddEarthCentredObstacle::Response> response){
    // check if the array of obstacles is full
    if (obstacleCount == obstacleLimit){
        // increase make a larger array to store obstacles
        obstacleLimit += 10;
        Obstacle** biggerArray = new Obstacle*[obstacleLimit];

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