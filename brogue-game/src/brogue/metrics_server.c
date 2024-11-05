#include "mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "Rogue.h"
#include "GlobalsBase.h"

#define PORT "18000"

// Shared data structure for metrics
typedef struct GameMetrics {
    int player_gold;
    int player_depth;
    int player_current_hp;
    long player_turn_number;
} GameMetrics;

// Initialize the shared metrics and mutex
GameMetrics gameMetrics = {0};
pthread_mutex_t metrics_mutex = PTHREAD_MUTEX_INITIALIZER;

// Getter functions to fetch metrics from `rogue`
void update_metrics() {
    pthread_mutex_lock(&metrics_mutex);  // Lock the mutex

    gameMetrics.player_gold = rogue.gold;
    gameMetrics.player_depth = rogue.depthLevel;
    gameMetrics.player_current_hp = player.currentHP;
    gameMetrics.player_turn_number = rogue.absoluteTurnNumber;

    // Print for debugging purposes
    printf("Updating metrics: player gold=%d, depth=%d, current HP=%d, turns=%ld\n",
           gameMetrics.player_gold,
           gameMetrics.player_depth,
           gameMetrics.player_current_hp,
           gameMetrics.player_turn_number);

    fflush(stdout);

    pthread_mutex_unlock(&metrics_mutex);  // Unlock the mutex
}

// HTTP handler function
static void handle_metrics(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        // Check if the request is for the /metrics endpoint
        if (mg_http_match_uri(hm, "/metrics")) {
            char metrics[2048];

            pthread_mutex_lock(&metrics_mutex);  // Lock mutex to access metrics

            // Collect metrics into the buffer
            snprintf(metrics, sizeof(metrics),
                     "# HELP brogue_player_turns Total turns played by the player\n"
                     "# TYPE brogue_player_turns counter\n"
                     "brogue_player_turns %ld\n"
                     "# HELP brogue_depth_level Current depth level of the player\n"
                     "# TYPE brogue_depth_level gauge\n"
                     "brogue_depth_level %d\n"
                     "# HELP brogue_player_gold Amount of gold collected by the player\n"
                     "# TYPE brogue_player_gold counter\n"
                     "brogue_player_gold %d\n"
                     "# HELP brogue_player_currentHP Current hit points of the player\n"
                     "# TYPE brogue_player_currentHP gauge\n"
                     "brogue_player_currentHP %d\n",
                     gameMetrics.player_turn_number,
                     gameMetrics.player_depth,
                     gameMetrics.player_gold,
                     gameMetrics.player_current_hp);

            pthread_mutex_unlock(&metrics_mutex);  // Unlock the mutex

            // Send HTTP response with metrics data
            mg_http_reply(conn, 200, "Content-Type: text/plain\r\n", "%s", metrics);
        } else {
            // Send 404 for any other endpoints
            mg_http_reply(conn, 404, "Content-Type: text/plain\r\n", "Not found");
        }
    }
}

// Start Mongoose server
void start_metrics_server() {
    struct mg_mgr mgr;
    struct mg_connection *conn;

    mg_mgr_init(&mgr);  // Initialize Mongoose manager

    conn = mg_http_listen(&mgr, "http://0.0.0.0:" PORT, handle_metrics, &mgr);
    if (conn == NULL) {
        fprintf(stderr, "Error starting server on port %s\n", PORT);
        exit(1);
    }

    printf("Metrics server running on http://localhost:%s/metrics\n", PORT);

    // Main loop to handle HTTP requests and update metrics
    for (;;) {
        update_metrics();   // Update metrics in each loop iteration
        mg_mgr_poll(&mgr, 1000); // Poll for new events every second
    }

    mg_mgr_free(&mgr);  // Free resources when server stops (unreachable in this loop)
}
