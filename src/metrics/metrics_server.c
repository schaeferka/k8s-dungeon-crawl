#include <microhttpd.h>  // For MHD functions
#include <stdio.h>
#include <stdlib.h>
#include <Rogue.h>

#define MAX_CREATURES 500

// External references to player and creature data
extern playerCharacter player;
//extern creature *creatures[MAX_CREATURES]; // Assumes you have an array of creatures


//int player_health = 100;
//int levels_completed = 0;

#define PORT 8000

// Function to respond with Prometheus metrics
enum MHD_Result metrics_handler(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method, const char *version,
                                const char *upload_data, unsigned long *upload_data_size, void **con_cls) {
    if (strcmp(url, "/metrics") != 0) {
        return MHD_NO; // Only respond to /metrics endpoint
    }

    // Collect metrics
    char metrics[2048];  // Buffer for metrics output
    int offset = 0;

    // Add player metrics
    offset += snprintf(metrics + offset, sizeof(metrics) - offset,
                       "# HELP brogue_player_turns Total turns played by the player\n"
                       "# TYPE brogue_player_turns counter\n"
                       "brogue_player_turns %lu\n",
                       player.currentTurnNumber);

    offset += snprintf(metrics + offset, sizeof(metrics) - offset,
                       "# HELP brogue_depth_level Current depth level of the player\n"
                       "# TYPE brogue_depth_level gauge\n"
                       "brogue_depth_level %d\n",
                       player.depthLevel);

    offset += snprintf(metrics + offset, sizeof(metrics) - offset,
                       "# HELP brogue_player_gold Amount of gold collected by the player\n"
                       "# TYPE brogue_player_gold counter\n"
                       "brogue_player_gold %lu\n",
                       player.gold);

    // Example: Add creature metrics (for all creatures in `creatures` array)
    //for (int i = 0; i < MAX_CREATURES; i++) {
    //    if (creatures[i] != NULL) {
    //        offset += snprintf(metrics + offset, sizeof(metrics) - offset,
    //                           "# HELP brogue_creature_hp Current HP of the creature\n"
    //                           "# TYPE brogue_creature_hp gauge\n"
    //                           "brogue_creature_hp{creature_index=\"%d\"} %d\n",
    //                           i, creatures[i]->currentHP);
    //    }
    //}

    // Respond with the metrics
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(metrics), (void*) metrics, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

void start_metrics_server() {
    struct MHD_Daemon *daemon;
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &metrics_handler, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start HTTP server\n");
        exit(1);
    }
    printf("Metrics server running on http://localhost:%d/metrics\n", PORT);
}