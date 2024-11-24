#ifndef PORTAL_ITEMS_HELPERS_H
#define PORTAL_ITEMS_HELPERS_H

#include "Rogue.h"

/**
 * @brief Converts item category to a descriptive string.
 *
 * This function maps an item category to a human-readable string.
 * 
 * @param category The category of the item.
 * @return A string representing the item category.
 */
extern const char* get_item_category(unsigned short category);

/**
 * @brief Converts item kind to a descriptive string.
 *
 * This function maps an item kind (weapon, armor, etc.) to a human-readable string.
 * 
 * @param category The item category (weapon, armor, etc.).
 * @param kind The kind of the item.
 * @return A string representing the item kind.
 */
extern const char* get_item_kind(unsigned short category, short kind);

/**
 * @brief Converts weapon kind to a descriptive string.
 *
 * This function maps a weapon kind to a human-readable string.
 *
 * @param kind The kind of the weapon.
 * @return A string representing the weapon kind.
 */
extern const char* get_weapon_kind(short kind);

/**
 * @brief Convert weapon enchantment kind to a descriptive string.
 *
 * This function maps a weapon enchantment kind to a human-readable string.
 *
 * @param enchant The kind of the weapon enchantment.
 * @return A string representing the weapon enchantment kind.
 */
const char* get_weapon_enchantment(short enchant);

/**
 * @brief Converts armor kind to a descriptive string.
 *
 * This function maps an armor kind to a human-readable string.
 *
 * @param kind The kind of the armor.
 * @return A string representing the armor kind.
 */
extern const char* get_armor_kind(short kind);

/**
 * @brief Converts armor enchantment kind to a descriptive string.
 *
 * This function maps an armor enchantment kind to a human-readable string.
 *
 * @param enchant The kind of the armor enchantment.
 * @return A string representing the armor enchantment kind.
 */
extern const char* get_armor_enchantment(short enchant);

/**
 * @brief Converts potion kind to a descriptive string.
 *
 * This function maps a potion kind to a human-readable string.
 *
 * @param kind The kind of the potion.
 * @return A string representing the potion kind.
 */
extern const char* get_potion_kind(short kind);

/**
 * @brief Converts wand kind to a descriptive string.
 *
 * This function maps a wand kind to a human-readable string.
 *
 * @param kind The kind of the wand.
 * @return A string representing the wand kind.
 */
extern const char* get_wand_kind(short kind);

/**
 * @brief Converts staff kind to a descriptive string.
 *
 * This function maps a staff kind to a human-readable string.
 *
 * @param kind The kind of the staff.
 * @return A string representing the staff kind.
 */
extern const char* get_staff_kind(short kind);

/**
 * @brief Converts ring kind to a descriptive string.
 *
 * This function maps a ring kind to a human-readable string.
 *
 * @param kind The kind of the ring.
 * @return A string representing the ring kind.
 */
extern const char* get_ring_kind(short kind);

/**
 * @brief Converts charm kind to a descriptive string.
 *
 * This function maps a charm kind to a human-readable string.
 *
 * @param kind The kind of the charm.
 * @return A string representing the charm kind.
 */
extern const char* get_charm_kind(short kind);

/**
 * @brief Converts scroll kind to a descriptive string.
 *
 * This function maps a scroll kind to a human-readable string.
 *
 * @param kind The kind of the scroll.
 * @return A string representing the scroll kind.
 */
extern const char* get_scroll_kind(short kind);

/**
 * @brief Escapes a string for JSON compatibility.
 *
 * This function escapes a string for use in a JSON string.
 *
 * @param src The string to escape.
 * @param dest The destination buffer.
 * @param dest_size The size of the destination buffer.
 */
extern void escape_json_string(const char *src, char *dest, size_t dest_size);

extern const itemTable *getWeaponDetails(short kind);

extern const itemTable *getArmorDetails(short kind);

extern const itemTable *getRingDetails(short kind);

#endif // PORTAL_ITEMS_HELPERS_H
