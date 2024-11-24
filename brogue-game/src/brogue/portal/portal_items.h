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

const char* getWeaponKindName(short kind);
const char* getCategoryName(unsigned short category);
const char* getWeaponEnchantName(short enchant);
const char* getArmorKindName(short kind);
const char* getArmorEnchantName(short enchant);
const char* getPotionKindName(short kind);
const char* getWandKindName(short kind);
const char* getStaffKindName(short kind);
const char* getRingKindName(short kind);
const char* getCharmKindName(short kind);
const char* getScrollKindName(short kind);
void send_pack_to_portal(void);

void escape_json_string(const char *src, char *dest, size_t dest_size);


// Function declarations
extern void update_items(void);
void send_items_to_portal(const ItemMetrics *items);


// Declaration of utility functions
const itemTable *getWeaponDetails(short kind);
const itemTable *getArmorDetails(short kind);
const itemTable *getRingDetails(short kind);
extern const char *getItemCategoryName(unsigned short category);
const char *getItemCategory(unsigned short category);

void extract_inventory_json(char *buffer, size_t buffer_size);

#endif // PORTAL_ITEMS_H
