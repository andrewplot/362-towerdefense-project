#ifndef RFID_BRIDGE_HH
#define RFID_BRIDGE_HH

// Returns an integer code representing which physical RFID tag was scanned.
//  0 = none / blank
//  1,2,3,... = tower choices you map in rfid_bridge.cpp
int rfid_get_tower_code();

#endif // RFID_BRIDGE_HH
