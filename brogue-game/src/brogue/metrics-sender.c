#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"

// Define the Portal URL
#define PORTAL_URL "http://portal-service.portal:5000/metrics"

// Prototype for send_metrics_to_portal
static void send_metrics_to_portal(int gold, int depth, int hp, int turns);

void update_metrics(void) {
    // Collect metrics based on the game state
    int player_gold = rogue.gold;
    int player_depth = rogue.depthLevel;
    int player_hp = player.currentHP;
    int player_turns = rogue.absoluteTurnNumber;

    // Log the collected metrics for debugging
    printf("Updating metrics: player gold=%d, depth=%d, current HP=%d, turns=%d\n",
           player_gold, player_depth, player_hp, player_turns);

    // Send the metrics to the Portal server
    send_metrics_to_portal(player_gold, player_depth, player_hp, player_turns);
}

// Function to send metrics data to the Portal
static void send_metrics_to_portal(int gold, int depth, int hp, int turns) {
    CURL *curl;
    CURLcode res;

    // Initialize a CURL session
    curl = curl_easy_init();
    if (curl) {
        // Prepare JSON data for the POST request
        char post_data[256];
        snprintf(post_data, sizeof(post_data), "{\"gold\": %d, \"depth\": %d, \"hp\": %d, \"turns\": %d}",
                 gold, depth, hp, turns);

        // Set the URL of the Portal
        curl_easy_setopt(curl, CURLOPT_URL, PORTAL_URL);

        // Specify the POST data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        // Set headers to indicate JSON content type
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the POST request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Metrics sent successfully: %s\n", post_data);
        }

        // Clean up
        curl_slist_free_all(headers); // Free the custom headers
        curl_easy_cleanup(curl);      // Clean up CURL session
    } else {
        fprintf(stderr, "Failed to initialize CURL session.\n");
    }
}
