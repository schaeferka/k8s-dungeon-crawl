#ifndef PORTAL_MONSTERS_H
#define PORTAL_MONSTERS_H

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

/** 
 * @brief Cache for storing monster data for comparison and change detection.
 */
extern MonsterCacheEntry monsterCache[MAX_MONSTERS];

/**
 * @brief Checks if the monster's data has changed since the last update.
 *
 * This function compares the current monster's data with its cached data to
 * determine if an update is needed.
 *
 * @param monst The monster whose data is being checked.
 * @param levelIndex The level index where the monster is located.
 * @return `true` if the data has changed, `false` otherwise.
 */
bool has_monster_data_changed(const creature *monst, int levelIndex);

/**
 * @brief Updates the monster data and sends it to the portal.
 *
 * This function iterates over all monsters in the game, checks if their data
 * has changed, generates the appropriate JSON string, and sends the data to
 * the portal if necessary.
 */
extern void update_monsters(void);

/**
 * @brief Sends the monster death data to the portal.
 *
 * This function generates the JSON string representing a monster's death and
 * sends it to the portal using the generic `send_data_to_portal` function.
 *
 * @param monst The monster that has died.
 */
void send_monster_death_to_portal(creature *monst);

/**
 * @brief Sends the monster data to the portal.
 *
 * This function sends a batch of monster data (in JSON format) to the portal.
 * 
 * @param data The JSON string containing the monster data to be sent.
 */
void send_monsters_to_portal(const char *data);

/**
 * @brief Sends a reset command to the portal.
 *
 * This function generates and sends a request to reset the monster data on the portal.
 */
extern void send_monster_reset_to_portal(void);

/**
 * @brief Generates the JSON string for a monster event.
 *
 * This function takes in the relevant monster data and formats it into a JSON string
 * that represents the monster's data (used for both updates and death events).
 *
 * @param monst The monster data structure containing the information.
 * @param monster_data A buffer to store the resulting JSON string.
 * @param size The size of the buffer.
 */
void generate_monster_json(const creature *monst, char *monster_data, size_t size);


extern void initialize_monsters(void);
extern void reset_monster_cache(void);
extern void monster_cleanup(void);

// External variables
extern levelData *levels;  // The global levels array, defined elsewhere

#endif // PORTAL_MONSTERS_H
