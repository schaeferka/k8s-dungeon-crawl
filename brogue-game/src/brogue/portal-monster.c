#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "portal-monster.h" 
#include "portal-player.h"
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"

#define MONSTER_PORTAL_URL "http://portal-service.portal:5000/monsters"
#define MONSTER_DEATH_URL "http://portal-service.portal:5000/monster/death"
#define RESET_URL "http://portal-service.portal:5000/monsters/reset"

MonsterCacheEntry monsterCache[MAX_MONSTERS] = {0};  // Initialize with zeroed entries

bool is_monster_data_changed(const creature *monst, int levelIndex) {
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

void update_monsters() {
    if (!rogue.gameHasEnded) {
        char monster_json[MONSTER_JSON_SIZE];
        size_t offset = 0;
        bool has_changes = false;

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
                        //monst->currentHP = 0;
                        send_monster_death_to_portal(monst);
                        continue;
                    }

                    if (!is_monster_data_changed(monst, levelIndex)) {
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

                    offset += snprintf(monster_json + offset, sizeof(monster_json) - offset,
                        "{ \"id\": %d, \"name\": \"%s\", \"type\": \"%s\", \"hp\": %d, \"maxHP\": %d, \"dead\": %s, \"level\": %d, "
                        "\"position\": {\"x\": %d, \"y\": %d}, \"attackSpeed\": %d, \"movementSpeed\": %d, \"accuracy\": %d, \"defense\": %d, "
                        "\"damageMin\": %d, \"damageMax\": %d, \"turnsBetweenRegen\": %ld }",
                        monst->id, monst->portalName, monst->info.monsterName, monst->currentHP, monst->info.maxHP,
                        monst->isDead ? "true" : "false", monst->spawnDepth,
                        monst->loc.x, monst->loc.y, monst->attackSpeed, monst->movementSpeed, monst->info.accuracy,
                        monst->info.defense, monst->info.damage.lowerBound, monst->info.damage.upperBound,
                        monst->info.turnsBetweenRegen);
                }
            }
        }

        offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, "]");

        if (has_changes) {
            send_monsters_to_portal("/monsters", monster_json);
        } else {
            printf("No changes detected in monster data; skipping portal update.\n");
        }
    }
}

void send_monster_death_to_portal(creature *monst) {
    char death_data[512];
    CURL *curl = curl_easy_init();
    if (!rogue.gameHasEnded && curl) {
        snprintf(death_data, sizeof(death_data),
            "{\"id\": %d, \"name\": \"%s\", \"type\": \"%s\", \"hp\": %d, \"maxHP\": %d, \"level\": %d, "
            "\"position\": {\"x\": %d, \"y\": %d}, \"attackSpeed\": %d, \"movementSpeed\": %d, \"accuracy\": %d, \"defense\": %d, "
            "\"damageMin\": %d, \"damageMax\": %d, \"turnsBetweenRegen\": %ld, \"isDead\": %s}",
            monst->id, monst->portalName, monst->info.monsterName, monst->currentHP, monst->info.maxHP,
            monst->spawnDepth, monst->loc.x, monst->loc.y, monst->attackSpeed, monst->movementSpeed,
            monst->info.accuracy, monst->info.defense, monst->info.damage.lowerBound,
            monst->info.damage.upperBound, monst->info.turnsBetweenRegen,
            monst->isDead ? "true" : "false");

        curl_easy_setopt(curl, CURLOPT_URL, MONSTER_DEATH_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, death_data);
        struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send monster death data to portal: %s\n", curl_easy_strerror(res));
        } else {
            printf("Successfully sent monster death data to portal: %s\n", death_data);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

void send_monsters_to_portal(const char *endpoint, const char *data) {
    if (!rogue.gameHasEnded) {
        CURL *curl = curl_easy_init();
        if (curl) {
            char url[256];
            snprintf(url, sizeof(url), "http://portal-service.portal:5000%s", endpoint);
            struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");

            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Failed to send monsters data to portal: %s\n", curl_easy_strerror(res));
            } else {
                printf("Successfully sent data to %s\n", endpoint);
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        } else {
            fprintf(stderr, "CURL initialization failed.\n");
        }
    }
}

void reset_monster_cache() {
    for (int i = 0; i < MAX_MONSTERS; i++) {
        memset(&monsterCache[i], 0, sizeof(monsterCache[i]));
        monsterCache[i].is_initialized = false;
    }
}

void end_of_game_monster_cleanup() {
    reset_monster_cache();
}

void initialize_monsters_new_game() {
    reset_monster_cache();
    send_monster_reset_to_portal();
    update_monsters();
    printf("Monster data reset and initialized for new game\n");
}

void send_monster_reset_to_portal() {
    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, RESET_URL);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

        struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send reset command to portal: %s\n", curl_easy_strerror(res));
        } else {
            printf("Successfully sent reset command to portal\n");
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "CURL initialization failed for reset command.\n");
    }
}
