// rfid_bridge.cpp – only hardware TowerType lives here
#include "rfid_bridge.hh"
#include "rfid.hh"      // includes tower.hh with hardware TowerType

int rfid_get_tower_code() {
    // TowerType here is the one from tower.hh (hardware side)
    TowerType hw_type = sample_rfid();

    switch (hw_type) {
        case MACHINE_GUN:  return 1;
        case CANNON:       return 2;
        case SNIPER:       return 3;
        case RADAR:        return 4;
        default:           return 0;   // nothing / invalid tag
    }
}
