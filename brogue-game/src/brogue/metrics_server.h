#ifndef METRICS_SERVER_H
#define METRICS_SERVER_H

// Include any necessary headers that metrics_server.c uses
#include <microhttpd.h>

// Declaration of the function to start the metrics server
void start_metrics_server();

// Declare the update_metrics function
void update_metrics();

#endif // METRICS_SERVER_H
