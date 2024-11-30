#ifndef PORTAL_MONSTERS_H
#define PORTAL_MONSTERS_H

#include "Rogue.h"  // Include the creature and levelData definitions
#include <stdbool.h>

#define MAX_MONSTERS 1024
#define MONSTER_JSON_SIZE 8192
#define CHECK_FLAG(flags, bit) (((bit) < (sizeof(flags) * 8)) ? ((flags) & (1UL << (bit))) : 0)

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
    bool is_initialized;
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
 * @brief Sends the monster death event to the portal.
 *
 * This function generates the JSON string for a monster death event and sends
 * it to the portal.
 *
 * @param monst The monster that has died.
 */
extern void report_monster_death(creature *monst);

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
extern void reset_monsters(void);

extern levelData *levels;

#endif // PORTAL_MONSTERS_H
