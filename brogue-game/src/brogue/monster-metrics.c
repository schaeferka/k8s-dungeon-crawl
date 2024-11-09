#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include "monster-metrics.h" 
#include "metrics-sender.h"
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include <string.h>
#include <stdbool.h>

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
        cacheEntry->turnsBetweenRegen != monst->info.turnsBetweenRegen) {

        // Update cache with new data
        strncpy(cacheEntry->name, monst->portalName, sizeof(cacheEntry->name) - 1);
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
        cacheEntry->turnsBetweenRegen = monst->info.turnsBetweenRegen;

        return true;  // Data has changed
    }

    return false;  // No changes detected
}

void update_monster_metrics() {
    char monster_json[MONSTER_JSON_SIZE];
    size_t offset = 0;

    offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, "[");

    bool first_entry = true;  // To manage comma placement between entries

    for (int levelIndex = 0; levelIndex <= rogue.deepestLevel; levelIndex++) {
        levelData *level = &levels[levelIndex];

        if (level->visited) {
            creatureIterator iter = iterateCreatures(&level->monsters);

            while (hasNextCreature(iter)) {
                creature *monst = nextCreature(&iter);

                // Only send monsters whose data has changed
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

                // Add monster data to JSON, including the `id` field
                offset += snprintf(monster_json + offset, sizeof(monster_json) - offset,
                                   "{ \"id\": %d, \"name\": \"%s\", \"type\": \"%s\", \"hp\": %d, \"maxHP\": %d, \"level\": %d, "
                                   "\"position\": {\"x\": %d, \"y\": %d}, "
                                   "\"attackSpeed\": %d, \"movementSpeed\": %d, \"accuracy\": %d, \"defense\": %d, "
                                   "\"damageMin\": %d, \"damageMax\": %d, \"turnsBetweenRegen\": %ld }",
                                   monst->id, monst->portalName, monst->info.monsterName, monst->currentHP, monst->info.maxHP, levelIndex,
                                   monst->loc.x, monst->loc.y, monst->attackSpeed, monst->movementSpeed, monst->info.accuracy,
                                   monst->info.defense, monst->info.damage.lowerBound, monst->info.damage.upperBound,
                                   monst->info.turnsBetweenRegen);
            }
        }
    }

    offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, "]");

    // Log the full JSON data being sent
    printf("Sending monster data to portal: %s\n", monster_json);
    send_monster_data_to_portal("/monsters", monster_json);
}

boolean remove_monster(creatureList *list, creature *remove) {
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

void send_monster_death(creature *monst) {
    char death_data[256];
    snprintf(death_data, sizeof(death_data), 
             "{\"name\": \"%s\", \"id\": \"%d\", \"status\": \"dead\"}", monst->portalName);
    
    // Log the death data being sent
    printf("Sending monster death data to portal: %s\n", death_data);
    send_monster_data_to_portal("/monster/death", death_data);
}

void reset_monster_metrics() {
    // K8S: Debug log
    printf("Entered reset_monster_metrics\n");
    //send_monster_data_to_portal("/monsters/reset", "{}");
    //printf("Monster lists and metrics have been reset for a new game.\n");
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
            // Log successful transmission
            //printf("Successfully sent data to %s: %s\n", endpoint, data);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}
