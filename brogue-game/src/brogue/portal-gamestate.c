// portal-gamestate.c

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal-gamestate.h"

#define GAMESTATE_PORTAL_URL "http://portal-service.portal:5000/gamestate"

// Function to send game state information to the portal
void update_gamestate(void) {
    GameStateMetrics gamestate = {
        .wizard = rogue.wizard,
        .rewardRoomsGenerated = rogue.rewardRoomsGenerated,
        .goldGenerated = rogue.goldGenerated,
        .gameInProgress = rogue.gameInProgress,
        .gameHasEnded = rogue.gameHasEnded,
        .easyMode = rogue.easyMode,
        .seed = rogue.seed,
        .RNG = rogue.RNG,
        .absoluteTurnNumber = rogue.absoluteTurnNumber,
        .milliseconds = rogue.milliseconds,
        .monsterSpawnFuse = rogue.monsterSpawnFuse,
        .turns = rogue.absoluteTurnNumber,
    };
    send_gamestate_metrics_to_portal(&gamestate);
}

// Function to send game state metrics to the portal
void send_gamestate_metrics_to_portal(const GameStateMetrics *gamestate) {
    CURL *curl = curl_easy_init();
    if (curl) {
        char post_data[1024];
        snprintf(post_data, sizeof(post_data),
            "{\"wizard\": %s, \"rewardRoomsGenerated\": %d, \"goldGenerated\": %d, "
            "\"gameInProgress\": %s, \"gameHasEnded\": %s, \"easyMode\": %s, \"seed\": %d, \"RNG\": %d, "
            "\"absoluteTurnNumber\": %d, \"milliseconds\": %d, \"monsterSpawnFuse\": %d, \"xpxpThisTurn\": %d}",
            gamestate->wizard ? "true" : "false", gamestate->rewardRoomsGenerated, gamestate->goldGenerated,
            gamestate->gameInProgress ? "true" : "false", gamestate->gameHasEnded ? "true" : "false",
            gamestate->easyMode ? "true" : "false", gamestate->seed, gamestate->RNG,
            gamestate->absoluteTurnNumber, gamestate->milliseconds, gamestate->monsterSpawnFuse,
            gamestate->turns);

        // Set up CURL options
        curl_easy_setopt(curl, CURLOPT_URL, GAMESTATE_PORTAL_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        // Set headers for JSON content
        struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Send the request and handle errors
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send game state metrics: %s\n", curl_easy_strerror(res));
        } else {
            printf("Successfully sent game state metrics to portal.\n");
        }

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "CURL initialization failed.\n");
    }
}
