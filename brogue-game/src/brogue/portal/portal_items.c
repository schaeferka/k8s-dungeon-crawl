// portal-items.c

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal_items.h"
#include "portal_items_helpers.h"

#define ITEM_PORTAL_URL "http://portal-service.portal:5000/items/update"
#define PACK_PORTAL_URL "http://portal-service.portal:5000/pack/update"
#define BUFFER_SIZE 8192 

// Function to send item information to the portal
void update_equipped_items(void) {
    EquippedItems items = {
        .weapon = (rogue.weapon != NULL) ? *rogue.weapon : (item){0},
        .armor = (rogue.armor != NULL) ? *rogue.armor : (item){0},
        .ringLeft = (rogue.ringLeft != NULL) ? *rogue.ringLeft : (item){0},
        .ringRight = (rogue.ringRight != NULL) ? *rogue.ringRight : (item){0},
    };
    
    send_equipped_items_to_portal(&items);
    send_pack_items_to_portal();
}

void send_equipped_items_to_portal(const EquippedItems *items) {
    if (!rogue.gameHasEnded) {
        CURL *curl = curl_easy_init();
        if (curl) {
            char weaponInscriptionEscaped[256];
            char armorInscriptionEscaped[256];

            // Escape inscriptions
            if (rogue.weapon && rogue.weapon->inscription[0]) {
                escape_json_string(rogue.weapon->inscription, weaponInscriptionEscaped, sizeof(weaponInscriptionEscaped));
            } else {
                strcpy(weaponInscriptionEscaped, "None");
            }

            if (rogue.armor && rogue.armor->inscription[0]) {
                escape_json_string(rogue.armor->inscription, armorInscriptionEscaped, sizeof(armorInscriptionEscaped));
            } else {
                strcpy(armorInscriptionEscaped, "None");
            }

            char post_data[2048];
            snprintf(post_data, sizeof(post_data),
                "{"
                "\"weapon\": {"
                    "\"category\": \"%s\", "
                    "\"kind\": \"%s\", "
                    "\"damage\": {\"min\": %d, \"max\": %d}, "
                    "\"enchant1\": \"%s\", "
                    "\"enchant2\": \"%s\", "
                    "\"charges\": %d, "
                    "\"timesEnchanted\": %d, "
                    "\"strengthRequired\": %d, "
                    "\"inventoryLetter\": \"%c\", "
                    "\"inscription\": \"%s\", "
                    "\"quantity\": %d"
                "}, "
                "\"armor\": {"
                    "\"category\": \"%s\", "
                    "\"kind\": \"%s\", "
                    "\"armor\": %d, "
                    "\"enchant1\": \"%s\", "
                    "\"charges\": %d, "
                    "\"timesEnchanted\": %d, "
                    "\"strengthRequired\": %d, "
                    "\"inventoryLetter\": \"%c\", "
                    "\"inscription\": \"%s\", "
                    "\"quantity\": %d"
                "}, "
                "\"ringLeft\": {"
                    "\"category\": \"%s\", "
                    "\"kind\": \"%s\", "
                    "\"quantity\": %d"
                "}, "
                "\"ringRight\": {"
                    "\"category\": \"%s\", "
                    "\"kind\": \"%s\", "
                    "\"quantity\": %d"
                "}"
                "}",
                // Weapon data
                getCategoryName(rogue.weapon ? rogue.weapon->category : 0), 
                getWeaponKindName(rogue.weapon ? rogue.weapon->kind : 0),
                rogue.weapon ? rogue.weapon->damage.lowerBound : 0,
                rogue.weapon ? rogue.weapon->damage.upperBound : 0,
                getWeaponEnchantName(rogue.weapon ? rogue.weapon->enchant1 : 0),
                getWeaponEnchantName(rogue.weapon ? rogue.weapon->enchant2 : 0),
                rogue.weapon ? rogue.weapon->charges : 0,
                rogue.weapon ? rogue.weapon->timesEnchanted : 0,
                rogue.weapon ? rogue.weapon->strengthRequired : 0,
                rogue.weapon ? rogue.weapon->inventoryLetter : ' ',
                weaponInscriptionEscaped,
                rogue.weapon ? rogue.weapon->quantity : 1,
                // Armor data
                getCategoryName(rogue.armor ? rogue.armor->category : 0), 
                getArmorKindName(rogue.armor ? rogue.armor->kind : 0),
                rogue.armor ? rogue.armor->armor : 0,
                getArmorEnchantName(rogue.armor ? rogue.armor->enchant1 : 0),
                rogue.armor ? rogue.armor->charges : 0,
                rogue.armor ? rogue.armor->timesEnchanted : 0,
                rogue.armor ? rogue.armor->strengthRequired : 0,
                rogue.armor ? rogue.armor->inventoryLetter : ' ',
                armorInscriptionEscaped,
                rogue.armor ? rogue.armor->quantity : 1,
                // RingLeft data
                getCategoryName(rogue.ringLeft ? rogue.ringLeft->category : 0), 
                getRingKindName(rogue.ringLeft ? rogue.ringLeft->kind : 0),
                rogue.ringLeft ? rogue.ringLeft->quantity : 1,
                // RingRight data
                getCategoryName(rogue.ringRight ? rogue.ringRight->category : 0), 
                getRingKindName(rogue.ringRight ? rogue.ringRight->kind : 0),
                rogue.ringRight ? rogue.ringRight->quantity : 1
            );


            curl_easy_setopt(curl, CURLOPT_URL, ITEM_PORTAL_URL);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

            struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Failed to send items: %s\n", curl_easy_strerror(res));
            } else {
                printf("Successfully sent equipped items to portal: %s\n", post_data);
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        } else {
            fprintf(stderr, "CURL initialization failed.\n");
        }
    }
}


void send_pack_items_to_portal() {
    if (!rogue.gameHasEnded) {
        CURL *curl = curl_easy_init();
        if (curl) {
            char post_data[BUFFER_SIZE];
            snprintf(post_data, sizeof(post_data), "{\"pack\":");

            // Extract inventory JSON and append it to post_data
            extract_inventory_json(post_data + strlen(post_data), sizeof(post_data) - strlen(post_data));

            // Close JSON object
            strncat(post_data, "}", sizeof(post_data) - strlen(post_data) - 1);

            // Configure CURL options
            curl_easy_setopt(curl, CURLOPT_URL, PACK_PORTAL_URL);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

            struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            // Send data and check for errors
            printf("Sending pack data to portal...\n");
            printf("Data: %s\n", post_data);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Failed to send pack data: %s\n", curl_easy_strerror(res));
            } else {
                printf("Successfully sent pack data to portal.\n");
            }

            // Clean up
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        } else {
            fprintf(stderr, "CURL initialization failed.\n");
        }
    }
}

// Function to extract item details and build JSON for the player's inventory
void create_pack_items_json(char *buffer, size_t buffer_size) {
    if (!rogue.gameHasEnded) {
    item *currentItem = packItems;
    size_t offset = 0;

    offset += snprintf(buffer + offset, buffer_size - offset, "[");

    while (currentItem && offset < buffer_size - 1) {
        const char *itemName = "Unknown";
        const char *itemDescription = "No description available";
        const char *categoryName = getCategoryName(currentItem->category);
        char *kindName = "Unknown";

       switch (currentItem->category) {
            case WEAPON:
                kindName = getWeaponKindName(currentItem->kind);
                break;
            case ARMOR:
                kindName = getArmorKindName(currentItem->kind);
                break;
            case RING:
                kindName = getRingKindName(currentItem->kind);
                break;
            case FOOD:
                kindName = "Unknown" ? kindName = "Some Food" : kindName;
                break;
            case POTION:
                kindName = getPotionKindName(currentItem->kind);
                break;
            case SCROLL:
                kindName = getScrollKindName(currentItem->kind);
                break;
            case STAFF:
                kindName = getStaffKindName(currentItem->kind);
                break;
            case WAND:
                kindName = getWandKindName(currentItem->kind);
                break;
            case CHARM:
                kindName = getCharmKindName(currentItem->kind);
                break;
            case GOLD:
                kindName = "Gold";
                break;
            case AMULET:
                kindName = "Amulet";
                break;
            case GEM:
                kindName = "Gem";
                break;
            case KEY:
                kindName = "Key";
                break;
        }

        // Format item details as JSON
        if (currentItem->inventoryLetter) {
                        offset += snprintf(buffer + offset, buffer_size - offset,
                       "{ \"category\": \"%s\", \"name\": \"%s\", \"description\": \"%s\", "
                       "\"quantity\": %d, \"armor\": %d, \"damage\": { \"min\": %d, \"max\": %d }, "
                       "\"inventoryLetter\": \"%c\" }",
                       categoryName, kindName, itemDescription,
                       currentItem->quantity, currentItem->armor,
                       currentItem->damage.lowerBound, currentItem->damage.upperBound,
                       currentItem->inventoryLetter);
       
            

        if (currentItem->nextItem && offset < buffer_size - 2) {
            offset += snprintf(buffer + offset, buffer_size - offset, ", ");
        }
         }

        currentItem = currentItem->nextItem;
    }
    snprintf(buffer + offset, buffer_size - offset, "]");

    printf("Inventory JSON: %s\n", buffer);
    }
} 