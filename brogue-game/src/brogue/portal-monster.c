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

#define PORTAL_URL "http://portal-service.portal:5000/monster"
#define PORTAL_DEATH_URL "http://portal-service.portal:5000/monster/death"

MonsterCacheEntry monsterCache[MAX_MONSTERS] = {0};  // Initialize with zeroed entries

bool is_monster_data_changed(const creature *monst, int levelIndex) {
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

void update_monster_metrics() {
    char monster_json[MONSTER_JSON_SIZE];
    size_t offset = 0;
    bool has_changes = false;  // Flag to track if there are changes to send

    offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, "[");

    bool first_entry = true;  // To manage comma placement between entries

    for (int levelIndex = 0; levelIndex <= rogue.deepestLevel; levelIndex++) {
        levelData *level = &levels[levelIndex];

        if (level->visited) {
            creatureIterator iter = iterateCreatures(&level->monsters);

            while (hasNextCreature(iter)) {
                creature *monst = nextCreature(&iter);

                // Check if the monster has died based on bookkeepingFlags
                if (CHECK_FLAG(monst->bookkeepingFlags, MB_HAS_DIED)) {
                    monst->isDead = true;  // Set the isDead flag
                    monst->currentHP = 0;  // Ensure HP is zero

                    // Notify the portal of the monster's death
                    monster_death_notification(monst);

                    // Skip processing further metrics for this monster
                    continue;
                }

                // Only process monsters whose data has changed
                if (!is_monster_data_changed(monst, levelIndex)) {
                    continue;
                }

                // Buffer space check
                if (offset >= sizeof(monster_json) - 200) {  // Leave extra space as buffer
                    fprintf(stderr, "Warning: Monster JSON buffer is almost full\n");
                    break;
                }

                // Manage comma placement if not the first entry
                if (!first_entry) {
                    offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, ", ");
                }
                first_entry = false;
                has_changes = true;  // Set flag since we have data to send

                // Add monster data to JSON, including the `id` field
                offset += snprintf(monster_json + offset, sizeof(monster_json) - offset,
                   "{ \"id\": %d, \"name\": \"%s\", \"type\": \"%s\", \"hp\": %d, \"maxHP\": %d, \"dead\": %s, \"level\": %d, "
                   "\"position\": {\"x\": %d, \"y\": %d}, "
                   "\"attackSpeed\": %d, \"movementSpeed\": %d, \"accuracy\": %d, \"defense\": %d, "
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

    // Only send the JSON if there are changes
    if (has_changes) {
        printf("Sending monster data to portal: %s\n", monster_json);
        send_monster_data_to_portal("/monsters", monster_json);
    } else {
        printf("No changes detected in monster data; skipping portal update.\n");
    }
}


bool remove_monster(creatureList *list, creature *remove) {
    creatureListNode **node = &list->head;
    while (*node != NULL) {
        if ((*node)->creature == remove) {
            creatureListNode *removeNode = *node;
            *node = removeNode->nextCreature;
            freeCreature(removeNode->creature);
            
            send_monster_death(removeNode->creature);
            
            free(removeNode);
            return true;
        }
        node = &(*node)->nextCreature;
    }
    return false;
}

void monster_death_notification(creature *monst) {
    char death_data[512];  // Increased buffer size for full data
    CURL *curl;
    CURLcode res;

    printf("OH NO! A monster died: %s\n", monst->portalName);  

    // Prepare the JSON payload with all monster's data, including `isDead` status
    snprintf(death_data, sizeof(death_data),
             "{\"id\": %d, \"name\": \"%s\", \"type\": \"%s\", \"hp\": %d, \"maxHP\": %d, \"level\": %d, "
             "\"position\": {\"x\": %d, \"y\": %d}, "
             "\"attackSpeed\": %d, \"movementSpeed\": %d, \"accuracy\": %d, \"defense\": %d, "
             "\"damageMin\": %d, \"damageMax\": %d, \"turnsBetweenRegen\": %ld, \"isDead\": %s}",
             monst->id, monst->portalName, monst->info.monsterName, monst->currentHP, monst->info.maxHP,
             monst->spawnDepth, monst->loc.x, monst->loc.y, monst->attackSpeed, monst->movementSpeed,
             monst->info.accuracy, monst->info.defense, monst->info.damage.lowerBound,
             monst->info.damage.upperBound, monst->info.turnsBetweenRegen,
             monst->isDead ? "true" : "false");

    // Log the data to be sent
    printf("Sending full monster death data to portal: %s\n", death_data);

    // Initialize CURL for sending data
    curl = curl_easy_init();
    if (curl) {
        // Set the URL to the portal's death endpoint
        curl_easy_setopt(curl, CURLOPT_URL, "http://portal-service.portal:5000/monster/death");

        // Set the payload to send
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, death_data);

        // Set HTTP headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the POST request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send full monster death data to portal: %s\n", curl_easy_strerror(res));
        } else {
            // Success message
            printf("Successfully sent full monster death data to portal: %s\n", death_data);
        }

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "CURL initialization failed for sending full monster death data.\n");
    }
}

void send_monster_death(creature *monst) {
    char death_data[256];
    CURL *curl;
    CURLcode res;

    printf("A monster died: %s %d\n", monst->portalName, monst->id);  
    // Prepare the JSON payload with monster's name, id, and status
    snprintf(death_data, sizeof(death_data), 
             "{\"name\": \"%s\", \"id\": \"%d\", \"status\": \"dead\"}", 
             monst->portalName, monst->id);

    // Log the data to be sent
    printf("Sending monster death data to portal: %s\n", death_data);

    // Initialize CURL for sending data
    curl = curl_easy_init();
    if (curl) {
        // Set the URL to the portal's death endpoint
        curl_easy_setopt(curl, CURLOPT_URL, "http://portal-service.portal:5000/monster/death");

        // Set the payload to send
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, death_data);

        // Set HTTP headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the POST request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send monster death data to portal: %s\n", curl_easy_strerror(res));
        } else {
            // Success message
            printf("Successfully sent monster death data to portal: %s\n", death_data);
        }

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "CURL initialization failed for sending monster death data.\n");
    }
}

void send_monster_data_to_portal(const char *endpoint, const char *data) {
    CURL *curl = curl_easy_init();
    if (curl) {
        char url[256];
        snprintf(url, sizeof(url), "http://portal-service.portal:5000%s", endpoint);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send data to portal: %s\n", curl_easy_strerror(res));
        } else {
            printf("Successfully sent data to %s: %s\n", endpoint, data);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "CURL initialization failed for sending data.\n");
    }
}
