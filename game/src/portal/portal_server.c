#include "mongoose.h"
#include <string.h>
#include <stdio.h>

static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "{%m:%m}\n",  MG_ESC("monsterName"), "monstie");
        // Check if the request is for /monster-deleted
        if (mg_match(hm->uri, mg_str("/monster-deleted"), NULL)) {
            char *monsterName = mg_json_get_str(hm->body, "$.monsterName");

            if (monsterName == NULL) {
                mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "No monsterName\n", (int) c->recv.len);
            } else {  
                mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "{%m:%m}\n",  MG_ESC("monsterName"), monsterName);
            }
        } else {
            // Respond with 404 for other endpoints
            mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Not Found\n");
        }
    } else {
        mg_http_reply(c, 500, "Content-Type: text/plain\r\n", "Internal Server Error\n");
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
