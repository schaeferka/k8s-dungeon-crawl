// portal-items.c

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal-items.h"

#define ITEM_PORTAL_URL "http://portal-service.portal:5000/items"

// Function to send item information to the portal
void update_items(void) {
    ItemMetrics items = {
        .weapon = (rogue.weapon != NULL) ? *rogue.weapon : (item){0},
        .armor = (rogue.armor != NULL) ? *rogue.armor : (item){0},
        .ringLeft = (rogue.ringLeft != NULL) ? *rogue.ringLeft : (item){0},
        .ringRight = (rogue.ringRight != NULL) ? *rogue.ringRight : (item){0},
    };
    send_item_metrics_to_portal(&items);
}

// Function to send item metrics to the portal
void send_item_metrics_to_portal(const ItemMetrics *items) {
    CURL *curl = curl_easy_init();
    if (curl) {
        char post_data[1024];
        
        snprintf(post_data, sizeof(post_data),
            "{\"weapon\": {\"category\": \"%s\", \"kind\": %d, \"damageMin\": %d, \"damageMax\": %d, \"strengthRequired\": %d}, "
            "\"armor\": {\"category\": \"%s\", \"kind\": %d, \"defense\": %d, \"strengthRequired\": %d}, "
            "\"ringLeft\": {\"category\": \"%s\", \"kind\": %d}, "
            "\"ringRight\": {\"category\": \"%s\", \"kind\": %d}}",
            getItemCategory(items->weapon.category), items->weapon.kind, items->weapon.damage.lowerBound, items->weapon.damage.upperBound, items->weapon.strengthRequired,
            getItemCategory(items->armor.category), items->armor.kind, items->armor.armor, items->armor.strengthRequired,
            getItemCategory(items->ringLeft.category), items->ringLeft.kind,
            getItemCategory(items->ringRight.category), items->ringRight.kind);

        // Set up CURL options
        curl_easy_setopt(curl, CURLOPT_URL, ITEM_PORTAL_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        // Set headers for JSON content
        struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Send the request and handle errors
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send item data: %s\n", curl_easy_strerror(res));
        } else {
            printf("Successfully sent item data to portal.\n");
        }

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "CURL initialization failed.\n");
    }
}

const char *getItemCategoryName(unsigned short category) {
    if (category < NUMBER_ITEM_CATEGORIES) {
        return itemCategoryNames[category];
    }
    return "Unknown";
}

const char itemCategories[NUMBER_ITEM_CATEGORIES][11] = {
    "food", "weapon", "armor", "potion", "scroll", "staff", "wand", "ring",
   "charm", "gold", "amulet", "gem", "key"
};

const char *getItemCategory(unsigned short category) {
    if (category < NUMBER_ITEM_CATEGORIES) {
        return itemCategories[category];
    }
    return "Unknown Category";
}
