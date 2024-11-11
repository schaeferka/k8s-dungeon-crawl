// portal-gamestate.c

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal-gamestats.h"

#define GAMESTATS_PORTAL_URL "http://portal-service.portal:5000/gamestats"

// Function to send game stat metrics to the portal
void update_gamestats(void) {
    GameStatsMetrics gamestats = {
        .games = gamestats.games,
        .escaped = gamestats.escaped,
        .mastered = gamestats.mastered,
        .won = gamestats.won,
        .winRate = gamestats.winRate,
        .deepestLevel = gamestats.deepestLevel,
        .cumulativeLevels = gamestats.cumulativeLevels,
        .highestScore = gamestats.highestScore,
        .cumulativeScore = gamestats.cumulativeScore,
        .mostGold = gamestats.mostGold,
        .cumulativeGold = gamestats.cumulativeGold,
        .mostLumenstones = gamestats.mostLumenstones,
        .cumulativeLumenstones = gamestats.cumulativeLumenstones,
        .fewestTurnsWin = gamestats.fewestTurnsWin,
        .cumulativeTurns = gamestats.cumulativeTurns,
        .longestWinStreak = gamestats.longestWinStreak,
        .longestMasteryStreak = gamestats.longestMasteryStreak,
        .currentWinStreak = gamestats.currentWinStreak,
        .currentMasteryStreak = gamestats.currentMasteryStreak,
    };
    send_gamestats_metrics_to_portal(&gamestats);
}

// Function to send game stats metrics to the portal
void send_gamestats_metrics_to_portal(const GameStatsMetrics *gamestats) {
    CURL *curl = curl_easy_init();
    if (curl) {
        char post_data[1024];
        snprintf(post_data, sizeof(post_data),
            "{\"games\": %d, \"escaped\": %d, \"mastered\": %d, \"won\": %d, \"winRate\": %f, "
            "\"deepestLevel\": %d, \"cumulativeLevels\": %d, \"highestScore\": %d, \"cumulativeScore\": %lu, "
            "\"mostGold\": %d, \"cumulativeGold\": %lu, \"mostLumenstones\": %d, \"cumulativeLumenstones\": %d, "
            "\"fewestTurnsWin\": %d, \"cumulativeTurns\": %lu, \"longestWinStreak\": %d, \"longestMasteryStreak\": %d, "
            "\"currentWinStreak\": %d, \"currentMasteryStreak\": %d}",
            gamestats->games, gamestats->escaped, gamestats->mastered, gamestats->won, gamestats->winRate,
            gamestats->deepestLevel, gamestats->cumulativeLevels, gamestats->highestScore, gamestats->cumulativeScore,
            gamestats->mostGold, gamestats->cumulativeGold, gamestats->mostLumenstones, gamestats->cumulativeLumenstones,
            gamestats->fewestTurnsWin, gamestats->cumulativeTurns, gamestats->longestWinStreak, gamestats->longestMasteryStreak,
            gamestats->currentWinStreak, gamestats->currentMasteryStreak);

        // Set up CURL options
        curl_easy_setopt(curl, CURLOPT_URL, GAMESTATS_PORTAL_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        // Set headers for JSON content
        struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Send the request and handle errors
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send game stats metrics: %s\n", curl_easy_strerror(res));
        } else {
            printf("Successfully sent game stats metrics to portal.\n");
        }

        // Clean    
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "CURL initialization failed.\n");
    }
}
