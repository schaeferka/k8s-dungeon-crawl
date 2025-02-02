#ifndef PORTAL_H
#define PORTAL_H

#include "portal_items.h"
#include <stdbool.h>

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

/**
 * Sends the gamestate data to the portal.
 * 
 * @param gamestate_data The JSON data representing the gamestate.
 */
extern void send_gamestate_to_portal(const char *gamestate_data);

/**
 * Sends the gamstats data to the portal.
 * 
 * @param gamestats_data The JSON data representing the gamestats.
 */
extern void send_gamestats_to_portal(const char *gamestats_data);

/**
 * @brief Sends a game reset command to the portal.
 *
 * This function sends a reset command to the portal, instructing it to reset the state
 * of all game data.
 */
extern void send_game_reset_to_portal(void);

/**
 * @brief Checks if a monster is in admin kills list.
 *
 * This function sends a monster_id to the portal to check if it is in the 
 * admin_kills list.
 */
bool is_monster_in_admin_kills(int monster_id);

char **get_new_monsties(void);

#endif // PORTAL_H
