#ifndef PORTAL_MONSTER_H
#define PORTAL_MONSTER_H

#include "Rogue.h"  // Include the creature and levelData definitions
#include <stdbool.h>  // For boolean type

// Define constants
#define MAX_MONSTERS 1024            // Adjust based on expected max number of monsters
#define MONSTER_JSON_SIZE 8192       // Increased buffer size for JSON payloads
#define CHECK_FLAG(flags, bit) ((flags) & (1UL << (bit)))

// Struct to store monster data for comparison
typedef struct {
    char name[50];
    int hp;
    int maxHP;
    int level;
    int x, y;
    int attackSpeed;
    int movementSpeed;
    int accuracy;
    int defense;
    int damageMin;
    int damageMax;
    bool isDead;
    long turnsBetweenRegen;
    bool is_initialized; // Flag to check if the entry has been initialized
} MonsterCacheEntry;

// Declare the monster cache array
extern MonsterCacheEntry monsterCache[MAX_MONSTERS];  // Array to cache monster data

// Function declarations
void send_monster_data_to_portal(const char *endpoint, const char *data); // Send data to portal
void update_monsters();  // Function to send updated monster metrics
bool remove_monster(creatureList *list, creature *remove);  // Remove a monster from a list
void send_monster_death(creature *monst);  // Notify the portal of a monster's death
void reset_monster_metrics();  // Reset all monster-related metrics
bool is_monster_data_changed(const creature *monst, int levelIndex);  // Check if monster data has changed
void monster_death_notification(creature *monst);  // Notify the portal of a monster's death

// External variables
extern levelData *levels;  // The global levels array, defined elsewhere

#endif // PORTAL_MONSTER_H
