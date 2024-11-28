#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal_gamestate.h"
#include "portal.h"

// Declare a previous game state variable
static GameStateData previous_gamestate = {0};

/**
 * @brief Initiate the updating of the game state.
 * 
 * This function is responsible for updating the game state and sending it to the portal.
 */
void update_gamestate(void) {
    if (is_gamestate_changed()) {
        GameStateData gamestate = {
            .wizard = rogue.wizard,
            .rewardRoomsGenerated = rogue.rewardRoomsGenerated,
            .goldGenerated = rogue.goldGenerated,
            .gameInProgress = rogue.gameInProgress,
            .gameHasEnded = rogue.gameHasEnded,
            .easyMode = rogue.easyMode,
            .seed = rogue.seed,
            .RNG = rogue.RNG,
            .absoluteTurnNumber = rogue.absoluteTurnNumber,
            .milliseconds = rogue.milliseconds,
            .monsterSpawnFuse = rogue.monsterSpawnFuse,
            .turns = rogue.absoluteTurnNumber,
            .currentDepth = rogue.depthLevel,
            .deepestLevel = rogue.deepestLevel
        };
        
        // Generate JSON and send it using the portal function
        char gamestate_json[1024];
        generate_gamestate_json(&gamestate, gamestate_json, sizeof(gamestate_json));
        send_gamestate_to_portal(gamestate_json);

        // Update the previous game state
        previous_gamestate = gamestate;
    }
}

/**
 * @brief Check if the game state has changed since the last update.
 *
 * This function compares the current game state with the previously sent game state to detect changes.
 *
 * @return `true` if there are changes, `false` otherwise.
 */
bool is_gamestate_changed(void) {
    return (previous_gamestate.wizard != rogue.wizard ||
            previous_gamestate.rewardRoomsGenerated != rogue.rewardRoomsGenerated ||
            previous_gamestate.goldGenerated != rogue.goldGenerated ||
            previous_gamestate.gameInProgress != rogue.gameInProgress ||
            previous_gamestate.gameHasEnded != rogue.gameHasEnded ||
            previous_gamestate.easyMode != rogue.easyMode ||
            previous_gamestate.seed != rogue.seed ||
            previous_gamestate.RNG != rogue.RNG ||
            previous_gamestate.absoluteTurnNumber != rogue.absoluteTurnNumber ||
            previous_gamestate.milliseconds != rogue.milliseconds ||
            previous_gamestate.monsterSpawnFuse != rogue.monsterSpawnFuse ||
            previous_gamestate.turns != rogue.absoluteTurnNumber ||
            previous_gamestate.currentDepth != rogue.depthLevel ||
            previous_gamestate.deepestLevel != rogue.deepestLevel);
}

/**
 * @brief Generates the JSON string for the game state.
 *
 * This function generates a JSON string representing the current game state metrics.
 *
 * @param gamestate The current game state metrics.
 * @param buffer The buffer to store the resulting JSON string.
 * @param size The size of the buffer.
 */
void generate_gamestate_json(const GameStateData *gamestate, char *buffer, size_t size) {
    snprintf(buffer, size,
        "{"
        "\"wizard\": \"%s\", "
        "\"reward_rooms_generated\": %d, "
        "\"gold_generated\": %d, "
        "\"current_depth\": %d, "
        "\"deepest_level\": %d, "
        "\"game_in_progress\": \"%s\", "
        "\"game_has_ended\": \"%s\", "
        "\"easy_mode\": \"%s\", "
        "\"seed\": %d, "
        "\"rng\": %d, "
        "\"absolute_turn_number\": %d, "
        "\"milliseconds\": %d, "
        "\"monster_spawn_fuse\": %d, "
        "\"turns\": %d"
        "}",
        gamestate->wizard ? "true" : "false",
        gamestate->rewardRoomsGenerated, gamestate->goldGenerated, gamestate->currentDepth, gamestate->deepestLevel,
        gamestate->gameInProgress ? "true" : "false", gamestate->gameHasEnded ? "true" : "false",
        gamestate->easyMode ? "true" : "false", gamestate->seed, gamestate->RNG,
        gamestate->absoluteTurnNumber, gamestate->milliseconds, gamestate->monsterSpawnFuse, gamestate->turns);
}
