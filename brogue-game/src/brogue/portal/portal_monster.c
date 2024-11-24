#include "portal_monster.h"
#include "GlobalsBase.h"
#include "portal_urls.h" 
#include "portal.h"       

/** 
 * @brief Cache for storing monster data for comparison and change detection.
 */
extern MonsterCacheEntry monsterCache[MAX_MONSTERS] = {0};  // Initialize with zeroed entries

/**
 * @brief Initializes or resets all monster data and the monster cache.
 *
 * This function is called at the start of the game to ensure that the monster data
 * and cache are in a clean state before new monsters are generated or updated.
 */
void initialize_monsters(void) {
    // Reset the monster cache to clear any previously stored data
    reset_monster_cache();

    // Additional monster initialization logic (if needed)
    // You might want to reset any global monster-related variables or flags here.

    // Example: Reset monster data in the game world if needed
    // for (int i = 0; i < MAX_MONSTERS; i++) {
    //     reset_monster(&monsters[i]);  // Assuming a function to reset individual monsters
    // }

    printf("Monsters initialized and cache cleared.\n");
}

void reset_monster_cache(void) {
    // Clear out the monster cache
    for (int i = 0; i < MAX_MONSTERS; i++) {
        memset(&monsterCache[i], 0, sizeof(MonsterCacheEntry));
        monsterCache[i].is_initialized = false;  // Set all entries as uninitialized
    }
}

void monster_cleanup(void) {
    // Free any dynamically allocated memory or perform cleanup tasks here
    // This function is called when the game is exiting or restarting
}

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
bool has_monster_data_changed(const creature *monst, int levelIndex) {
    if (!rogue.gameHasEnded) {
        if (monst->id < 0 || monst->id >= MAX_MONSTERS) {
            fprintf(stderr, "Warning: Monster ID %d out of bounds\n", monst->id);
            return true;  // Default to true to ensure it's sent if out of bounds
        }

        MonsterCacheEntry *cacheEntry = &monsterCache[monst->id];

        // Check if the cache has been initialized for this monster
        if (!cacheEntry->is_initialized) {
            cacheEntry->is_initialized = true;
            return true;  // Treat uninitialized entry as "changed"
        }

        // Compare all fields to detect changes
        if (strcmp(cacheEntry->name, monst->portalName) != 0 ||
            cacheEntry->hp != monst->currentHP ||
            cacheEntry->maxHP != monst->info.maxHP ||
            cacheEntry->level != levelIndex ||
            cacheEntry->x != monst->loc.x ||
            cacheEntry->y != monst->loc.y ||
            cacheEntry->attackSpeed != monst->attackSpeed ||
            cacheEntry->movementSpeed != monst->movementSpeed ||
            cacheEntry->accuracy != monst->info.accuracy ||
            cacheEntry->defense != monst->info.defense ||
            cacheEntry->damageMin != monst->info.damage.lowerBound ||
            cacheEntry->damageMax != monst->info.damage.upperBound ||
            cacheEntry->isDead != monst->isDead ||
            cacheEntry->turnsBetweenRegen != monst->info.turnsBetweenRegen) {

            // Update cache with new data
            strncpy(cacheEntry->name, monst->portalName, sizeof(cacheEntry->name) - 1);
            cacheEntry->name[sizeof(cacheEntry->name) - 1] = '\0';  // Ensure null termination
            cacheEntry->hp = monst->currentHP;
            cacheEntry->maxHP = monst->info.maxHP;
            cacheEntry->level = levelIndex;
            cacheEntry->x = monst->loc.x;
            cacheEntry->y = monst->loc.y;
            cacheEntry->attackSpeed = monst->attackSpeed;
            cacheEntry->movementSpeed = monst->movementSpeed;
            cacheEntry->accuracy = monst->info.accuracy;
            cacheEntry->defense = monst->info.defense;
            cacheEntry->damageMin = monst->info.damage.lowerBound;
            cacheEntry->damageMax = monst->info.damage.upperBound;
            cacheEntry->isDead = monst->isDead;
            cacheEntry->turnsBetweenRegen = monst->info.turnsBetweenRegen;

            return true;  // Data has changed
        }

        return false;  // No changes detected
    }
}

/**
 * @brief Updates the monster data and sends it to the portal.
 *
 * This function iterates over all monsters in the game, checks if their data
 * has changed, generates the appropriate JSON string, and sends the data to
 * the portal if necessary.
 */
void update_monsters() {
    if (!rogue.gameHasEnded) {
        char monster_json[MONSTER_JSON_SIZE];
        size_t offset = 0;
        bool has_changes = false;

        // Start the array of monsters in JSON format
        offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, "[");

        bool first_entry = true;
        for (int levelIndex = 0; levelIndex <= rogue.deepestLevel; levelIndex++) {
            levelData *level = &levels[levelIndex];

            if (level->visited) {
                creatureIterator iter = iterateCreatures(&level->monsters);

                while (hasNextCreature(iter)) {
                    creature *monst = nextCreature(&iter);

                    if (CHECK_FLAG(monst->bookkeepingFlags, MB_HAS_DIED)) {
                        monst->isDead = true;
                        // Generate the monster death JSON and send it to portal
                        send_monster_death_to_portal(monst);
                        continue;
                    }

                    if (!has_monster_data_changed(monst, levelIndex)) {
                        continue;
                    }

                    if (offset >= sizeof(monster_json) - 200) {
                        fprintf(stderr, "Warning: Monster JSON buffer is almost full\n");
                        break;
                    }

                    if (!first_entry) {
                        offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, ", ");
                    }
                    first_entry = false;
                    has_changes = true;

                    // Generate the update JSON for this monster and append it
                    char update_data[512];  // Buffer for the update JSON string
                    generate_monster_json(monst, update_data, sizeof(update_data));

                    // Append the generated JSON to the monster_json array
                    offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, "%s", update_data);
                }
            }
        }

        offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, "]");

        if (has_changes) {
            // Send the collected monster data to the portal
            send_monsters_to_portal(monster_json);
        } else {
            printf("No changes detected in monster data; skipping portal update.\n");
        }
    }
}

/**
 * @brief Sends the monster death data to the portal.
 *
 * This function generates the JSON string representing a monster's death and
 * sends it to the portal using the generic `send_data_to_portal` function.
 *
 * @param monst The monster that has died.
 */
void send_monster_death_to_portal(creature *monst) {
    char monster_data[512];
    generate_monster_json(monst, monster_data, sizeof(monster_data));

    // Send the death data using the generic function from portal.c
    send_data_to_portal(get_monster_death_url(), monster_data);
}

/**
 * @brief Sends the monster data to the portal.
 *
 * This function sends a batch of monster data (in JSON format) to the portal.
 * 
 * @param data The JSON string containing the monster data to be sent.
 */
void send_monsters_to_portal(const char *data) {
    // Send the data using the generic function from portal.c
    send_data_to_portal(get_monster_update_url(), data);
}

/**
 * @brief Sends a reset command to the portal.
 *
 * This function generates and sends a request to reset the monster data on the portal.
 */
void send_monster_reset_to_portal() {
    // Send the reset command using the generic function from portal.c
    send_data_to_portal(get_monster_reset_url(), NULL);
}

/**
 * @brief Generates the JSON string for a monster event.
 *
 * This function takes in the relevant monster data and formats it into a JSON string
 * that represents the monster's data.
 *
 * @param monst The monster data structure containing the information.
 * @param monster_data A buffer to store the resulting JSON string.
 * @param size The size of the buffer.
 */
void generate_monster_json(const creature *monst, char *monster_data, size_t size) {
    snprintf(monster_data, size,
        "{\"id\": %d, \"name\": \"%s\", \"type\": \"%s\", \"hp\": %d, \"maxHP\": %d, \"depth\": %d, "
        "\"position\": {\"x\": %d, \"y\": %d}, \"attackSpeed\": %d, \"movementSpeed\": %d, \"accuracy\": %d, \"defense\": %d, "
        "\"damageMin\": %d, \"damageMax\": %d, \"turnsBetweenRegen\": %ld, \"isDead\": %d}",
        monst->id, monst->portalName, monst->info.monsterName, monst->currentHP, monst->info.maxHP,
        monst->spawnDepth, monst->loc.x, monst->loc.y, monst->attackSpeed, monst->movementSpeed,
        monst->info.accuracy, monst->info.defense, monst->info.damage.lowerBound,
        monst->info.damage.upperBound, monst->info.turnsBetweenRegen,
        monst->isDead ? 1 : 0);
}