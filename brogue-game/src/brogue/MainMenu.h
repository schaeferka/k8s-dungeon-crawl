#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <pthread.h>  // Include pthread for the mutex
#include "portal-player.h"
#include "portal-monster.h"
#include "Rogue.h"

// Declaration of the function to start the metrics thread if needed
void start_metrics_thread_if_needed(void);

// Declaration of the function to sleep for a specified number of microseconds
void sleep_for_microseconds(int microseconds);

// Optionally, declare any shared variables like the mutex if it's needed outside this file
extern pthread_mutex_t metrics_thread_mutex;  // Make the mutex accessible if needed for other functions

void *metrics_update_loop(void *arg);

// Declare the metrics thread
static pthread_t metrics_thread;

extern void cleanup_game_resources();

#endif // MAIN_MENU_H