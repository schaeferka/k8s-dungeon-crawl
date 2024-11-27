#include "portal_monsters.h"
#include "GlobalsBase.h"
#include "portal_urls.h" 
#include "portal.h"       

// Debug macro for logging
#define DEBUG_LOG(fmt, ...) \
    printf("[DEBUG] [%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/** 
 * @brief Cache for storing monster data for comparison and change detection.
 */
MonsterCacheEntry monsterCache[MAX_MONSTERS] = {0};

/**
 * @brief Initializes or resets all monster data and the monster cache.
 *
 * This function is called at the start of the game to ensure that the monster data
 * and cache are in a clean state before new monsters are generated or updated.
 */
void initialize_monsters(void) {
    reset_monster_cache();
    send_monster_reset_to_portal(); 

    // TODO: Additional monster initialization logic
    
    printf("Monsters initialized and cache cleared.\n");
}

void reset_monster_cache(void) {
    for (int i = 0; i < MAX_MONSTERS; i++) {
        memset(&monsterCache[i], 0, sizeof(MonsterCacheEntry));
        monsterCache[i].is_initialized = false;  
    }
}

void monster_cleanup(void) {
    // Free any dynamically allocated memory or perform cleanup tasks here
    // This function is called when the game is exiting or restarting
    printf("Monster cleanup complete.\n");
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

    return false;  // Game has ended; no need to update
}

/**
 * @brief Updates the monster data and sends it to the portal.
 *
 * This function iterates over all monsters in the game, checks if their data
 * has changed, generates the appropriate JSON string, and sends the data to
 * the portal if necessary.
 */
void update_monsters(void) {
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
                        char death_json[512];  // Buffer to hold the death JSON string
                        snprintf(death_json, sizeof(death_json), "{\"id\": \"%d\"}", monst->id);  // Create the JSON for the monster ID
                        send_monster_death_to_portal(death_json);
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
            send_monsters_to_portal(monster_json);
        } else {
            printf("No changes detected in monster data; skipping portal update.\n");
        }
    }
}

/**
 * @brief Sends the monster death event to the portal.
 *
 * This function generates the JSON string for a monster death event and sends
 * it to the portal.
 *
 * @param monst The monster that has died.
 */
extern void report_monster_death(creature *monst) {
    monst->isDead = true;
    
    update_monsters();

    // Prepare the JSON data to be sent
    char death_data[512];
    snprintf(death_data, sizeof(death_data),
        "{\"id\": \"%d\"}", monst->id);

    send_monster_death_to_portal(death_data);
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
