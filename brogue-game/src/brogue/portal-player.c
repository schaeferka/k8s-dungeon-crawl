#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal-player.h"

// Define the Portal URL
#define PORTAL_URL "http://portal-service.portal:5000/metrics"

//void free_pack_items() {
//    item *current = packItems;
//    while (current) {
//        item *next = current->nextItem;
//        free(current);
//        current = next;
//    }
//    packItems = NULL;  // Reset to NULL to prevent dangling pointer
//}

const char itemCategories[NUMBER_ITEM_CATEGORIES][11] = {
    "food", "weapon", "armor", "potion", "scroll", "staff", "wand", "ring",
   "charm", "gold", "amulet", "gem", "key"
};


const itemTable *getWeaponDetails(short kind) {
    if (kind >= 0 && kind < NUMBER_WEAPON_KINDS) {
        return &weaponTable[kind];
    }
    return NULL;
}

const itemTable *getArmorDetails(short kind) {
    if (kind >= 0 && kind < NUMBER_ARMOR_KINDS) {
        return &armorTable[kind];
    }
    return NULL;
}

const itemTable *getRingDetails(short kind) {
    if (kind >= 0 && kind < NUMBER_RING_KINDS) {
        return &ringTable[kind];
    }
    return NULL;
}

// Now, you can access itemCategoryNames directly
const char *getItemCategory(unsigned short category) {
    if (category < NUMBER_ITEM_CATEGORIES) {
        return itemCategories[category];
    }
    return "Unknown Category";
}

void update_metrics(void) {
    if (!rogue.gameHasEnded) {
    // Create and populate an instance of PlayerMetrics
    Metrics metrics = {
        .gold = rogue.gold,
        .goldGenerated = rogue.goldGenerated,
        .depthLevel = rogue.depthLevel,
        .deepestLevel = rogue.deepestLevel,
        .currentHP = player.currentHP,
        .maxHP = player.info.maxHP,
        .turns = rogue.absoluteTurnNumber,
        .strength = rogue.strength,
        .monsterSpawnFuse = rogue.monsterSpawnFuse,
        .playerTurnNumber = rogue.playerTurnNumber,
        .absoluteTurnNumber = rogue.absoluteTurnNumber,
        .milliseconds = rogue.milliseconds,
        .xpxpThisTurn = rogue.xpxpThisTurn,
        .stealthRange = rogue.stealthRange,
        .rewardRoomsGenerated = rogue.rewardRoomsGenerated,

        .wizard = rogue.wizard,
        .disturbed = rogue.disturbed,
        .gameInProgress = rogue.gameInProgress,
        .gameHasEnded = rogue.gameHasEnded,
        .easyMode = rogue.easyMode,

        .seed = rogue.seed,
        .RNG = rogue.RNG,
    
        // ring bonuses:
        .clairvoyance = rogue.clairvoyance,
        .stealthBonus = rogue.stealthBonus,
        .regenerationBonus = rogue.regenerationBonus,
        .lightMultiplier = rogue.lightMultiplier,
        .awarenessBonus = rogue.awarenessBonus,
        .transference = rogue.transference,         
        .wisdomBonus = rogue.wisdomBonus,
        .reaping = rogue.reaping, 
        
        .regenPerTurn = player.regenPerTurn,  
        .weaknessAmount = player.weaknessAmount,
        .poisonAmount = player.poisonAmount,       

        // Populate equipped items if they are present
        .weapon = (rogue.weapon != NULL) ? *rogue.weapon : (item){0},
        .armor = (rogue.armor != NULL) ? *rogue.armor : (item){0},
        .ringLeft = (rogue.ringLeft != NULL) ? *rogue.ringLeft : (item){0},
        .ringRight = (rogue.ringRight != NULL) ? *rogue.ringRight : (item){0},
         
    };

    // Log for debugging
    //printf("Updating metrics: gold=%d, depth=%d, currentHP=%d, maxHP=%d, turns=%d, wizard=%d, gameInProgress=%d\n",
    //       metrics.gold, metrics.depthLevel, metrics.currentHP, metrics.maxHP, metrics.turns, metrics.wizard, metrics.gameInProgress);

    // Send the metrics to the Portal server
    send_metrics_to_portal(&metrics);
}
}

void send_metrics_to_portal(const Metrics *metrics) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (!rogue.gameHasEnded) {
    if (curl) {
        char post_data[4096];
         char inventory_json[2048];

        extract_inventory_json(inventory_json, sizeof(inventory_json));


        // Fetch item details with default fallbacks for each equipped item
        const itemTable *weaponDetails = getWeaponDetails(metrics->weapon.kind);
        const char *weaponName = weaponDetails ? weaponDetails->name : "Unknown Weapon";
        const char *weaponDescription = weaponDetails ? weaponDetails->description : "No description available";
        int weaponStrengthRequired = weaponDetails ? weaponDetails->strengthRequired : 0;
        int weaponDamageMin = weaponDetails ? weaponDetails->range.lowerBound : 0;
        int weaponDamageMax = weaponDetails ? weaponDetails->range.upperBound : 0;

        const itemTable *armorDetails = getArmorDetails(metrics->armor.kind);
        const char *armorName = armorDetails ? armorDetails->name : "Unknown Armor";
        const char *armorDescription = armorDetails ? armorDetails->description : "No description available";
        int armorStrengthRequired = armorDetails ? armorDetails->strengthRequired : 0;
        int armorDefense = armorDetails ? armorDetails->power : 0;

        const itemTable *ringLeftDetails = getRingDetails(metrics->ringLeft.kind);
        const char *ringLeftName = ringLeftDetails ? ringLeftDetails->name : "Unknown Ring";
        const char *ringLeftDescription = ringLeftDetails ? ringLeftDetails->description : "No description available";

        const itemTable *ringRightDetails = getRingDetails(metrics->ringRight.kind);
        const char *ringRightName = ringRightDetails ? ringRightDetails->name : "Unknown Ring";
        const char *ringRightDescription = ringRightDetails ? ringRightDetails->description : "No description available";

        snprintf(post_data, sizeof(post_data),
            "{\"gold\": %d, \"goldGenerated\": %d, \"depthLevel\": %d, \"deepestLevel\": %d, "
            "\"currentHP\": %d, \"maxHP\": %d, \"turns\": %d, \"strength\": %d, \"monsterSpawnFuse\": %d, "
            "\"playerTurnNumber\": %d, \"absoluteTurnNumber\": %d, \"milliseconds\": %d, "
            "\"xpxpThisTurn\": %d, \"stealthRange\": %d, \"rewardRoomsGenerated\": %d, "
            "\"wizard\": %s, \"disturbed\": %s, \"gameInProgress\": %s, \"gameHasEnded\": %s, "
            "\"easyMode\": %s, \"seed\": %d, \"RNG\": %d, "
            "\"clairvoyance\": %d, \"stealthBonus\": %d, \"regenerationBonus\": %d, "
            "\"lightMultiplier\": %d, \"awarenessBonus\": %d, \"transference\": %d, "
            "\"wisdomBonus\": %d, \"reaping\": %d, \"regenPerTurn\": %d, "
            "\"weaknessAmount\": %d, \"poisonAmount\": %d, \"inventory\": %s,"
            "\"weapon\": {"
            "\"name\": \"%s\", \"description\": \"%s\", \"category\": \"%s\", \"kind\": %d, "
            "\"damageMin\": %d, \"damageMax\": %d, \"strengthRequired\": %d"
            "}, "
            "\"armor\": {"
            "\"name\": \"%s\", \"description\": \"%s\", \"category\": \"%s\", \"kind\": %d, "
            "\"armor\": %d, \"strengthRequired\": %d"
            "}, "
            "\"ringLeft\": {"
            "\"name\": \"%s\", \"description\": \"%s\", \"category\": \"%s\", \"kind\": %d"
            "}, "
            "\"ringRight\": {"
            "\"name\": \"%s\", \"description\": \"%s\", \"category\": \"%s\", \"kind\": %d"
            "}"
            "}",

            metrics->gold, metrics->goldGenerated, metrics->depthLevel, metrics->deepestLevel, metrics->currentHP,
            metrics->maxHP, metrics->turns, metrics->strength, metrics->monsterSpawnFuse,
            metrics->playerTurnNumber, metrics->absoluteTurnNumber, metrics->milliseconds,
            metrics->xpxpThisTurn, metrics->stealthRange, metrics->rewardRoomsGenerated,
            metrics->wizard ? "true" : "false", metrics->disturbed ? "true" : "false",
            metrics->gameInProgress ? "true" : "false", metrics->gameHasEnded ? "true" : "false",
            metrics->easyMode ? "true" : "false", metrics->seed, metrics->RNG,
            metrics->clairvoyance, metrics->stealthBonus, metrics->regenerationBonus,
            metrics->lightMultiplier, metrics->awarenessBonus, metrics->transference,
            metrics->wisdomBonus, metrics->reaping, metrics->regenPerTurn,
            metrics->weaknessAmount, metrics->poisonAmount,
            inventory_json,
            // Weapon details
            weaponName, weaponDescription,
            getItemCategory(metrics->weapon.category),
            metrics->weapon.kind,
            weaponDamageMin,
            weaponDamageMax,
            weaponStrengthRequired,

            // Armor details
            armorName, armorDescription,
            getItemCategory(metrics->armor.category),
            metrics->armor.kind,
            armorDefense,
            armorStrengthRequired,

            // Left Ring details
            ringLeftName, ringLeftDescription,
            getItemCategory(metrics->ringLeft.category),
            metrics->ringLeft.kind,

            // Right Ring details
            ringRightName, ringRightDescription,
            getItemCategory(metrics->ringRight.category),
            metrics->ringRight.kind
    );


        // Set CURL options for sending data (as before)
        curl_easy_setopt(curl, CURLOPT_URL, PORTAL_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

        // Set headers for JSON
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the POST request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to send metrics: %s\n", curl_easy_strerror(res));
        } else {
            //printf("Metrics sent successfully: %s\n", post_data);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Failed to initialize CURL session for metrics.\n");
    }
    }
}

// Function to extract item details and build JSON for the player's inventory
void extract_inventory_json(char *buffer, size_t buffer_size) {
    if (!rogue.gameHasEnded) {
    item *currentItem = packItems;
    size_t offset = 0;

    offset += snprintf(buffer + offset, buffer_size - offset, "[");

    while (currentItem && offset < buffer_size - 1) {
        const itemTable *itemDetails = NULL;
        const char *categoryName = getItemCategory(currentItem->category);
        const char *itemName = "Unknown";
        const char *itemDescription = "No description available";

        // Determine item details based on category and kind
        switch (currentItem->category) {
            case WEAPON:
                itemDetails = getWeaponDetails(currentItem->kind);
                break;
            case ARMOR:
                itemDetails = getArmorDetails(currentItem->kind);
                break;
            case RING:
                itemDetails = getRingDetails(currentItem->kind);
                break;
            // Add cases for other categories if needed
        }
        
        if (itemDetails) {
            itemName = itemDetails->name;
            itemDescription = itemDetails->description;
        }

        // Format item details as JSON
        offset += snprintf(buffer + offset, buffer_size - offset,
                           "{ \"category\": \"%s\", \"name\": \"%s\", \"description\": \"%s\", "
                           "\"quantity\": %d, \"armor\": %d, \"damage\": { \"min\": %d, \"max\": %d } }",
                           categoryName, itemName, itemDescription,
                           currentItem->quantity, currentItem->armor,
                           currentItem->damage.lowerBound, currentItem->damage.upperBound);

        if (currentItem->nextItem && offset < buffer_size - 2) {
            offset += snprintf(buffer + offset, buffer_size - offset, ", ");
        }

        currentItem = currentItem->nextItem;
    }
    snprintf(buffer + offset, buffer_size - offset, "]");
}
}