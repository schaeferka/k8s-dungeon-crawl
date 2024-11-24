#ifndef PORTAL_URLS_H
#define PORTAL_URLS_H

/**
 * @brief Base URL for the portal service.
 *
 * This is the base URL used for interacting with the portal service. All other endpoints
 * are appended to this URL.
 */
extern const char *PORTAL_BASE_URL;

/**
 * @brief Get the URL for updating monster data in the portal.
 *
 * This function generates the full URL for updating monster data by appending the
 * endpoint to the base URL.
 *
 * @return The full URL for updating monster data.
 */
const char *get_monster_update_url(void);

/**
 * @brief Get the URL for sending monster death data to the portal.
 *
 * This function generates the full URL for sending monster death data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for sending monster death data.
 */
const char *get_monster_death_url(void);

/**
 * @brief Get the URL for resetting monster data in the portal.
 *
 * This function generates the full URL for resetting monster data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for resetting monster data.
 */
const char *get_monster_reset_url(void);

/**
 * @brief Get the URL for updating monster data in the portal.
 *
 * This function generates the full URL for updating monster data by appending the
 * endpoint to the base URL.
 *
 * @return The full URL for updating monster data.
 */
const char *get_player_update_url(void);

/**
 * @brief Get the URL for sending player death data to the portal.
 *
 * This function generates the full URL for sending player death data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for sending player death data.
 */
const char *get_player_death_url(void);

/**
 * @brief Get the URL for resetting player data in the portal.
 *
 * This function generates the full URL for resetting player data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for resetting player data.
 */
const char *get_player_reset_url(void);

/**
 * @brief Get the URL for resetting player data in the portal.
 *
 * This function generates the full URL for resetting player data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for resetting player data.
 */
const char *get_equipped_items_url(void);

/**
 * @brief Get the URL for resetting player data in the portal.
 *
 * This function generates the full URL for resetting player data by appending
 * the endpoint to the base URL.
 *
 * @return The full URL for resetting player data.
 */
const char *get_pack_url(void);

#endif // PORTAL_URLS_H
