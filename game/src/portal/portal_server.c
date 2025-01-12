#include "mongoose.h"
#include "portal_server.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void ev_handler(struct mg_connection *c, int ev, void *ev_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;

        if (mg_match(hm->uri, mg_str("/monsters/admin-kill"), NULL))
        {
            int monsterID = 0;
            char monsterName[128] = {0};

            // Parse JSON payload manually
            const char *json = hm->body.buf;
            const char *id_start = strstr(json, "\"monsterID\":");
            const char *name_start = strstr(json, "\"monsterName\":");

            if (id_start && name_start)
            {
                // Parse monsterID
                sscanf(id_start, "\"monsterID\":%d", &monsterID);

                // Parse monsterName
                name_start = strchr(name_start, ':'); // Find the colon after "monsterName"
                if (name_start)
                {
                    name_start++; // Move past the colon
                    while (*name_start == ' ' || *name_start == '\"') name_start++; // Skip spaces and quotes

                    const char *name_end = strchr(name_start, '\"'); // Find the closing quote
                    if (name_end)
                    {
                        size_t name_length = name_end - name_start;
                        if (name_length < sizeof(monsterName))
                        {
                            strncpy(monsterName, name_start, name_length);
                            monsterName[name_length] = '\0'; // Null-terminate the string
                        }
                    }
                }

                printf("Extracted monsterID: %d, monsterName: %s\n", monsterID, monsterName);

                // Construct the response JSON
                char response[256];
                snprintf(response, sizeof(response), "{\"status\":\"success\",\"monsterID\":%d,\"monsterName\":\"%s\"}", monsterID, monsterName);
                
                // Respond to the portal
                mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", response);
            }
            else
            {
                // Invalid JSON payload
                mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\":\"Invalid JSON payload\"}");
            }
        }
        else
        {
            // Respond with 404 for other endpoints
            mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Not Found\n");
        }
    }
}

int server_main(void)
{
    struct mg_mgr mgr;
    struct mg_connection *c;

    // Initialize the Mongoose manager
    mg_mgr_init(&mgr);
    printf("Starting HTTP server on http://localhost:8000\n");

    // Create the server
    c = mg_http_listen(&mgr, "http://0.0.0.0:8000", ev_handler, NULL);
    if (c == NULL)
    {
        printf("Error starting server!\n");
        return 1;
    }

    // Run the event loop
    for (;;)
        mg_mgr_poll(&mgr, 1000);

    // Cleanup
    mg_mgr_free(&mgr);
    return 0;
}
