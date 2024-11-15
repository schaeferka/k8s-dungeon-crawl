// portal-gamestate.h

#ifndef PORTAL_GAMESTATS_H
#define PORTAL_GAMESTATS_H

#include "Rogue.h"
#include "stdbool.h"
#include "MainMenu.h"

// Function declarations
extern void update_gamestats(void);

void extractGameStatsJSON(const gameStats *stats, char *buffer, size_t buffer_size);

void send_gamestats_metrics_to_portal(const char *post_data);

#endif // PORTAL_GAMESTATS_H
