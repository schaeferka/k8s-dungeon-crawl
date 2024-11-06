#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"

// Define the Portal URL
#define PORTAL_URL "http://portal-service.portal:5000/metrics"

// Define the PlayerMetrics struct
typedef struct {
    // Numeric metrics
    int gold;
    int goldGenerated;
    int depthLevel;
    int deepestLevel;
    int hp;
    int turns;
    int strength;
    int monsterSpawnFuse;

    int playerTurnNumber;               // number of input turns in recording. Does not increment during paralysis.
    int absoluteTurnNumber;             // number of turns since the beginning of time. Always increments.
    int milliseconds;                   // milliseconds since launch, to decide whether to engage cautious mode
    int xpxpThisTurn;                   // how many squares the player explored this turn
    int stealthRange;                   // distance from which monsters will notice you

    int rewardRoomsGenerated;           // to meter the number of reward machines

    // Non-numeric metrics
    boolean wizard;
    boolean disturbed;                  // player should stop auto-acting
    boolean gameInProgress;             // the game is in progress (the player has not died, won or quit yet)
    boolean gameHasEnded;               // stop everything and go to death screen
    boolean easyMode;                   // enables easy mode

    int seed;                           // the master seed for generating the entire dungeon
    int RNG;                            // which RNG are we currently using?

    // ring bonuses:
    int clairvoyance;
    int stealthBonus;
    int regenerationBonus;
    int lightMultiplier;
    int awarenessBonus;
    int transference;
    int wisdomBonus;
    int reaping;

    int regenPerTurn;                 // number of HP to regenerate every single turn
    int weaknessAmount;               // number of points of weakness that are inflicted by the weakness status
    int poisonAmount; 

    // Additional metrics can be added here
} PlayerMetrics;

// Function prototypes
static void send_metrics_to_portal(const PlayerMetrics *metrics);
void update_metrics(void);

void update_metrics(void) {
    // Create and populate an instance of PlayerMetrics
    PlayerMetrics metrics = {
        .gold = rogue.gold,
        .goldGenerated = rogue.goldGenerated,
        .depthLevel = rogue.depthLevel,
        .deepestLevel = rogue.deepestLevel,
        .hp = player.currentHP,
        .turns = rogue.absoluteTurnNumber,
        .strength = rogue.strength,
        .monsterSpawnFuse = rogue.monsterSpawnFuse,
        .playerTurnNumber = rogue.playerTurnNumber,
        .absoluteTurnNumber = rogue.absoluteTurnNumber,
        .milliseconds = rogue.milliseconds,
        .xpxpThisTurn = rogue.xpxpThisTurn,
        .stealthRange = rogue.stealthRange,
        .rewardRoomsGenerated = rogue.rewardRoomsGenerated,

        .wizard = rogue.wizard,
        .disturbed = rogue.disturbed,
        .gameInProgress = rogue.gameInProgress,
        .gameHasEnded = rogue.gameHasEnded,
        .easyMode = rogue.easyMode,

        .seed = rogue.seed,
        .RNG = rogue.RNG,
    
        // ring bonuses:
        .clairvoyance = rogue.clairvoyance,
        .stealthBonus = rogue.stealthBonus,
        .regenerationBonus = rogue.regenerationBonus,
        .lightMultiplier = rogue.lightMultiplier,
        .awarenessBonus = rogue.awarenessBonus,
        .transference = rogue.transference,         
        .wisdomBonus = rogue.wisdomBonus,
        .reaping = rogue.reaping, 
        
        .regenPerTurn = player.regenPerTurn,  
        .weaknessAmount = player.weaknessAmount,
        .poisonAmount = player.poisonAmount              
    };

    // Log for debugging
    printf("Updating metrics: gold=%d, depth=%d, hp=%d, turns=%d, wizard=%d, gameInProgress=%d\n",
           metrics.gold, metrics.depthLevel, metrics.hp, metrics.turns, metrics.wizard, metrics.gameInProgress);

    // Send the metrics to the Portal server
    send_metrics_to_portal(&metrics);
}

static void send_metrics_to_portal(const PlayerMetrics *metrics) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        // Prepare JSON data for the POST request with all metrics
        char post_data[1024];
        snprintf(post_data, sizeof(post_data),
                 "{\"gold\": %d, \"goldGenerated\": %d, \"depthLevel\": %d, \"deepestLevel\": %d, "
                 "\"hp\": %d, \"turns\": %d, \"strength\": %d, \"monsterSpawnFuse\": %d, "
                 "\"playerTurnNumber\": %d, \"absoluteTurnNumber\": %d, \"milliseconds\": %d, "
                 "\"xpxpThisTurn\": %d, \"stealthRange\": %d, \"rewardRoomsGenerated\": %d, "
                 "\"wizard\": %s, \"disturbed\": %s, \"gameInProgress\": %s, \"gameHasEnded\": %s, "
                 "\"easyMode\": %s, \"seed\": %d, \"RNG\": %d, "
                 "\"clairvoyance\": %d, \"stealthBonus\": %d, \"regenerationBonus\": %d, "
                 "\"lightMultiplier\": %d, \"awarenessBonus\": %d, \"transference\": %d, "
                 "\"wisdomBonus\": %d, \"reaping\": %d, \"regenPerTurn\": %d, "
                 "\"weaknessAmount\": %d, \"poisonAmount\": %d}",
                 metrics->gold, metrics->goldGenerated, metrics->depthLevel, metrics->deepestLevel,
                 metrics->hp, metrics->turns, metrics->strength, metrics->monsterSpawnFuse,
                 metrics->playerTurnNumber, metrics->absoluteTurnNumber, metrics->milliseconds,
                 metrics->xpxpThisTurn, metrics->stealthRange, metrics->rewardRoomsGenerated,
                 metrics->wizard ? "true" : "false", metrics->disturbed ? "true" : "false",
                 metrics->gameInProgress ? "true" : "false", metrics->gameHasEnded ? "true" : "false",
                 metrics->easyMode ? "true" : "false", metrics->seed, metrics->RNG,
                 metrics->clairvoyance, metrics->stealthBonus, metrics->regenerationBonus,
                 metrics->lightMultiplier, metrics->awarenessBonus, metrics->transference,
                 metrics->wisdomBonus, metrics->reaping, metrics->regenPerTurn,
                 metrics->weaknessAmount, metrics->poisonAmount);

        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, PORTAL_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        // Set headers to indicate JSON content type
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the POST request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send metrics: %s\n", curl_easy_strerror(res));
        } else {
            printf("Metrics sent successfully: %s\n", post_data);
        }

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Failed to initialize CURL session for metrics.\n");
    }
}
