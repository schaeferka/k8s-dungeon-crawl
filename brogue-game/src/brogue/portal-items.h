// portal-items.h

#ifndef PORTAL_ITEMS_H
#define PORTAL_ITEMS_H

#include "Rogue.h"

// Structure to hold item-related metrics
typedef struct {
    item weapon;
    item armor;
    item ringLeft;
    item ringRight;
} ItemMetrics;

// Function declarations
void update_items(void);
void send_item_metrics_to_portal(const ItemMetrics *items);

// Utility function to get item category name
const char *getItemCategory(unsigned short category);

#endif // PORTAL_ITEMS_H
