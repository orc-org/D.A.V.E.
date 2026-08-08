#define enum for preference
uint8 LOCAL = 0
uint8 GEODETIC = 1
uint8 EARTH_CENTRED = 2

#define enum for routes
uint8 ROUTE1 = 0
uint8 ROUTE2 = 1
uint8 ROUTE3 = 2
uint8 ROUTE4 = 3
uint8 ROUTE5 = 4

# current position of the rover
float64 local_x
float64 local_y
float64 rover_x
float64 rover_y
float64 rover_z

# position of next Waypoint
float64 waypoint_local_x
float64 waypoint_local_y
float64 next_waypoint_x
float64 next_waypoint_y
float64 next_waypoint_z

# information for the bearing indicator
float64 heading  # in radians
float64 bearing2waypoint # in Radians
float64 range2waypoint

# misc
uint8 preference
uint8 route2follow
uint8 route2edit
bool traveling_forwards
float64 speed

# the X, Y and Z variables for the rover and the next waypoint will be reused for all 3 coordinate systems
# check the preference variable W.R.T the preference enum to see what format they are in
# (z = 0 for local coordinates)
