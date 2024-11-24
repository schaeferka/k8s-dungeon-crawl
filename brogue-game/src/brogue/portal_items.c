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

// Declare previous pack items
static item previous_pack_items[PACK_CAPACITY] = {0};

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
    // Compare equipped items with previously sent items
    return (memcmp(&rogue.weapon, &previous_equipped_items.weapon, sizeof(item)) != 0 ||
            memcmp(&rogue.armor, &previous_equipped_items.armor, sizeof(item)) != 0 ||
            memcmp(&rogue.ringLeft, &previous_equipped_items.ringLeft, sizeof(item)) != 0 ||
            memcmp(&rogue.ringRight, &previous_equipped_items.ringRight, sizeof(item)) != 0);
}

/**
 * @brief Updates equipped items and sends them to the portal if necessary.
 *
 * This function checks if the equipped items have changed, generates the JSON string,
 * and sends the data to the portal if necessary.
 */
void send_equipped_items_to_portal(void) {
    if (!rogue.gameHasEnded && is_equipped_items_changed()) {
        EquippedItems items = {
            .weapon = (rogue.weapon != NULL) ? *rogue.weapon : (item){0},
            .armor = (rogue.armor != NULL) ? *rogue.armor : (item){0},
            .ringLeft = (rogue.ringLeft != NULL) ? *rogue.ringLeft : (item){0},
            .ringRight = (rogue.ringRight != NULL) ? *rogue.ringRight : (item){0},
        };
        
        // Generate and send equipped items JSON
        char equipped_json[BUFFER_SIZE];
        generate_equipped_items_json(&items, equipped_json, sizeof(equipped_json));
        send_items_to_portal(equipped_json);

        // Update previous equipped items
        previous_equipped_items = items;
    }
}

/**
 * @brief Generates the JSON string for the equipped items.
 *
 * This function generates a JSON string based on the equipped items data.
 *
 * @param items The equipped items to be serialized into JSON.
 * @param buffer The buffer to store the resulting JSON string.
 * @param size The size of the buffer.
 */
void generate_equipped_items_json(const EquippedItems *items, char *buffer, size_t size) {
    // Fetch item details (with descriptions) for each item
    const itemTable *weaponDetails = getWeaponDetails(items->weapon.kind);
    const char *weaponDescription = weaponDetails ? weaponDetails->description : "No description available";
    
    const itemTable *armorDetails = getArmorDetails(items->armor.kind);
    const char *armorDescription = armorDetails ? armorDetails->description : "No description available";

    const itemTable *ringLeftDetails = getRingDetails(items->ringLeft.kind);
    const char *ringLeftDescription = ringLeftDetails ? ringLeftDetails->description : "No description available";
    
    const itemTable *ringRightDetails = getRingDetails(items->ringRight.kind);
    const char *ringRightDescription = ringRightDetails ? ringRightDetails->description : "No description available";

    // Ensure the buffer is large enough
    snprintf(buffer, size, 
        "{"
        "\"weapon\": {"
            "\"category\": \"%s\", "
            "\"kind\": \"%s\", "
            "\"damage\": {\"min\": %d, \"max\": %d}, "
            "\"enchant1\": \"%hd\", "
            "\"enchant2\": \"%hd\", "
            "\"charges\": %d, "
            "\"timesEnchanted\": %d, "
            "\"strengthRequired\": %d, "
            "\"inventoryLetter\": \"%c\", "
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
            "\"timesEnchanted\": %d, "
            "\"strengthRequired\": %d, "
            "\"inventoryLetter\": \"%c\", "
            "\"inscription\": \"%s\", "
            "\"quantity\": %d, "
            "\"description\": \"%s\""
        "}, "
        "\"ringLeft\": {"
            "\"category\": \"%s\", "
            "\"kind\": \"%s\", "
            "\"quantity\": %d, "
            "\"enchant1\": \"%hd\", "
            "\"enchant2\": \"%hd\", "
            "\"originDepth\": %d, "
            "\"timesEnchanted\": %d, "
            "\"strengthRequired\": %d, "
            "\"description\": \"%s\""
        "}, "
        "\"ringRight\": {"
            "\"category\": \"%s\", "
            "\"kind\": \"%s\", "
            "\"quantity\": %d, "
            "\"enchant1\": \"%hd\", "
            "\"enchant2\": \"%hd\", "
            "\"originDepth\": %d, "
            "\"timesEnchanted\": %d, "
            "\"strengthRequired\": %d, "
            "\"description\": \"%s\""
        "}"
        "}",
        // Weapon data
        get_item_category(items->weapon.category),
        get_weapon_kind(items->weapon.kind),
        items->weapon.damage.lowerBound,
        items->weapon.damage.upperBound,
        items->weapon.enchant1,  // Use %hd for short
        items->weapon.enchant2,  // Use %hd for short
        items->weapon.charges,
        items->weapon.timesEnchanted,
        items->weapon.strengthRequired,
        items->weapon.inventoryLetter,
        items->weapon.inscription,
        items->weapon.quantity,
        weaponDescription,  // Include description from itemTable

        // Armor data
        get_item_category(items->armor.category),
        get_armor_kind(items->armor.kind),
        items->armor.armor,
        items->armor.enchant1,  // Use %hd for short
        items->armor.charges,
        items->armor.timesEnchanted,
        items->armor.strengthRequired,
        items->armor.inventoryLetter,
        items->armor.inscription,
        items->armor.quantity,
        armorDescription,  // Include description from itemTable

        // RingLeft data
        get_item_category(items->ringLeft.category),
        get_ring_kind(items->ringLeft.kind),
        items->ringLeft.quantity,
        items->ringLeft.enchant1,  // Use %hd for short
        items->ringLeft.enchant2,  // Use %hd for short
        items->ringLeft.originDepth,
        items->ringLeft.timesEnchanted,
        items->ringLeft.strengthRequired,
        ringLeftDescription,  // Include description from itemTable

        // RingRight data
        get_item_category(items->ringRight.category),
        get_ring_kind(items->ringRight.kind),
        items->ringRight.quantity,
        items->ringRight.enchant1,  // Use %hd for short
        items->ringRight.enchant2,  // Use %hd for short
        items->ringRight.originDepth,
        items->ringRight.timesEnchanted,
        items->ringRight.strengthRequired,
        ringRightDescription  // Include description from itemTable
    );
}


/**
 * @brief Check if the pack items have changed since the last update.
 *
 * This function compares the current pack items with the previously sent ones
 * to detect changes.
 *
 * @return true if there are changes, false otherwise.
 */
static bool is_pack_items_changed(void) {
    // Compare pack items with previously sent items
    return (memcmp(packItems, previous_pack_items, sizeof(item)) != 0);
}

/**
 * @brief Sends the pack items to the portal.
 *
 * This function generates the JSON string for the pack items and sends it to the portal.
 */
void send_pack_items_to_portal(void) {
    if (!rogue.gameHasEnded && is_pack_items_changed()) {
        // Dynamically allocate memory for the JSON string
        size_t buffer_size = 16384;
        char *post_data = malloc(buffer_size);
        if (post_data == NULL) {
            fprintf(stderr, "Memory allocation failed!\n");
            return;
        }

        snprintf(post_data, sizeof(post_data), "{\"pack\":");

        // Generate and append the inventory JSON
        generate_pack_items_json(post_data + strlen(post_data), sizeof(post_data) - strlen(post_data));

        // Close JSON object
        strncat(post_data, "}", sizeof(post_data) - strlen(post_data) - 1);

        // Send data to portal using the new portal function
        send_pack_to_portal(post_data);

        // Update previous pack items
        memcpy(previous_pack_items, packItems, sizeof(item) * PACK_CAPACITY);

        // Free the dynamically allocated memory
        free(post_data);
    }
}

/**
 * @brief Extracts item details and builds JSON for the player's inventory.
 *
 * This function iterates over the player's inventory and generates a JSON string 
 * representing all items in the inventory.
 *
 * @param buffer A buffer to store the generated JSON string.
 * @param buffer_size The size of the buffer.
 */
void generate_pack_items_json(char *buffer, size_t buffer_size) {
    if (!rogue.gameHasEnded) {
        item *current_item = packItems;
        size_t offset = 0;

        offset += snprintf(buffer + offset, buffer_size - offset, "[");

        while (current_item && offset < buffer_size - 1) {
            const char *item_description = "No description available";
            const char *category_name = get_item_category(current_item->category);
            const char *kind_name = "Unknown";

            // Use category-based name retrieval function
            switch (current_item->category) {
                case WEAPON:
                    kind_name = get_weapon_kind(current_item->kind);
                    break;
                case ARMOR:
                    kind_name = get_armor_kind(current_item->kind);
                    break;
                case RING:
                    kind_name = get_ring_kind(current_item->kind);
                    break;
                case FOOD:
                    kind_name = "Food"; // Modify as necessary
                    break;
                case POTION:
                    kind_name = get_potion_kind(current_item->kind);
                    break;
                case SCROLL:
                    kind_name = get_scroll_kind(current_item->kind);
                    break;
                case STAFF:
                    kind_name = get_staff_kind(current_item->kind);
                    break;
                case WAND:
                    kind_name = get_wand_kind(current_item->kind);
                    break;
                case CHARM:
                    kind_name = get_charm_kind(current_item->kind);
                    break;
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
                    "{ \"category\": \"%s\", \"name\": \"%s\", \"description\": \"%s\", "
                    "\"quantity\": %d, \"armor\": %d, \"damage\": { \"min\": %d, \"max\": %d }, "
                    "\"inventoryLetter\": \"%c\" }",
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
        snprintf(buffer + offset, buffer_size - offset, "]");

        printf("Inventory JSON: %s\n", buffer);
    }
}
