#include <stdio.h>
#include <string.h>  // For snprintf
#include "portal_urls.h"

/**
 * @brief Base URL for the portal service.
 *
 * This is the base URL used for interacting with the portal service. All other endpoints
 * are appended to this URL.
 */
const char *PORTAL_BASE_URL = "http://portal-service.portal:5000";

/**
 * @brief Get the URL for updating monster data in the portal.
 *
 * This function generates the full URL for updating monster data by appending the
 * endpoint to the base URL.
 *
 * @return The full URL for updating monster data.
 */
const char *get_monster_update_url(void) {
    static char url[256]; 
    snprintf(url, sizeof(url), "%s/monsters/update", PORTAL_BASE_URL);
    return url;
}

/**
 * @brief Get the URL for sending monster death data to the portal.
 *
 * This function generates the full URL for sending monster death data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for sending monster death data.
 */
const char *get_monster_death_url(void) {
    static char url[256];
    snprintf(url, sizeof(url), "%s/monsters/death", PORTAL_BASE_URL);
    return url;
}

/**
 * @brief Get the URL for resetting monster data in the portal.
 *
 * This function generates the full URL for resetting monster data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for resetting monster data.
 */
const char *get_monster_reset_url(void) {
    static char url[256];
    snprintf(url, sizeof(url), "%s/monsters/reset", PORTAL_BASE_URL);
    return url;
}

/**
 * @brief Get the URL for updating player data in the portal.
 *
 * This function generates the full URL for updating player data by appending the
 * endpoint to the base URL.
 *
 * @return The full URL for updating player data.
 */
const char *get_player_update_url(void) {
    static char url[256];
    snprintf(url, sizeof(url), "%s/player/update", PORTAL_BASE_URL);
    return url;
}

/**
 * @brief Get the URL for sending player death data to the portal.
 *
 * This function generates the full URL for sending player death data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for sending player death data.
 */
const char *get_player_death_url(void) {
    static char url[256];
    snprintf(url, sizeof(url), "%s/player/death", PORTAL_BASE_URL);
    return url;
}

/**
 * @brief Get the URL for resetting player data in the portal.
 *
 * This function generates the full URL for resetting player data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for resetting player data.
 */
const char *get_player_reset_url(void) {
    static char url[256];
    snprintf(url, sizeof(url), "%s/player/reset", PORTAL_BASE_URL);
    return url;
}

/**
 * @brief Get the URL for sending equipped item data to the portal.
 *
 * This function generates the full URL for sending equipped item data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for sending equipped item data.
 */
const char *get_equipped_items_url(void) {
    static char url[256];
    snprintf(url, sizeof(url), "%s/items/update", PORTAL_BASE_URL);
    return url;
}

/**
 * @brief Get the URL for sending pack data in the portal.
 *
 * This function generates the full URL for sending pack data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for sending pack data.
 */
const char *get_pack_url(void) {
    static char url[256];
    snprintf(url, sizeof(url), "%s/pack/update", PORTAL_BASE_URL);
    return url;
}

/**
 * @brief Get the URL for sending gamestate data in the portal.
 *
 * This function generates the full URL for gamestate data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for sending gamestate data.
 */
const char *get_gamestate_url(void) {
    static char url[256];
    snprintf(url, sizeof(url), "%s/gamestate/update", PORTAL_BASE_URL);
    return url;
}

/**
 * @brief Get the URL for sending gamestats data in the portal.
 *
 * This function generates the full URL for gamestats data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for sending gamestats data.
 */
const char *get_gamestats_url(void) {
    static char url[256];
    snprintf(url, sizeof(url), "%s/gamestats/update", PORTAL_BASE_URL);
    return url;
}

/**
 * @brief Get the URL for resetting gamestate data in the portal.
 *
 * This function generates the full URL for resetting gamestate data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for resetting gamestate data.
 */
const char *get_game_reset_url(void) {
    static char url[256];
    snprintf(url, sizeof(url), "%s/game/reset", PORTAL_BASE_URL);
    return url;
}

/**
 * @brief Get the URL for checking if a monster is in the admin_kills list.
 *
 * This function generates the full URL for checking if a monster is in the admin_kills list
 * by appending the endpoint to the base URL.
 *
 * @param monster_id The ID of the monster to check.
 * @return The full URL for checking if a monster is in the admin_kills list.
 */
const char *get_is_admin_kill_url(int monster_id) {
    static char url[256];
    snprintf(url, sizeof(url), "%s/admin-kills/%d", PORTAL_BASE_URL, monster_id);
    return url;
}