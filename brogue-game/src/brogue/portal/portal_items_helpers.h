#ifndef PORTAL_ITEMS_HELPERS_H
#define PORTAL_ITEMS_HELPERS_H

#include "Rogue.h"

extern const char* getWeaponKindName(short kind);
extern const char* getCategoryName(unsigned short category);
extern const char* getWeaponEnchantName(short enchant);
extern const char* getArmorKindName(short kind);
extern const char* getArmorEnchantName(short enchant);
extern const char* getPotionKindName(short kind);
extern const char* getWandKindName(short kind);
extern const char* getStaffKindName(short kind);
extern const char* getRingKindName(short kind);
extern const char* getCharmKindName(short kind);
extern const char* getScrollKindName(short kind);

extern void escape_json_string(const char *src, char *dest, size_t dest_size);

extern const itemTable *getWeaponDetails(short kind);
extern const itemTable *getArmorDetails(short kind);
extern const itemTable *getRingDetails(short kind);
extern const char *getItemCategoryName(unsigned short category);
extern const char *getItemCategory(unsigned short category);

extern void extract_inventory_json(char *buffer, size_t buffer_size);

#endif // PORTAL_ITEMS_HELPERS_H
