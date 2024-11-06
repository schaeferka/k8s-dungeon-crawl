#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include "portal-utilities.h" // Define this header for function prototypes and any shared constants

#define PORTAL_URL "http://portal-service.portal:5000/monster"
#define PORTAL_DEATH_URL "http://portal-service.portal:5000/monster/death"

// Function to send monster details to the portal server
void send_monster_to_portal(const creature *monst) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        // Prepare JSON data for the POST request
        char post_data[512];
        snprintf(post_data, sizeof(post_data),
                 "{\"portalName\": \"%s\", \"monsterName\": \"%s\", \"maxHP\": %d, "
                 "\"defense\": %d, \"accuracy\": %d, \"damageMin\": %d, \"damageMax\": %d, "
                 "\"turnsBetweenRegen\": %ld, \"movementSpeed\": %d, \"attackSpeed\": %d, "
                 "\"depth\": %d, \"currentHP\": %d}",
                 monst->portalName, monst->info.monsterName, monst->info.maxHP,
                 monst->info.defense, monst->info.accuracy, monst->info.damage.lowerBound,
                 monst->info.damage.upperBound, monst->info.turnsBetweenRegen,
                 monst->info.movementSpeed, monst->info.attackSpeed,
                 monst->depth, monst->currentHP);

        curl_easy_setopt(curl, CURLOPT_URL, PORTAL_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send monster details to portal: %s\n", curl_easy_strerror(res));
        } else {
            printf("Monster details sent successfully for %s\n", monst->portalName);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Failed to initialize CURL session for sending monster details.\n");
    }
}

// Function to send monster death notification to the portal server
void send_monster_death_to_portal(const creature *monst) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        // Prepare JSON data for the POST request
        char post_data[512];
        snprintf(post_data, sizeof(post_data),
                 "{\"portalName\": \"%s\", \"monsterName\": \"%s\", \"maxHP\": %d, "
                 "\"defense\": %d, \"accuracy\": %d, \"damageMin\": %d, \"damageMax\": %d, "
                 "\"turnsBetweenRegen\": %ld, \"movementSpeed\": %d, \"attackSpeed\": %d, "
                 "\"depth\": %d, \"currentHP\": %d}",
                 monst->portalName, monst->info.monsterName, monst->info.maxHP,
                 monst->info.defense, monst->info.accuracy, monst->info.damage.lowerBound,
                 monst->info.damage.upperBound, monst->info.turnsBetweenRegen,
                 monst->info.movementSpeed, monst->info.attackSpeed,
                 monst->depth, monst->currentHP);

        curl_easy_setopt(curl, CURLOPT_URL, PORTAL_DEATH_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to notify portal of monster death: %s\n", curl_easy_strerror(res));
        } else {
            printf("Monster death notification sent successfully for %s\n", monst->portalName);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Failed to initialize CURL session for monster death notification.\n");
    }
}
