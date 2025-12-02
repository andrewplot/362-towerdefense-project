#include "rfid_bridge.hh"
#include "rfid.hh"   // from lib/rfid, brings in TowerType (RFID-side)

// NOTE: You must adapt these cases to match your tower.hh enum.
// I'm assuming tower.hh defines something like: dart, tack, bomb, sniper, blank.

int rfid_get_tower_code() {
    TowerType t = sample_rfid();  // from rfid.cpp

    switch (t) {
        case dart:   return 1;  // map to your machine-gun tower
        case tack:   return 2;  // map to cannon tower
        // add more mappings if your tower.hh has other types
        case blank:
        default:
            return 0;          // no tower selected
    }
}
