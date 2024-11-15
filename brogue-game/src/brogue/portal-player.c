#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal-player.h"

#define PLAYER_PORTAL_URL "http://portal-service.portal:5000/player/update"

// Function to send player information to the portal
void update_player(void) {
    if (!rogue.gameHasEnded) {
        PlayerMetrics metrics = {
            .gold = rogue.gold,
            .depthLevel = rogue.depthLevel,
            .deepestLevel = rogue.deepestLevel,
            .currentHP = player.currentHP,
            .maxHP = player.info.maxHP,
            .strength = rogue.strength,
            .playerTurnNumber = rogue.playerTurnNumber,
            .xpxpThisTurn = rogue.xpxpThisTurn,
            .stealthRange = rogue.stealthRange,
            .disturbed = rogue.disturbed,
            .regenPerTurn = player.regenPerTurn,
            .weaknessAmount = player.weaknessAmount,
            .poisonAmount = player.poisonAmount,
            .clairvoyance = rogue.clairvoyance,
            .stealthBonus = rogue.stealthBonus,
            .regenerationBonus = rogue.regenerationBonus,
            .lightMultiplier = rogue.lightMultiplier,
            .awarenessBonus = rogue.awarenessBonus,
            .transference = rogue.transference,
            .wisdomBonus = rogue.wisdomBonus,
            .reaping = rogue.reaping,
        };
        send_player_metrics_to_portal(&metrics);
    }
}

// Function to send player metrics to the portal
void send_player_metrics_to_portal(const PlayerMetrics *metrics) {
    CURL *curl = curl_easy_init();
    if (curl) {
        char post_data[1024];
        snprintf(post_data, sizeof(post_data),
            "{\"gold\": %d, \"depthLevel\": %d, \"deepestLevel\": %d, "
            "\"currentHP\": %d, \"maxHP\": %d, \"strength\": %d, "
            "\"playerTurnNumber\": %d, \"xpxpThisTurn\": %d, \"stealthRange\": %d, "
            "\"disturbed\": %d, \"regenPerTurn\": %d, \"weaknessAmount\": %d, \"poisonAmount\": %d, "
            "\"clairvoyance\": %d, \"stealthBonus\": %d, \"regenerationBonus\": %d, "
            "\"lightMultiplier\": %d, \"awarenessBonus\": %d, \"transference\": %d, "
            "\"wisdomBonus\": %d, \"reaping\": %d}",
            metrics->gold, metrics->depthLevel, metrics->deepestLevel,
            metrics->currentHP, metrics->maxHP, metrics->strength,
            metrics->playerTurnNumber, metrics->xpxpThisTurn, metrics->stealthRange,
            metrics->disturbed ? 1 : 0,  // Ensure this is a boolean value (1 or 0, not "true" or "false")
            metrics->regenPerTurn, metrics->weaknessAmount, metrics->poisonAmount,
            metrics->clairvoyance, metrics->stealthBonus, metrics->regenerationBonus,
            metrics->lightMultiplier, metrics->awarenessBonus, metrics->transference,
            metrics->wisdomBonus, metrics->reaping);

        curl_easy_setopt(curl, CURLOPT_URL, PLAYER_PORTAL_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        // Set HTTP headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the POST request and check for errors
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send player data to portal: %s\n", curl_easy_strerror(res));
        } else {
            printf("Successfully sent player data to portal: %s\n", post_data);
        }

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "CURL initialization failed for sending player data.\n");
    }
}
