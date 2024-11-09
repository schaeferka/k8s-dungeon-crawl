#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include "monster-metrics.h" 
#include "metrics-sender.h"
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"

// External declaration of levels
extern levelData *levels;

#define PORTAL_URL "http://portal-service.portal:5000/monster"
#define PORTAL_DEATH_URL "http://portal-service.portal:5000/monster/death"

void update_monster_metrics() {
    char monster_json[4096];
    size_t offset = 0;

    offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, "[");

    for (int levelIndex = 0; levelIndex <= rogue.deepestLevel; levelIndex++) {
        levelData *level = &levels[levelIndex];

        if (level->visited) {
            creatureIterator iter = iterateCreatures(&level->monsters);

            while (hasNextCreature(iter)) {
                creature *monst = nextCreature(&iter);

                if (offset > 1) {  // Add a comma if this is not the first entry
                    offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, ", ");
                }

                offset += snprintf(monster_json + offset, sizeof(monster_json) - offset,
                                   "{ \"name\": \"%s\", \"type\": \"%s\", \"hp\": %d, \"maxHP\": %d, \"level\": %d, "
                                   "\"position\": {\"x\": %d, \"y\": %d}, "
                                   "\"attackSpeed\": %d, \"movementSpeed\": %d, \"accuracy\": %d, \"defense\": %d, "
                                   "\"damageMin\": %d, \"damageMax\": %d, \"turnsBetweenRegen\": %ld }",
                                   monst->portalName, monst->info.monsterName, monst->currentHP, monst->info.maxHP, monst->spawnDepth,
                                   monst->loc.x, monst->loc.y, monst->attackSpeed, monst->movementSpeed, monst->info.accuracy,
                                   monst->info.defense, monst->info.damage.lowerBound, monst->info.damage.upperBound,
                                   monst->info.turnsBetweenRegen);

                if (offset >= sizeof(monster_json) - 100) {
                    fprintf(stderr, "Warning: Monster JSON buffer is almost full\n");
                    break;
                }
            }
        }
    }

    offset += snprintf(monster_json + offset, sizeof(monster_json) - offset, "]");
    
    // Log the full JSON data being sent
    //printf("Sending monster data to portal: %s\n", monster_json);
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
             "{\"name\": \"%s\", \"status\": \"dead\"}", monst->portalName);
    
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
