#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal_gamestats.h"
#include "platform.h"
#include "MainMenu.h"
#include "portal.h"

// Declare previous game stats
static gameStats previous_gamestats = {0};

/**
 * @brief Initiate the updating of game stats.
 *
 * This function is responsible for updating game stats and sending them to the portal.
 */
void update_gamestats(void) {
    gameStats gamestats = {0};

    rogueRun *runHistory = loadRunHistory();
    rogueRun *currentRun = runHistory;

    // Aggregate data from each run into gamestats
    while (currentRun != NULL) {
        if (currentRun->seed != 0) {  // Only process valid runs
            addRuntoGameStats(currentRun, &gamestats);
        }
        currentRun = currentRun->nextRun;
    }

    // Free the run history after aggregating data
    currentRun = runHistory;
    rogueRun *nextRun;
    while (currentRun != NULL) {
        nextRun = currentRun->nextRun;
        free(currentRun);
        currentRun = nextRun;
    }
// Check if game stats have changed
    if (is_gamestats_changed(&gamestats)) {
        char jsonBuffer[1024];
        extractGameStatsJSON(&gamestats, jsonBuffer, sizeof(jsonBuffer));

        send_gamestats_to_portal(jsonBuffer);

        previous_gamestats = gamestats;
    }
}


/**
 * @brief Check if the game stats have changed since the last update.
 *
 * This function compares the current game stats with the previously sent game stats to detect changes.
 *
 * @return `true` if there are changes, `false` otherwise.
 */
bool is_gamestats_changed(const gameStats *current_stats) {
    return (previous_gamestats.games != current_stats->games ||
            previous_gamestats.escaped != current_stats->escaped ||
            previous_gamestats.mastered != current_stats->mastered ||
            previous_gamestats.won != current_stats->won ||
            previous_gamestats.winRate != current_stats->winRate ||
            previous_gamestats.deepestLevel != current_stats->deepestLevel ||
            previous_gamestats.cumulativeLevels != current_stats->cumulativeLevels ||
            previous_gamestats.highestScore != current_stats->highestScore ||
            previous_gamestats.cumulativeScore != current_stats->cumulativeScore ||
            previous_gamestats.mostGold != current_stats->mostGold ||
            previous_gamestats.cumulativeGold != current_stats->cumulativeGold ||
            previous_gamestats.mostLumenstones != current_stats->mostLumenstones ||
            previous_gamestats.cumulativeLumenstones != current_stats->cumulativeLumenstones ||
            previous_gamestats.fewestTurnsWin != current_stats->fewestTurnsWin ||
            previous_gamestats.cumulativeTurns != current_stats->cumulativeTurns ||
            previous_gamestats.longestWinStreak != current_stats->longestWinStreak ||
            previous_gamestats.longestMasteryStreak != current_stats->longestMasteryStreak ||
            previous_gamestats.currentWinStreak != current_stats->currentWinStreak ||
            previous_gamestats.currentMasteryStreak != current_stats->currentMasteryStreak);
}

/**
 * @brief Generates the JSON string for the game stats.
 *
 * This function generates a JSON string representing the current game stats metrics.
 *
 * @param stats The current game stats.
 * @param buffer The buffer to store the resulting JSON string.
 * @param size The size of the buffer.
 */
void extractGameStatsJSON(const gameStats *stats, char *buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size,
        "{"
        "\"games\": %d, "
        "\"escaped\": %d, "
        "\"mastered\": %d, "
        "\"won\": %d, "
        "\"winRate\": %.2f, "
        "\"deepestLevel\": %d, "
        "\"cumulativeLevels\": %d, "
        "\"highestScore\": %d, "
        "\"cumulativeScore\": %lu, "
        "\"mostGold\": %d, "
        "\"cumulativeGold\": %lu, "
        "\"mostLumenstones\": %d, "
        "\"cumulativeLumenstones\": %d, "
        "\"fewestTurnsWin\": %d, "
        "\"cumulativeTurns\": %lu, "
        "\"longestWinStreak\": %d, "
        "\"longestMasteryStreak\": %d, "
        "\"currentWinStreak\": %d, "
        "\"currentMasteryStreak\": %d"
        "}",
        stats->games,
        stats->escaped,
        stats->mastered,
        stats->won,
        stats->winRate,
        stats->deepestLevel,
        stats->cumulativeLevels,
        stats->highestScore,
        stats->cumulativeScore,
        stats->mostGold,
        stats->cumulativeGold,
        stats->mostLumenstones,
        stats->cumulativeLumenstones,
        stats->fewestTurnsWin,
        stats->cumulativeTurns,
        stats->longestWinStreak,
        stats->longestMasteryStreak,
        stats->currentWinStreak,
        stats->currentMasteryStreak
    );
}
