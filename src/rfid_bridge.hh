// rfid_bridge.hh
#ifndef RFID_BRIDGE_HH
#define RFID_BRIDGE_HH

// Return a small code representing the scanned tag.
// 0 = no valid tag / nothing
// 1,2,3,4 = tower choices
int rfid_get_tower_code();

#endif // RFID_BRIDGE_HH
