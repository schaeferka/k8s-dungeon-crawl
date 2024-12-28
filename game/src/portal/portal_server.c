#include "mongoose.h"
#include <string.h>
#include <stdio.h>

static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        printf("MONSTIES Received request to %.*s\n", (int) hm->uri.len, hm->uri.buf);
        printf("MONSTIES Request body: %.*s\n", (int) hm->body.len, hm->body.buf);

        // Check if the request is for /monster-deleted
        if (mg_match(hm->uri, mg_str("/monster-deleted"), NULL)) {
            //char *monsterID[128] = {0};
            //char *monsterName[128] = {0};

            // Parse JSON payload to extract monster_id and monster_name
            //mg_json_get(hm->body, "$.monster_id", monsterID);
            //mg_json_get(hm->body, "$.monster_name", monsterName);

            char *monsterID = mg_json_get_str(hm->body, "$.monster_id");
            char *monsterName = mg_json_get_str(hm->body, "$.monster_name");

            // Log and process the received payload
            //printf("Monster deleted: ID=%s, Name=%s\n", monsterID, monsterName);

            // Construct the response JSON
            char response[256];
            snprintf(response, sizeof(response), "{\"status\":\"success\",\"monsterID\":\"%s\",\"monsterName\":\"%s\"}", monsterID, monsterName);

            // Respond to the portal
            mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", response);
        } else {
            // Respond with 404 for other endpoints
            mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Not Found\n");
        }
    }
}

int server_main(void) {
    struct mg_mgr mgr;
    struct mg_connection *c;

    // Initialize the Mongoose manager
    mg_mgr_init(&mgr);
    printf("Starting HTTP server on http://localhost:8000\n");

    // Create the server
    c = mg_http_listen(&mgr, "http://0.0.0.0:8000", ev_handler, NULL);
    if (c == NULL) {
        printf("Error starting server!\n");
        return 1;
    }

    // Run the event loop
    for (;;) mg_mgr_poll(&mgr, 1000);

    // Cleanup
    mg_mgr_free(&mgr);
    return 0;
}
