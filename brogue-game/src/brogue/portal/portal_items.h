// portal-items.h

#ifndef PORTAL_ITEMS_H
#define PORTAL_ITEMS_H

#include "Rogue.h"

typedef struct {
    item weapon;
    item armor;
    item ringLeft;
    item ringRight;
} EquippedItems;
extern EquippedItems equippedItems;

typedef struct {
    unsigned short category;
    short kind;
    unsigned long flags;
    randomRange damage;
    short armor;
    short charges;
    short enchant1;
    short enchant2;
    short timesEnchanted;
    short strengthRequired;
    unsigned short quiverNumber;
    short quantity;
    char inventoryLetter;
    char inscription[DCOLS];
} PackItem;
extern PackItem packItem;

// Function declarations
extern void update_items(void);
void send_equipped_items_to_portal(const EquippedItems *items);
void send_pack_items_to_portal(void);

void create_pack_items_json(char *buffer, size_t buffer_size);

#endif // PORTAL_ITEMS_H
