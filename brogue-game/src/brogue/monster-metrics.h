#ifndef PORTAL_UTILITIES_H
#define PORTAL_UTILITIES_H

#include "Rogue.h"  // Include the creature definition

extern void update_monster_metrics();

boolean remove_monster(creatureList *list, creature *remove);

void send_monster_death(creature *monst);

void reset_monster_metrics();

void send_monster_data_to_portal(const char *endpoint, const char *data);

extern levelData *levels;

#endif
