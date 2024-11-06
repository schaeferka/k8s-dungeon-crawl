#ifndef PORTAL_UTILITIES_H
#define PORTAL_UTILITIES_H

#include "Rogue.h"  // Include the creature definition

// Function to send monster details to the portal server
void send_monster_to_portal(const creature *monst);

// Function to send monster death notification to the portal server
void send_monster_death_to_portal(const creature *monst);

#endif
