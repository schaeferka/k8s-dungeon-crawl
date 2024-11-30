#ifndef PORTAL_ITEMS_H
#define PORTAL_ITEMS_H

#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal.h"

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
} InventoryItem;
extern InventoryItem inventoryItems[26];

/**
 * @brief Updates all items and sends them to the portal if necessary.
 */
extern void update_items(void);

/**
 * @brief Updates equipped items and sends them to the portal if necessary.
 */
extern void update_equipped_items(void);

/**
 * @brief Generates the JSON string for the equipped items.
 *
 * @param items The equipped items to be serialized into JSON.
 * @param buffer The buffer to store the resulting JSON string.
 * @param size The size of the buffer.
 */
extern void generate_equipped_items_json(const EquippedItems *items, char *buffer, size_t size);

/**
 * @brief Creates the JSON string for the player's inventory (pack items).
 *
 * @param buffer The buffer to store the resulting JSON string.
 * @param buffer_size The size of the buffer.
 */
extern void generate_pack_items_json(char *buffer, size_t buffer_size);

/**
 * @brief Sends the pack items to the portal.
 */
extern void send_pack_items_to_portal(void);

/**
 * @brief Sends the equipped items data to the portal.
 */
extern void send_equipped_items_to_portal(void);

extern void reset_items(void);

#endif // PORTAL_ITEMS_H
