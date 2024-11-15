// K8S: Declares the functions to send metrics data to the Portal

#ifndef METRICS_SENDER_H
#define METRICS_SENDER_H

// Declaration of the function to send metrics data to the Portal
void send_metrics_to_portal(int gold, int depth, int hp, int turns);

// Declare the update_metrics function
void update_metrics();

#endif // METRICS_SENDER_H