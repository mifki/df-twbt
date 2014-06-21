/* THIS FILE WAS GENERATED. DO NOT EDIT. */
#ifndef DF_ITEM_TYPE_H
#define DF_ITEM_TYPE_H
namespace df {
  namespace enums {
    namespace item_type {
      enum item_type : int16_t {
        NONE = -1,
        /**
         * Bars, such as metal, fuel, or soap.
         */
        BAR,
        /**
         * Cut gemstones usable in jewelers workshop
         */
        SMALLGEM,
        /**
         * Blocks of any kind.
         */
        BLOCKS,
        /**
         * Rough gemstones.
         */
        ROUGH,
        /**
         * Raw mined stone.
         */
        BOULDER,
        /**
         * Wooden logs.
         */
        WOOD,
        /**
         * Doors.
         */
        DOOR,
        /**
         * Floodgates.
         */
        FLOODGATE,
        /**
         * Beds.
         */
        BED,
        /**
         * Chairs and thrones.
         */
        CHAIR,
        /**
         * Restraints.
         */
        CHAIN,
        /**
         * Flasks.
         */
        FLASK,
        /**
         * Goblets.
         */
        GOBLET,
        /**
         * Musical instruments.
         */
        INSTRUMENT,
        /**
         * Toys.
         */
        TOY,
        /**
         * Glass windows.
         */
        WINDOW,
        /**
         * Cages.
         */
        CAGE,
        /**
         * Barrels.
         */
        BARREL,
        /**
         * Buckets.
         */
        BUCKET,
        /**
         * Animal traps.
         */
        ANIMALTRAP,
        /**
         * Tables.
         */
        TABLE,
        /**
         * Coffins.
         */
        COFFIN,
        /**
         * Statues.
         */
        STATUE,
        /**
         * Corpses. Does not have a material.
         */
        CORPSE,
        /**
         * Weapons.
         */
        WEAPON,
        /**
         * Armor and clothing worn on the upper body.
         */
        ARMOR,
        /**
         * Armor and clothing worn on the feet.
         */
        SHOES,
        /**
         * Shields and bucklers.
         */
        SHIELD,
        /**
         * Armor and clothing worn on the head.
         */
        HELM,
        /**
         * Armor and clothing worn on the hands.
         */
        GLOVES,
        /**
         * Chests (wood), coffers (stone), boxes (glass), and bags (cloth or leather).
         */
        BOX,
        /**
         * Bins.
         */
        BIN,
        /**
         * Armor stands.
         */
        ARMORSTAND,
        /**
         * Weapon racks.
         */
        WEAPONRACK,
        /**
         * Cabinets.
         */
        CABINET,
        /**
         * Figurines.
         */
        FIGURINE,
        /**
         * Amulets.
         */
        AMULET,
        /**
         * Scepters.
         */
        SCEPTER,
        /**
         * Ammunition for hand-held weapons.
         */
        AMMO,
        /**
         * Crowns.
         */
        CROWN,
        /**
         * Rings.
         */
        RING,
        /**
         * Earrings.
         */
        EARRING,
        /**
         * Bracelets.
         */
        BRACELET,
        /**
         * Large gems.
         */
        GEM,
        /**
         * Anvils.
         */
        ANVIL,
        /**
         * Body parts. Does not have a material.
         */
        CORPSEPIECE,
        /**
         * Dead vermin bodies. Material is CREATURE_ID:CASTE.
         */
        REMAINS,
        /**
         * Butchered meat.
         */
        MEAT,
        /**
         * Prepared fish. Material is CREATURE_ID:CASTE.
         */
        FISH,
        /**
         * Unprepared fish. Material is CREATURE_ID:CASTE.
         */
        FISH_RAW,
        /**
         * Live vermin. Material is CREATURE_ID:CASTE.
         */
        VERMIN,
        /**
         * Tame vermin. Material is CREATURE_ID:CASTE.
         */
        PET,
        /**
         * Seeds from plants.
         */
        SEEDS,
        /**
         * Plants.
         */
        PLANT,
        /**
         * Tanned skins.
         */
        SKIN_TANNED,
        /**
         * Leaves, usually from quarry bushes.
         */
        LEAVES,
        /**
         * Thread gathered from webs or made at the farmers workshop.
         */
        THREAD,
        /**
         * Cloth made at the loom.
         */
        CLOTH,
        /**
         * Skull totems.
         */
        TOTEM,
        /**
         * Armor and clothing worn on the legs.
         */
        PANTS,
        /**
         * Backpacks.
         */
        BACKPACK,
        /**
         * Quivers.
         */
        QUIVER,
        /**
         * Catapult parts.
         */
        CATAPULTPARTS,
        /**
         * Ballista parts.
         */
        BALLISTAPARTS,
        /**
         * Siege engine ammunition.
         */
        SIEGEAMMO,
        /**
         * Ballista arrow heads.
         */
        BALLISTAARROWHEAD,
        /**
         * Mechanisms.
         */
        TRAPPARTS,
        /**
         * Trap components.
         */
        TRAPCOMP,
        /**
         * Alcoholic drinks.
         */
        DRINK,
        /**
         * Powders such as flour, gypsum plaster, dye, or sand.
         */
        POWDER_MISC,
        /**
         * Pieces of cheese.
         */
        CHEESE,
        /**
         * Prepared meals. Subtypes come from item_food.txt
         */
        FOOD,
        /**
         * Liquids such as water, lye, and extracts.
         */
        LIQUID_MISC,
        /**
         * Coins.
         */
        COIN,
        /**
         * Fat, tallow, pastes/pressed objects, and small bits of molten rock/metal.
         */
        GLOB,
        /**
         * Small rocks (usually sharpened and/or thrown in adventurer mode)
         */
        ROCK,
        /**
         * Pipe sections.
         */
        PIPE_SECTION,
        /**
         * Hatch covers.
         */
        HATCH_COVER,
        /**
         * Grates.
         */
        GRATE,
        /**
         * Querns.
         */
        QUERN,
        /**
         * Millstones.
         */
        MILLSTONE,
        /**
         * Splints.
         */
        SPLINT,
        /**
         * Crutches.
         */
        CRUTCH,
        /**
         * Traction benches.
         */
        TRACTION_BENCH,
        /**
         * Casts
         */
        ORTHOPEDIC_CAST,
        /**
         * Tools.
         */
        TOOL,
        /**
         * Slabs.
         */
        SLAB,
        /**
         * Eggs. Material is CREATURE_ID:CASTE.
         */
        EGG,
        /**
         * Books.
         */
        BOOK
      };
    }
  }
  using enums::item_type::item_type;
  template<> struct DFHACK_EXPORT identity_traits<item_type> {
    static enum_identity identity;
    static enum_identity *get() { return &identity; }
  };
  template<> struct DFHACK_EXPORT enum_traits<item_type> {
    typedef int16_t base_type;
    typedef item_type enum_type;
    static const base_type first_item_value = -1;
    static const base_type last_item_value = 88;
    static inline bool is_valid(enum_type value) {
      return (base_type(value) >= first_item_value && base_type(value) <= last_item_value);
    }
    static const enum_type first_item = (enum_type)first_item_value;
    static const enum_type last_item = (enum_type)last_item_value;
    static const char *const key_table[90];
    struct attr_entry_type {
      const char* caption;
      bool is_rawable;
      bool is_stackable;
      bool is_caste_mat;
      const char* classname;
      static struct_identity _identity;
    };
    static const attr_entry_type attr_table[90+1];
    static const attr_entry_type &attrs(enum_type value);
  };
}
#endif
