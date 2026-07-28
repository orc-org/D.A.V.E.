# NAVnode Code Review 

correcting C++ errors and identifying areas that still need confirmation from Isaac.

## Major errors corrected

The review focused on the faults most likely to cause crashes, memory corruption, or incorrect navigation:
### Main issues (fixed)

- **Null-pointer access in track-crawling logic**  
  Some calculations assumed that every waypoint had both a previous and next node.

- **Incorrect array deallocation**  
  Several arrays created with `new[]` were released using `delete`. These were changed to `delete[]` to avoid undefined behaviour and possible heap corruption.  [Link to stackoverflow](https://stackoverflow.com/questions/2425728/what-is-the-difference-between-delete-and-delete)

- **Invalid obstacle indices**  
  Collision-search functions can return `-1`. The code now validates these results before using them to index the obstacle array. -1 is invalid in Cpp.

- **Repeated position allocation and history aliasing**  
  The placeholder position update repeatedly allocated new `Waypoint` objects, creating a memory leak.The repeated allocation was removed and the history now stores separate snapshots.

- **Incorrect velocity and heading calculations**  
  Velocity was calculated by averaging multiple displacements and dividing again. It now uses the oldest and newest samples over the elapsed time. Heading now uses `atan2()` instead of `asin()`, which handles all movement directions correctly.

- **Coordinate-conversion errors**  
  One local-coordinate equation used `home_latitude` where `home_longitude` was required. Geodetic coordinates are also converted from degrees to radians before trigonometric operations.



## GNSS position update (patrick already made one)
A separate Python GPS bridge already:

- reads the serial byte stream;
- buffers partial NMEA messages;
- validates NMEA checksums;
- parses GGA and RMC messages;
- converts NMEA coordinates to decimal degrees;
- publishes the GNSS fix through ROS 2.

The C++ `updatePositionData()` function can remain a placeholder until the navigation node is connected to the published GPS topic.

## Items requiring clarification

1. **Home-coordinate initialisation**  
   Confirm where `home_latitude`, `home_longitude` units.

2. **Unfinished service callbacks**  
   - `stopNavigating()`
   - `removeLocalObstacle()`
   - `removeGeodeticObstacle()`
   - `removeEarthCentredObstacle()`
   - the home-reset behaviour in `resetHome()`

4. **Memory ownership**  
   Confirm which classes own and delete:
   - route waypoints;
   - `pointsVisited` waypoints;
   - obstacle objects;
   - `currentPosition`;
   - the five stored position samples;
   - the serial-port object.

## Files

- `NAVnode.cpp` contains the corrected navigation-node implementation.
- `Waypoint.cpp` contains the coordinate-conversion corrections.
