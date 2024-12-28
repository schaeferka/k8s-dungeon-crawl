#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "portal_items.h"
#include "portal_items_helpers.h"
#include "portal.h"
#include "portal_urls.h"

#define BUFFER_SIZE 16384
#define PACK_CAPACITY 26

// Define inventory items
InventoryItem inventoryItems[PACK_CAPACITY] = {0};

// Declare previous inventory items
static item previous_inventory_items[PACK_CAPACITY] = {0};

// Declare previous equipped items
static EquippedItems previous_equipped_items = {0};

/**
 * @brief Initiate the updating of all items
 * 
 * This function is responsible for updating all player items.
 */
void update_items(void) {
    send_equipped_items_to_portal();
    send_pack_items_to_portal();
}

/** 
 * @brief Check if the equipped items have changed since the last update.
 *
 * This function compares the current equipped items with the previously sent ones
 * to detect changes.
 *
 * @return true if there are changes, false otherwise.
 */
static bool is_equipped_items_changed(void) {
    return (memcmp(&rogue.weapon, &previous_equipped_items.weapon, sizeof(item)) != 0 ||
            memcmp(&rogue.armor, &previous_equipped_items.armor, sizeof(item)) != 0 ||
            memcmp(&rogue.ringLeft, &previous_equipped_items.ringLeft, sizeof(item)) != 0 ||
            memcmp(&rogue.ringRight, &previous_equipped_items.ringRight, sizeof(item)) != 0);
}

/**
 * @brief Updates equipped items and sends them to the portal if necessary.
 */
void send_equipped_items_to_portal(void) {
    if (!rogue.gameHasEnded) {
        EquippedItems items = {
            .weapon = (rogue.weapon != NULL) ? *rogue.weapon : (item){0},
            .armor = (rogue.armor != NULL) ? *rogue.armor : (item){0},
            .ringLeft = (rogue.ringLeft != NULL) ? *rogue.ringLeft : (item){0},
            .ringRight = (rogue.ringRight != NULL) ? *rogue.ringRight : (item){0},
        };

        // Generate and send equipped items JSON
        char equipped_json[BUFFER_SIZE];
        generate_equipped_items_json(&items, equipped_json, sizeof(equipped_json));

        //printf("EQUIPPED Sending equipped items to portal...\n\n");
        //printf("%s\n\n", equipped_json);
        send_items_to_portal(equipped_json);

        // Update previous equipped items
        previous_equipped_items = items;
    }
}

/**
 * @brief Generates the JSON string for the equipped items.
 */
void generate_equipped_items_json(const EquippedItems *items, char *buffer, size_t size) {
    const itemTable *weaponDetails = getWeaponDetails(items->weapon.kind);
    const char *weaponDescription = weaponDetails ? weaponDetails->description : "No description available";
    
    const itemTable *armorDetails = getArmorDetails(items->armor.kind);
    const char *armorDescription = armorDetails ? armorDetails->description : "No description available";

    const itemTable *ringLeftDetails = getRingDetails(items->ringLeft.kind);
    const char *ringLeftDescription = ringLeftDetails ? ringLeftDetails->description : "No description available";
    
    const itemTable *ringRightDetails = getRingDetails(items->ringRight.kind);
    const char *ringRightDescription = ringRightDetails ? ringRightDetails->description : "No description available";

    snprintf(buffer, size, 
        "{"
        "\"weapon\": {"
            "\"category\": \"%s\", "
            "\"kind\": \"%s\", "
            "\"damage\": {\"min\": %d, \"max\": %d}, "
            "\"enchant1\": \"%hd\", "
            "\"enchant2\": \"%hd\", "
            "\"charges\": %d, "
            "\"times_enchanted\": %d, "
            "\"strength_required\": %d, "
            "\"inventory_letter\": \"%c\", "
            "\"inscription\": \"%s\", "
            "\"quantity\": %d, "
            "\"description\": \"%s\""
        "}, "
        "\"armor\": {"
            "\"category\": \"%s\", "
            "\"kind\": \"%s\", "
            "\"armor\": %d, "
            "\"enchant1\": \"%hd\", "
            "\"charges\": %d, "
            "\"times_enchanted\": %d, "
            "\"strength_required\": %d, "
            "\"inventory_letter\": \"%c\", "
            "\"inscription\": \"%s\", "
            "\"quantity\": %d, "
            "\"description\": \"%s\""
        "}, "
        "\"left_ring\": {"
            "\"category\": \"%s\", "
            "\"kind\": \"%s\", "
            "\"quantity\": %d, "
            "\"enchant1\": \"%hd\", "
            "\"enchant2\": \"%hd\", "
            "\"origin_depth\": %d, "
            "\"times_enchanted\": %d, "
            "\"strength_required\": %d, "
            "\"description\": \"%s\""
        "}, "
        "\"right_ring\": {"
            "\"category\": \"%s\", "
            "\"kind\": \"%s\", "
            "\"quantity\": %d, "
            "\"enchant1\": \"%hd\", "
            "\"enchant2\": \"%hd\", "
            "\"origin_depth\": %d, "
            "\"times_enchanted\": %d, "
            "\"strength_required\": %d, "
            "\"description\": \"%s\""
        "}"
        "}",
        get_item_category(items->weapon.category),
        get_weapon_kind(items->weapon.kind),
        items->weapon.damage.lowerBound,
        items->weapon.damage.upperBound,
        items->weapon.enchant1,
        items->weapon.enchant2,
        items->weapon.charges,
        items->weapon.timesEnchanted,
        items->weapon.strengthRequired,
        items->weapon.inventoryLetter,
        items->weapon.inscription,
        items->weapon.quantity,
        weaponDescription,
        get_item_category(items->armor.category),
        get_armor_kind(items->armor.kind),
        items->armor.armor,
        items->armor.enchant1,
        items->armor.charges,
        items->armor.timesEnchanted,
        items->armor.strengthRequired,
        items->armor.inventoryLetter,
        items->armor.inscription,
        items->armor.quantity,
        armorDescription,
        get_item_category(items->ringLeft.category),
        get_ring_kind(items->ringLeft.kind),
        items->ringLeft.quantity,
        items->ringLeft.enchant1,
        items->ringLeft.enchant2,
        items->ringLeft.originDepth,
        items->ringLeft.timesEnchanted,
        items->ringLeft.strengthRequired,
        ringLeftDescription,
        get_item_category(items->ringRight.category),
        get_ring_kind(items->ringRight.kind),
        items->ringRight.quantity,
        items->ringRight.enchant1,
        items->ringRight.enchant2,
        items->ringRight.originDepth,
        items->ringRight.timesEnchanted,
        items->ringRight.strengthRequired,
        ringRightDescription
    );
}

/**
 * @brief Check if the inventory items have changed since the last update.
 */
static bool is_inventory_items_changed(void) {
    return (memcmp(inventoryItems, previous_inventory_items, sizeof(InventoryItem) * PACK_CAPACITY) != 0);
}

/**
 * @brief Sends the inventory items to the portal.
 */
void send_pack_items_to_portal(void) {
    if (!rogue.gameHasEnded) {
        size_t buffer_size = 16384;
        char *post_data = malloc(buffer_size);
        if (post_data == NULL) {
            fprintf(stderr, "Memory allocation failed!\n");
            return;
        }

        // Zero out the buffer to avoid leftover characters
        memset(post_data, 0, buffer_size);

        generate_pack_items_json(post_data + strlen(post_data), buffer_size - strlen(post_data));

        //printf("PACK Sending pack items to portal...\n\n");
        //printf("%s\n\n", post_data);
        send_pack_to_portal(post_data);

        // Update previous inventory items
        memcpy(previous_inventory_items, inventoryItems, sizeof(InventoryItem) * PACK_CAPACITY);

        free(post_data);
    }
}

/**
 * @brief Extracts item details and builds JSON for the player's inventory.
 */
void generate_pack_items_json(char *buffer, size_t buffer_size) {
    if (!rogue.gameHasEnded) {
        item *current_item = packItems;
        size_t offset = 0;

        // Start the JSON object and array
        offset += snprintf(buffer + offset, buffer_size - offset, "{\"pack\":[");

        while (current_item && offset < buffer_size - 1) {
            const char *item_description = "No description available";
            const char *category_name = get_item_category(current_item->category);
            const char *kind_name = "Unknown";

            // Fetch description based on item category
            switch (current_item->category) {
                case WEAPON: {
                    const itemTable *weaponDetails = getWeaponDetails(current_item->kind);
                    kind_name = get_weapon_kind(current_item->kind);
                    item_description = weaponDetails ? weaponDetails->description : "No description available";
                    break;
                }
                case ARMOR: {
                    const itemTable *armorDetails = getArmorDetails(current_item->kind);
                    kind_name = get_armor_kind(current_item->kind);
                    item_description = armorDetails ? armorDetails->description : "No description available";
                    break;
                }
                case RING: {
                    const itemTable *ringDetails = getRingDetails(current_item->kind);
                    kind_name = get_ring_kind(current_item->kind);
                    item_description = ringDetails ? ringDetails->description : "No description available";
                    break;
                }
                case FOOD:
                    kind_name = "Food";
                    break;
                case POTION: {
                    kind_name = get_potion_kind(current_item->kind);
                    break;
                }
                case SCROLL: {
                    kind_name = get_scroll_kind(current_item->kind);
                    break;
                }
                case STAFF: {
                    kind_name = get_staff_kind(current_item->kind);
                    break;
                }
                case WAND: {
                    kind_name = get_wand_kind(current_item->kind);
                    break;
                }
                case CHARM: {
                    kind_name = get_charm_kind(current_item->kind);
                    break;
                }
                case GOLD:
                    kind_name = "Gold";
                    break;
                case AMULET:
                    kind_name = "Amulet";
                    break;
                case GEM:
                    kind_name = "Gem";
                    break;
                case KEY:
                    kind_name = "Key";
                    break;
                default:
                    kind_name = "Unknown";
            }

            // Format item details as JSON
            if (current_item->inventoryLetter) {
                offset += snprintf(buffer + offset, buffer_size - offset,
                    "{ \"category\": \"%s\", \"kind\": \"%s\", \"description\": \"%s\", "
                    "\"quantity\": %d, \"armor\": %d, \"damage\": { \"min\": %d, \"max\": %d }, "
                    "\"inventory_letter\": \"%c\" }",
                    category_name, kind_name, item_description,
                    current_item->quantity, current_item->armor,
                    current_item->damage.lowerBound, current_item->damage.upperBound,
                    current_item->inventoryLetter);

                if (current_item->nextItem && offset < buffer_size - 2) {
                    offset += snprintf(buffer + offset, buffer_size - offset, ", ");
                }
            }

            current_item = current_item->nextItem;
        }

        // End the JSON array and object
        snprintf(buffer + offset, buffer_size - offset, "]}");
    }
}

/**
 * @brief Resets the items data.
 *
 * This function resets the items data.
 */
void reset_items(void) {
    // Reset the previous inventory and equipped items
    memset(previous_inventory_items, 0, sizeof(previous_inventory_items));
    memset(&previous_equipped_items, 0, sizeof(previous_equipped_items));
}
