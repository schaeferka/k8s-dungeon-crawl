#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portal.h"
#include "portal_urls.h"

/**
 * Sends data to the specified URL using a POST request.
 * 
 * @param url The URL to send the data to.
 * @param data The JSON data to send. If NULL, no data is sent.
 */
void send_data_to_portal(const char *url, const char *data) {
    //printf("Getting ready to send data to %s\n", url);
    //printf("Data: %s\n", data);
    CURL *curl = curl_easy_init();  // Initialize the CURL session
    if (curl) {
        struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url);            // Set URL
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);    // Set POST data
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); // Set headers

        CURLcode res = curl_easy_perform(curl);  // Perform the request

        //printf("Response from portal: %d\n", res);
        if (res != CURLE_OK) {
            // Print error message if the request fails
            fprintf(stderr, "Failed to send data to portal: %s\n", curl_easy_strerror(res));
            printf("Curl response data: %s\n", curl_easy_strerror(res));
        } else {
            // Success message if request is successful
            //printf("Successfully sent data to %s\n", url);
        }

        curl_slist_free_all(headers);  // Free the headers list
        curl_easy_cleanup(curl);       // Cleanup CURL session
    } else {
        // Error message if CURL initialization fails
        fprintf(stderr, "CURL initialization failed.\n");
    }
}

/**
 * Sends monster death data to the portal.
 * 
 * @param monster_data The JSON data representing the monster's death.
 */
void send_monster_death_to_portal(const char *monster_data) {
    send_data_to_portal(get_monster_death_url(), monster_data);
}

/**
 * Sends the list of monsters to the portal.
 * 
 * @param monster_data The JSON data representing the monsters.
 */
void send_monsters_to_portal(const char *monster_data) {
    send_data_to_portal(get_monster_update_url(), monster_data);
}

/**
 * Sends a monster reset command to the portal.
 */
void send_monster_reset_to_portal(void) {
    send_data_to_portal(get_monster_reset_url(), NULL);
}

/**
 * Sends player death data to the portal.
 * 
 * @param player_data The JSON data representing the player's death.
 */
void send_player_death_to_portal(const char *player_data) {
    send_data_to_portal(get_player_death_url(), player_data);
}

/**
 * Sends the player data to the portal.
 * 
 * @param player_data The JSON data representing the player.
 */
void send_player_to_portal(const char *player_data) {
    send_data_to_portal(get_player_update_url(), player_data);
}

/**
 * Sends a player reset command to the portal.
 */
void send_player_reset_to_portal(void) {
    send_data_to_portal(get_player_reset_url(), NULL);
}

/**
 * Sends the equipped items data to the portal.
 * 
 * @param items_data The JSON data representing the player.
 */
extern void send_items_to_portal(const char *items_data){
    send_data_to_portal(get_equipped_items_url(), items_data);
}

/**
 * Sends the player data to the portal.
 * 
 * @param pack_data The JSON data representing the player.
 */
void send_pack_to_portal(const char *pack_data) {
    send_data_to_portal(get_pack_url(), pack_data);
}

/**
 * Sends the gamestate data to the portal.
 * 
 * @param gamestate_data The JSON data representing the gamestate.
 */
void send_gamestate_to_portal(const char *gamestate_data) {
    send_data_to_portal(get_gamestate_url(), gamestate_data);
}

/**
 * Sends the gamestats data to the portal.
 * 
 * @param gamestats_data The JSON data representing the gamestats.
 */
void send_gamestats_to_portal(const char *gamestats_data) {
    send_data_to_portal(get_gamestats_url(), gamestats_data);
}

/**
 * Sends a reset command for the items to the portal.
 */
void send_game_reset_to_portal(void) {
    send_data_to_portal(get_game_reset_url(), NULL);
}
