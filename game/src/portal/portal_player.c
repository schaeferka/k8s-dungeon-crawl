#include "GlobalsBase.h"
#include "portal_player.h"
#include "portal.h" 
#include "portal_urls.h"

/** 
 * @brief Cache for storing player data for comparison and change detection.
 */
static PlayerData cachedPlayerData = {0};

/**
 * @brief Checks if the player data has changed since the last update.
 *
 * This function compares the current player's data with its cached data to
 * determine if an update is needed.
 *
 * @return `true` if the data has changed, `false` otherwise.
 */
static bool has_player_data_changed(void) {
    // Compare cached data with current player data
    if (memcmp(&cachedPlayerData, &rogue, sizeof(PlayerData)) != 0) {
        // Update cache with the current data if changed
        memcpy(&cachedPlayerData, &rogue, sizeof(PlayerData));
        return true;
    }
    return false;
}

/**
 * @brief Updates the player data and sends it to the portal if changed.
 *
 * This function checks if the player data has changed, generates the JSON string,
 * and sends the data to the portal if necessary.
 */
void update_player(void) {
    if (!rogue.gameHasEnded && has_player_data_changed()) {
        // Form the player data
        PlayerData player_data = {
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

        // Generate the JSON string for the player data
        char *player_json = generate_player_json(&player_data);

        send_player_to_portal(player_json);

        free(player_json);
    }
}

/**
 * @brief Resets the player data.
 *
 * This function resets the player data to its initial state.
 */
void reset_player(void) {
    memset(&cachedPlayerData, 0, sizeof(cachedPlayerData));
}

/**
 * @brief Generates the JSON string for the player data.
 *
 * This function takes in the relevant player data and formats it into a JSON string.
 * 
 * @param player_data The player data to be converted into JSON.
 * @return A dynamically allocated string containing the player data in JSON format.
 */
char *generate_player_json(const PlayerData *player_data) {
    // Allocate memory for the JSON string
    char *json_data = (char *)malloc(1024 * sizeof(char));
    if (!json_data) {
        fprintf(stderr, "Memory allocation failed for player JSON\n");
        return NULL;
    }

    // Format the player data into JSON
    snprintf(json_data, 1024,
        "{\"gold\": %d, \"depth_level\": %d, \"deepest_level\": %d, "
        "\"current_hp\": %d, \"max_hp\": %d, \"strength\": %d, "
        "\"player_turn_number\": %d, \"xpxp_this_turn\": %d, \"stealth_range\": %d, "
        "\"disturbed\": %d, \"regen_per_turn\": %d, \"weakness_amount\": %d, \"poison_amount\": %d, "
        "\"clairvoyance\": %d, \"stealth_bonus\": %d, \"regeneration_bonus\": %d, "
        "\"light_multiplier\": %d, \"awareness_bonus\": %d, \"transference\": %d, "
        "\"wisdom_bonus\": %d, \"reaping\": %d}",
        player_data->gold, player_data->depthLevel, player_data->deepestLevel,
        player_data->currentHP, player_data->maxHP, player_data->strength,
        player_data->playerTurnNumber, player_data->xpxpThisTurn, player_data->stealthRange,
        player_data->disturbed ? 1 : 0,  // Ensure this is a boolean value (1 or 0, not "true" or "false")
        player_data->regenPerTurn, player_data->weaknessAmount, player_data->poisonAmount,
        player_data->clairvoyance, player_data->stealthBonus, player_data->regenerationBonus,
        player_data->lightMultiplier, player_data->awarenessBonus, player_data->transference,
        player_data->wisdomBonus, player_data->reaping);

    return json_data;
}
