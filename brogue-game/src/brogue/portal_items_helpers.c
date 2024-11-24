#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "portal_items_helpers.h"

const itemTable *getWeaponDetails(short kind) {
    if (kind >= 0 && kind < NUMBER_WEAPON_KINDS) {
        return &weaponTable[kind];
    }
    fprintf(stderr, "Error: Invalid weapon kind %d\n", kind);
    return NULL;
}

const itemTable *getArmorDetails(short kind) {
    if (kind >= 0 && kind < NUMBER_ARMOR_KINDS) {
        return &armorTable[kind];
    }
    fprintf(stderr, "Error: Invalid armor kind %d\n", kind);
    return NULL;
}

const itemTable *getRingDetails(short kind) {
    if (kind >= 0 && kind < NUMBER_RING_KINDS) {
        return &ringTable[kind];
    }
    fprintf(stderr, "Error: Invalid ring kind %d\n", kind);
    return NULL;
}

/**
 * @brief Convert item category to a descriptive string.
 *
 * This function maps an item category to a human-readable string.
 *
 * @param category The category of the item.
 * @return A string representing the item category.
 */
const char* get_item_category(unsigned short category) {
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

/**
 * @brief Convert item kind to a descriptive string.
 *
 * This function maps an item kind (weapon, armor, etc.) to a human-readable string.
 *
 * @param category The item category (weapon, armor, etc.).
 * @param kind The kind of the item.
 * @return A string representing the item kind.
 */
const char* get_item_kind(unsigned short category, short kind) {
    switch (category) {
        case WEAPON:
            return get_weapon_kind(kind);
        case ARMOR:
            return get_armor_kind(kind);
        case RING:
            return get_ring_kind(kind);
        case POTION:
            return get_potion_kind(kind);
        case SCROLL:
            return get_scroll_kind(kind);
        case STAFF:
            return get_staff_kind(kind);
        case WAND:
            return get_wand_kind(kind);
        case CHARM:
            return get_charm_kind(kind);
        case FOOD:
        case GOLD:
        case AMULET:
        case GEM:
        case KEY:
            return "Unknown";  // Handle specific cases or default to "Unknown"
        default:
            return "Unknown";
    }
}

/**
 * @brief Convert weapon kind to a descriptive string.
 *
 * This function maps a weapon kind to a human-readable string.
 *
 * @param kind The kind of the weapon.
 * @return A string representing the weapon kind.
 */
const char* get_weapon_kind(short kind) {
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

/**
 * @brief Convert weapon enchantment kind to a descriptive string.
 *
 * This function maps a weapon enchantment kind to a human-readable string.
 *
 * @param enchant The kind of the weapon enchantment.
 * @return A string representing the weapon enchantment kind.
 */
const char* get_weapon_enchantment(short enchant) {
    switch (enchant) {
        case W_SLAYING: return "Slaying";
        case W_FORCE: return "Force";
        case W_PARALYSIS: return "Paralysis";
        case W_CONFUSION: return "Confusion";
        case W_SPEED: return "Speed";
        case W_MULTIPLICITY: return "Multiplicity";
        case W_MERCY: return "Mercy";
        case W_PLENTY: return "Plenty";
        case W_QUIETUS: return "Quietus";
        case W_SLOWING: return "Slowing";
    }
    return "Unknown";
}

/**
 * @brief Convert armor kind to a descriptive string.
 *
 * This function maps an armor kind to a human-readable string.
 *
 * @param kind The kind of the armor.
 * @return A string representing the armor kind.
 */
const char* get_armor_kind(short kind) {
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

/**
 * @brief Convert armor enchantment kind to a descriptive string.
 *
 * This function maps an armor enchantment kind to a human-readable string.
 *
 * @param enchant The kind of the armor enchantment.
 * @return A string representing the armor enchantment kind.
 */
const char* get_armor_enchantment(short enchant) {
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

/**
 * @brief Convert potion kind to a descriptive string.
 *
 * This function maps a potion kind to a human-readable string.
 *
 * @param kind The kind of the potion.
 * @return A string representing the potion kind.
 */
const char* get_potion_kind(short kind) {
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

/**
 * @brief Convert wand kind to a descriptive string.
 *
 * This function maps a wand kind to a human-readable string.
 *
 * @param kind The kind of the wand.
 * @return A string representing the wand kind.
 */
const char* get_wand_kind(short kind) {
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

/**
 * @brief Convert staff kind to a descriptive string.
 *
 * This function maps a staff kind to a human-readable string.
 *
 * @param kind The kind of the staff.
 * @return A string representing the staff kind.
 */
const char* get_staff_kind(short kind) {
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

/**
 * @brief Convert ring kind to a descriptive string.
 *
 * This function maps a ring kind to a human-readable string.
 *
 * @param kind The kind of the ring.
 * @return A string representing the ring kind.
 */
const char* get_ring_kind(short kind) {
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

/**
 * @brief Convert charm kind to a descriptive string.
 *
 * This function maps a charm kind to a human-readable string.
 *
 * @param kind The kind of the charm.
 * @return A string representing the charm kind.
 */
const char* get_charm_kind(short kind) {
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

/**
 * @brief Convert scroll kind to a descriptive string.
 *
 * This function maps a scroll kind to a human-readable string.
 *
 * @param kind The kind of the scroll.
 * @return A string representing the scroll kind.
 */
const char* get_scroll_kind(short kind) {
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

/**
 * @brief Escapes a string for JSON.
 *
 * This function escapes a string for use in a JSON string.
 *
 * @param src The string to escape
 * @param dest The destination buffer
 * @param dest_size The size of the destination buffer
 * @return A string representing escapped data that can be used in JSON.
 */
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
    dest[j] = '\0'; // Null-terminate the destination string
}
