// portal-items.c

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal-items.h"

#define ITEM_PORTAL_URL "http://portal-service.portal:5000/items/update"
#define PACK_PORTAL_URL "http://portal-service.portal:5000/pack/update"
#define BUFFER_SIZE 8192 

void extract_inventory_json(char *buffer, size_t buffer_size);

// Convert item category to a descriptive string
const char* getCategoryName(unsigned short category) {
    switch (category) {
        case FOOD: return "Food";
        case WEAPON: return "Weapon";
        case ARMOR: return "Armor";
        case POTION: return "Potion";
        case SCROLL: return "Scroll";
        case STAFF: return "Staff";
        case WAND: return "Wand";
        case RING: return "Ring";
        case CHARM: return "Charm";
        case GOLD: return "Gold";
        case AMULET: return "Amulet";
        case GEM: return "Gem";
        case KEY: return "Key";
        default: return "Unknown";
    }
}

// Convert weapon kind to a descriptive string
const char* getWeaponKindName(short kind) {
    switch (kind) {
        case DAGGER: return "Dagger";
        case SWORD: return "Sword";
        case BROADSWORD: return "Broadsword";
        case WHIP: return "Whip";
        case RAPIER: return "Rapier";
        case FLAIL: return "Flail";
        case MACE: return "Mace";
        case HAMMER: return "Hammer";
        case SPEAR: return "Spear";
        case PIKE: return "Pike";
        case AXE: return "Axe";
        case WAR_AXE: return "War Axe";
        case DART: return "Dart";
        case INCENDIARY_DART: return "Incendiary Dart";
        case JAVELIN: return "Javelin";
        default: return "Unknown";
    }
}

const char* getWeaponEnchantName(short enchant) {
    switch (enchant) {
        case W_SLAYING: return "Slaying";
        case W_FORCE: return "Force";
        case W_CONFUSION: return "Confusion";
        case W_SLOWING: return "Slowing";
        case W_MULTIPLICITY: return "Multiplicity";
        case W_PARALYSIS: return "Paralysis";
        case W_QUIETUS: return "Quietus";
        case W_SPEED: return "Speed";
        case W_MERCY: return "Mercy";
        case W_PLENTY: return "Plenty";
        default: return "Unknown";
    }
}

const char* getArmorKindName(short kind) {
    switch (kind) {
        case LEATHER_ARMOR: return "Leather Armor";
        case SCALE_MAIL: return "Scale Mail";
        case CHAIN_MAIL: return "Chain Mail";
        case BANDED_MAIL: return "Banded Mail";
        case SPLINT_MAIL: return "Splint Mail";
        case PLATE_MAIL: return "Plate Mail";
        default: return "Unknown";
    }
}

const char* getArmorEnchantName(short enchant) {
    switch (enchant) {
        case A_MULTIPLICITY: return "Multiplicity";
        case A_MUTUALITY: return "Mutuality";
        case A_ABSORPTION: return "Absorption";
        case A_REPRISAL: return "Reprisal";
        case A_IMMUNITY: return "Immunity";
        case A_REFLECTION: return "Reflection";
        case A_RESPIRATION: return "Respiration";
        case A_DAMPENING: return "Dampening";
        case A_BURDEN: return "Burden";
        case A_VULNERABILITY: return "Vulnerability";
        case A_IMMOLATION: return "Immolation";
        default: return "Unknown";
    }
}

const char* getPotionKindName(short kind) {
    switch (kind) {
        case POTION_LIFE: return "Life";
        case POTION_STRENGTH: return "Strength";
        case POTION_TELEPATHY: return "Telepathy";
        case POTION_LEVITATION: return "Levitation";
        case POTION_DETECT_MAGIC: return "Detect Magic";
        case POTION_HASTE_SELF: return "Haste Self";
        case POTION_FIRE_IMMUNITY: return "Fire Immunity";
        case POTION_INVISIBILITY: return "Invisibility";
        case POTION_POISON: return "Poison";
        case POTION_PARALYSIS: return "Paralysis";
        case POTION_HALLUCINATION: return "Hallucination";
        case POTION_CONFUSION: return "Confusion";
        case POTION_INCINERATION: return "Incineration";
        case POTION_DARKNESS: return "Darkness";
        case POTION_DESCENT: return "Descent";
        case POTION_LICHEN: return "Lichen";
        default: return "Unknown";
    }
}

const char* getWandKindName(short kind) {
    switch (kind) {
        case WAND_TELEPORT: return "Teleport";
        case WAND_SLOW: return "Slow";
        case WAND_POLYMORPH: return "Polymorph";
        case WAND_NEGATION: return "Negation";
        case WAND_DOMINATION: return "Domination";
        case WAND_BECKONING: return "Beckoning";
        case WAND_PLENTY: return "Plenty";
        case WAND_INVISIBILITY: return "Invisibility";
        case WAND_EMPOWERMENT: return "Empowerment";
        default: return "Unknown";
    }
}

const char* getStaffKindName(short kind) {
    switch (kind) {
        case STAFF_LIGHTNING: return "Lightning";
        case STAFF_FIRE: return "Fire";
        case STAFF_POISON: return "Poison";
        case STAFF_TUNNELING: return "Tunneling";
        case STAFF_BLINKING: return "Blinking";
        case STAFF_ENTRANCEMENT: return "Entrancement";
        case STAFF_OBSTRUCTION: return "Obstruction";
        case STAFF_DISCORD: return "Discord";
        case STAFF_CONJURATION: return "Conjuration";
        case STAFF_HEALING: return "Healing";
        case STAFF_HASTE: return "Haste";
        case STAFF_PROTECTION: return "Protection";
        default: return "Unknown";
    }
}

const char* getRingKindName(short kind) {
    switch (kind) {
        case RING_CLAIRVOYANCE: return "Clairvoyance";
        case RING_STEALTH: return "Stealth";
        case RING_REGENERATION: return "Regeneration";
        case RING_TRANSFERENCE: return "Transference";
        case RING_LIGHT: return "Light";
        case RING_AWARENESS: return "Awareness";
        case RING_WISDOM: return "Wisdom";
        case RING_REAPING: return "Reaping";
        default: return "Unknown";
    }
}

const char* getCharmKindName(short kind) {
    switch (kind) {
        case CHARM_HEALTH: return "Health";
        case CHARM_PROTECTION: return "Protection";
        case CHARM_HASTE: return "Haste";
        case CHARM_FIRE_IMMUNITY: return "Fire Immunity";
        case CHARM_INVISIBILITY: return "Invisibility";
        case CHARM_TELEPATHY: return "Telepathy";
        case CHARM_LEVITATION: return "Levitation";
        case CHARM_SHATTERING: return "Shattering";
        case CHARM_GUARDIAN: return "Guardian";
        case CHARM_TELEPORTATION: return "Teleportation";
        case CHARM_RECHARGING: return "Recharging";
        case CHARM_NEGATION: return "Negation";
        default: return "Unknown";
    }
}

const char* getScrollKindName(short kind) {
    switch (kind) {
        case SCROLL_ENCHANTING: return "Enchanting";
        case SCROLL_IDENTIFY: return "Identify";
        case SCROLL_TELEPORT: return "Teleport";
        case SCROLL_REMOVE_CURSE: return "Remove Curse";
        case SCROLL_RECHARGING: return "Recharging";
        case SCROLL_PROTECT_ARMOR: return "Protect Armor";
        case SCROLL_PROTECT_WEAPON: return "Protect Weapon";
        case SCROLL_SANCTUARY: return "Sanctuary";
        case SCROLL_MAGIC_MAPPING: return "Magic Mapping";
        case SCROLL_NEGATION: return "Negation";
        case SCROLL_SHATTERING: return "Shattering";
        case SCROLL_DISCORD: return "Discord";
        case SCROLL_AGGRAVATE_MONSTER: return "Aggravate Monster";
        case SCROLL_SUMMON_MONSTER: return "Summon Monster";
        default: return "Unknown";
    }
}

// Function to send item information to the portal
void update_items(void) {
    ItemMetrics items = {
        .weapon = (rogue.weapon != NULL) ? *rogue.weapon : (item){0},
        .armor = (rogue.armor != NULL) ? *rogue.armor : (item){0},
        .ringLeft = (rogue.ringLeft != NULL) ? *rogue.ringLeft : (item){0},
        .ringRight = (rogue.ringRight != NULL) ? *rogue.ringRight : (item){0},
    };
    
    send_items_to_portal(&items);
    send_pack_to_portal();
}

void escape_json_string(const char *src, char *dest, size_t dest_size) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j < dest_size - 1; i++) {
        if (src[i] == '"' || src[i] == '\\') {
            if (j < dest_size - 2) { // Ensure room for escape character
                dest[j++] = '\\';
            }
        }
        dest[j++] = src[i];
    }
    dest[j] = '\0'; // Null-terminate the destination
}

void send_items_to_portal(const ItemMetrics *items) {
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



void send_pack_to_portal() {
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
void extract_inventory_json(char *buffer, size_t buffer_size) {
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