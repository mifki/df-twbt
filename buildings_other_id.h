/* THIS FILE WAS GENERATED. DO NOT EDIT. */
#ifndef DF_BUILDINGS_OTHER_ID_H
#define DF_BUILDINGS_OTHER_ID_H
#ifndef DF_BUILDING_TYPE_H
#include "building_type.h"
#endif
#ifndef DF_CIVZONE_TYPE_H
#include "civzone_type.h"
#endif
#ifndef DF_FURNACE_TYPE_H
#include "furnace_type.h"
#endif
#ifndef DF_WORKSHOP_TYPE_H
#include "workshop_type.h"
#endif
namespace df {
  namespace enums {
    namespace buildings_other_id {
      enum buildings_other_id : int32_t {
        ANY = -1,
        /**
         * actually on the map, as opposed to in an offloaded fortress elsewhere
         */
        IN_PLAY,
        STOCKPILE,
        ANY_ZONE,
        ACTIVITY_ZONE,
        ANY_ACTUAL,
        ANY_MACHINE,
        ANY_HOSPITAL_STORAGE,
        ANY_STORAGE,
        ANY_BARRACKS,
        ANY_NOBLE_ROOM,
        ANY_HOSPITAL,
        BOX,
        CABINET,
        TRAP,
        DOOR,
        FLOODGATE,
        HATCH,
        GRATE_WALL,
        GRATE_FLOOR,
        BARS_VERTICAL,
        BARS_FLOOR,
        WINDOW_ANY,
        WELL,
        TABLE,
        BRIDGE,
        CHAIR,
        TRADE_DEPOT,
        NEST,
        NEST_BOX,
        HIVE,
        WAGON,
        SHOP,
        BED,
        TRACTION_BENCH,
        ANY_ROAD,
        FARM_PLOT,
        GEAR_ASSEMBLY,
        ROLLERS,
        AXLE_HORIZONTAL,
        AXLE_VERTICAL,
        SUPPORT,
        ARCHERY_TARGET,
        SCREW_PUMP,
        WATER_WHEEL,
        WINDMILL,
        CHAIN,
        CAGE,
        STATUE,
        SLAB,
        COFFIN,
        WEAPON_RACK,
        ARMOR_STAND,
        FURNACE_ANY,
        FURNACE_WOOD,
        FURNACE_SMELTER_ANY,
        FURNACE_SMELTER_MAGMA,
        FURNACE_KILN_ANY,
        FURNACE_GLASS_ANY,
        FURNACE_CUSTOM,
        WORKSHOP_ANY,
        WORKSHOP_BUTCHER,
        WORKSHOP_MASON,
        WORKSHOP_KENNEL,
        WORKSHOP_FISHERY,
        WORKSHOP_JEWELER,
        WORKSHOP_LOOM,
        WORKSHOP_TANNER,
        WORKSHOP_DYER,
        WORKSHOP_MILL_ANY,
        WORKSHOP_QUERN,
        WORKSHOP_TOOL,
        WORKSHOP_MILLSTONE,
        WORKSHOP_KITCHEN,
        WORKSHOP_STILL,
        WORKSHOP_FARMER,
        WORKSHOP_ASHERY,
        WORKSHOP_CARPENTER,
        WORKSHOP_CRAFTSDWARF,
        WORKSHOP_MECHANIC,
        WORKSHOP_SIEGE,
        WORKSHOP_CLOTHIER,
        WORKSHOP_LEATHER,
        WORKSHOP_BOWYER,
        WORKSHOP_MAGMA_FORGE,
        WORKSHOP_FORGE_ANY,
        WORKSHOP_CUSTOM,
        WEAPON_UPRIGHT
      };
    }
  }
  using enums::buildings_other_id::buildings_other_id;
  template<> struct DFHACK_EXPORT identity_traits<buildings_other_id> {
    static enum_identity identity;
    static enum_identity *get() { return &identity; }
  };
  template<> struct DFHACK_EXPORT enum_traits<buildings_other_id> {
    typedef int32_t base_type;
    typedef buildings_other_id enum_type;
    static const base_type first_item_value = -1;
    static const base_type last_item_value = 86;
    static inline bool is_valid(enum_type value) {
      return (base_type(value) >= first_item_value && base_type(value) <= last_item_value);
    }
    static const enum_type first_item = (enum_type)first_item_value;
    static const enum_type last_item = (enum_type)last_item_value;
    static const char *const key_table[88];
    struct attr_entry_type {
      df::building_type building;
      enum_list_attr<df::building_type> generic_building;
      enum_list_attr<df::workshop_type> workshop;
      enum_list_attr<df::furnace_type> furnace;
      enum_list_attr<df::civzone_type> civzone;
      static struct_identity _identity;
    };
    static const attr_entry_type attr_table[88+1];
    static const attr_entry_type &attrs(enum_type value);
  };
}
#endif
