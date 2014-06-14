/* THIS FILE WAS GENERATED. DO NOT EDIT. */
#ifndef DF_BUILDING_TYPE_H
#define DF_BUILDING_TYPE_H
namespace df {
  namespace enums {
    namespace building_type {
      enum building_type : int32_t {
        NONE = -1,
        Chair,
        Bed,
        Table,
        Coffin,
        FarmPlot,
        Furnace,
        TradeDepot,
        Shop,
        Door,
        Floodgate,
        Box,
        Weaponrack,
        Armorstand,
        Workshop,
        Cabinet,
        Statue,
        WindowGlass,
        WindowGem,
        Well,
        Bridge,
        RoadDirt,
        RoadPaved,
        SiegeEngine,
        Trap,
        AnimalTrap,
        Support,
        ArcheryTarget,
        Chain,
        Cage,
        Stockpile,
        Civzone,
        Weapon,
        Wagon,
        ScrewPump,
        Construction,
        Hatch,
        GrateWall,
        GrateFloor,
        BarsVertical,
        BarsFloor,
        GearAssembly,
        AxleHorizontal,
        AxleVertical,
        WaterWheel,
        Windmill,
        TractionBench,
        Slab,
        Nest,
        NestBox,
        Hive,
        Rollers
      };
    }
  }
  using enums::building_type::building_type;
  template<> struct DFHACK_EXPORT identity_traits<building_type> {
    static enum_identity identity;
    static enum_identity *get() { return &identity; }
  };
  template<> struct DFHACK_EXPORT enum_traits<building_type> {
    typedef int32_t base_type;
    typedef building_type enum_type;
    static const base_type first_item_value = -1;
    static const base_type last_item_value = 50;
    static inline bool is_valid(enum_type value) {
      return (base_type(value) >= first_item_value && base_type(value) <= last_item_value);
    }
    static const enum_type first_item = (enum_type)first_item_value;
    static const enum_type last_item = (enum_type)last_item_value;
    static const char *const key_table[52];
    struct attr_entry_type {
      const char* classname;
      static struct_identity _identity;
    };
    static const attr_entry_type attr_table[52+1];
    static const attr_entry_type &attrs(enum_type value);
  };
}
#endif
