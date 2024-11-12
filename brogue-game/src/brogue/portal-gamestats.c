// portal-gamestate.c

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal-gamestats.h"
#include "platform.h"
#include "MainMenu.h"

#define GAMESTATS_PORTAL_URL "http://portal-service.portal:5000/gamestats"

// Function to send game stat metrics to the portal
void extractGameStatsJSON(const gameStats *stats, char *buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size,
        "{"
        "\"games\": %d, "
        "\"escaped\": %d, "
        "\"mastered\": %d, "
        "\"won\": %d, "
        "\"winRate\": %.2f, "
        "\"deepestLevel\": %d, "
        "\"cumulativeLevels\": %d, "
        "\"highestScore\": %d, "
        "\"cumulativeScore\": %lu, "
        "\"mostGold\": %d, "
        "\"cumulativeGold\": %lu, "
        "\"mostLumenstones\": %d, "
        "\"cumulativeLumenstones\": %d, "
        "\"fewestTurnsWin\": %d, "
        "\"cumulativeTurns\": %lu, "
        "\"longestWinStreak\": %d, "
        "\"longestMasteryStreak\": %d, "
        "\"currentWinStreak\": %d, "
        "\"currentMasteryStreak\": %d"
        "}",
        stats->games,
        stats->escaped,
        stats->mastered,
        stats->won,
        stats->winRate,
        stats->deepestLevel,
        stats->cumulativeLevels,
        stats->highestScore,
        stats->cumulativeScore,
        stats->mostGold,
        stats->cumulativeGold,
        stats->mostLumenstones,
        stats->cumulativeLumenstones,
        stats->fewestTurnsWin,
        stats->cumulativeTurns,
        stats->longestWinStreak,
        stats->longestMasteryStreak,
        stats->currentWinStreak,
        stats->currentMasteryStreak
    );
}

// Function to send game stat metrics to the portal
void update_gamestats(void) {
    gameStats gamestats = {0};  // Initialize gamestats with zeros

    rogueRun *runHistory = loadRunHistory();
    rogueRun *currentRun = runHistory;

    // Aggregate data from each run into gamestats
    while (currentRun != NULL) {
        if (currentRun->seed != 0) {  // Only process valid runs
            addRuntoGameStats(currentRun, &gamestats);
        }
        currentRun = currentRun->nextRun;
    }

    // Free the run history after aggregating data
    currentRun = runHistory;
    rogueRun *nextRun;
    while (currentRun != NULL) {
        nextRun = currentRun->nextRun;
        free(currentRun);
        currentRun = nextRun;
    }

    // Create JSON buffer
    char jsonBuffer[1024];
    extractGameStatsJSON(&gamestats, jsonBuffer, sizeof(jsonBuffer));

    // Send the data to the portal
    send_gamestats_metrics_to_portal(jsonBuffer);
}

// Function to send game stats metrics to the portal
void send_gamestats_metrics_to_portal(const char *post_data) {
    CURL *curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_URL, GAMESTATS_PORTAL_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send game stats metrics: %s\n", curl_easy_strerror(res));
        } else {
            printf("Successfully sent game stats metrics to portal.\n");
            printf("Game stats data: %s\n", post_data);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "CURL initialization failed.\n");
    }
}