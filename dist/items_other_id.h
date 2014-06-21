/* THIS FILE WAS GENERATED. DO NOT EDIT. */
#ifndef DF_ITEMS_OTHER_ID_H
#define DF_ITEMS_OTHER_ID_H
#ifndef DF_ITEM_TYPE_H
#include "item_type.h"
#endif
namespace df {
  namespace enums {
    namespace items_other_id {
      enum items_other_id : int32_t {
        ANY = -1,
        /**
         * actually on the map, as opposed to in an offloaded fortress elsewhere
         */
        IN_PLAY,
        ANY_ARTIFACT,
        WEAPON,
        ANY_WEAPON,
        ANY_SPIKE,
        ANY_TRUE_ARMOR,
        ANY_ARMOR_HELM,
        ANY_ARMOR_SHOES,
        SHIELD,
        ANY_ARMOR_GLOVES,
        ANY_ARMOR_PANTS,
        QUIVER,
        SPLINT,
        ORTHOPEDIC_CAST,
        CRUTCH,
        BACKPACK,
        AMMO,
        WOOD,
        BOULDER,
        ROCK,
        ANY_REFUSE,
        ANY_GOOD_FOOD,
        ANY_AUTO_CLEAN,
        ANY_GENERIC23,
        ANY_GENERIC24,
        ANY_FURNITURE,
        ANY_CAGE_OR_TRAP,
        ANY_EDIBLE_RAW,
        ANY_EDIBLE_CARNIVORE,
        ANY_EDIBLE_BONECARN,
        ANY_EDIBLE_VERMIN,
        ANY_EDIBLE_VERMIN_BOX,
        ANY_CAN_ROT,
        ANY_MURDERED,
        ANY_DEAD_DWARF,
        ANY_GENERIC35,
        ANY_GENERIC36,
        ANY_GENERIC37,
        ANY_GENERIC38,
        DOOR,
        FLOODGATE,
        HATCH_COVER,
        GRATE,
        CAGE,
        FLASK,
        WINDOW,
        GOBLET,
        INSTRUMENT,
        TOY,
        TOOL,
        BUCKET,
        BARREL,
        CHAIN,
        ANIMALTRAP,
        BED,
        TRACTION_BENCH,
        CHAIR,
        COFFIN,
        TABLE,
        STATUE,
        SLAB,
        QUERN,
        MILLSTONE,
        BOX,
        BIN,
        ARMORSTAND,
        WEAPONRACK,
        CABINET,
        ANVIL,
        CATAPULTPARTS,
        BALLISTAPARTS,
        SIEGEAMMO,
        TRAPPARTS,
        ANY_WEBS,
        PIPE_SECTION,
        ANY_ENCASED,
        ANY_IN_CONSTRUCTION,
        DRINK,
        ANY_DRINK,
        LIQUID_MISC,
        POWDER_MISC,
        ANY_COOKABLE,
        ANY_GENERIC82,
        VERMIN,
        PET,
        ANY_CRITTER,
        COIN,
        GLOB,
        TRAPCOMP,
        BAR,
        SMALLGEM,
        BLOCKS,
        ROUGH,
        ANY_CORPSE,
        CORPSE,
        BOOK,
        FIGURINE,
        AMULET,
        SCEPTER,
        CROWN,
        RING,
        EARRING,
        BRACELET,
        GEM,
        CORPSEPIECE,
        REMAINS,
        MEAT,
        FISH,
        FISH_RAW,
        EGG,
        SEEDS,
        PLANT,
        SKIN_TANNED,
        LEAVES,
        THREAD,
        CLOTH,
        TOTEM,
        PANTS,
        CHEESE,
        FOOD,
        BALLISTAARROWHEAD,
        ARMOR,
        SHOES,
        HELM,
        GLOVES,
        ANY_GENERIC123,
        FOOD_STORAGE,
        ANY_RECENTLY_DROPPED,
        ANY_MELT_DESIGNATED,
        BAD
      };
    }
  }
  using enums::items_other_id::items_other_id;
  template<> struct DFHACK_EXPORT identity_traits<items_other_id> {
    static enum_identity identity;
    static enum_identity *get() { return &identity; }
  };
  template<> struct DFHACK_EXPORT enum_traits<items_other_id> {
    typedef int32_t base_type;
    typedef items_other_id enum_type;
    static const base_type first_item_value = -1;
    static const base_type last_item_value = 129;
    static inline bool is_valid(enum_type value) {
      return (base_type(value) >= first_item_value && base_type(value) <= last_item_value);
    }
    static const enum_type first_item = (enum_type)first_item_value;
    static const enum_type last_item = (enum_type)last_item_value;
    static const char *const key_table[131];
    struct attr_entry_type {
      df::item_type item;
      enum_list_attr<df::item_type> generic_item;
      static struct_identity _identity;
    };
    static const attr_entry_type attr_table[131+1];
    static const attr_entry_type &attrs(enum_type value);
  };
}
#endif
