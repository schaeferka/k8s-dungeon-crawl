#ifndef PORTAL_H
#define PORTAL_H

#include "portal_items.h"

void send_data_to_portal(const char *url, const char *data);

/**
 * @brief Sends monster death data to the portal.
 *
 * This function is responsible for sending the JSON data representing a monster's death
 * to the portal using the appropriate endpoint.
 *
 * @param monster_data A JSON string containing the monster's death data.
 */
extern void send_monster_death_to_portal(const char *monster_data);

/**
 * @brief Sends the list of monsters to the portal.
 *
 * This function is responsible for sending the JSON data representing all active monsters
 * in the game to the portal.
 *
 * @param monster_data A JSON string containing the monster data.
 */
extern void send_monsters_to_portal(const char *monster_data);

/**
 * @brief Sends a monster reset command to the portal.
 *
 * This function sends a reset command to the portal, instructing it to reset the state
 * of monster data.
 */
extern void send_monster_reset_to_portal(void);

/**
 * @brief Sends player death data to the portal.
 *
 * This function is responsible for sending the JSON data representing a player's death
 * to the portal using the appropriate endpoint.
 *
 * @param player_data A JSON string containing the player's death data.
 */
extern void send_player_death_to_portal(const char *player_data);

/**
 * @brief Sends the player data to the portal.
 *
 * This function is responsible for sending the JSON data representing the player
 * in the game to the portal.
 *
 * @param player_data A JSON string containing the player data.
 */
extern void send_player_to_portal(const char *player_data);

/**
 * @brief Sends a reset command to the portal.
 *
 * This function sends a reset command to the portal, instructing it to reset the state
 * of player data.
 */
extern void send_player_reset_to_portal(void);

/**
 * Sends the equipped items data to the portal.
 * 
 * @param items_data The JSON data representing the player.
 */
extern void send_items_to_portal(const char *items_data);

/**
 * Sends the player data to the portal.
 * 
 * @param pack_data The JSON data representing the player.
 */
extern void send_pack_to_portal(const char *pack_data);

#endif // PORTAL_H
