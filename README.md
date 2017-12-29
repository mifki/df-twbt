# Text Will Be Text #

This is a plugin for [Dwarf Fortress](http://bay12games.com) / [DFHack](http://github.com/dfhack/dfhack) that improves various aspects the game interface.

Originally I wrote a small plugin because I was tired seeing coffins instead of zeroes and all that stuff. It has greately evolved since then. The primary goal is to improve Fortress Mode. Adventure Mode is generally supported but may have issues.

*Note:* To activate the plugin, set PRINT_MODE to TWBT (or TWBT_LEGACY for legacy mode with text and map tiles of the same size) in your `data/init/init.txt` file. The installation package also includes several other plugins which required changes to be compatible with TWBT.

![Screenshot](http://i.imgur.com/Xdmv3JH.png)
![Screenshot](http://mifki.com/assets/uploads/Screen-Shot-2017-03-20-at-12.25.29-AM.png)

## Features ##

[Text and Map Tilesets](#text-and-map-tilesets)  
Use separate fonts for text and map, possibly with different tile size.

[Overrides](#overrides)  
Change images used for specific buildings, items and tile types.

[Multi-level Rendering](#multi-level-rendering)  
See multiple terrain levels in mountainous areas and caverns.

[Additional Colours](#additional-colours)

[Colormap Manipulation](#colormap-manipulation)  
Change and reload colours without restarting the game.

[Mapshot!](#mapshot)  
A command to save full-size screenshot of the map.

[Other Improvements](#other-improvements)

## Authors and Links ##

Vitaly Pronkin aka mifki <pronvit@me.com>

[Home Page / Sources](https://github.com/mifki/df-twbt) -- [Latest Release](https://github.com/mifki/df-twbt/releases) -- [Development Builds](http://build.mifki.com) -- [Discussion](http://www.bay12forums.com/smf/index.php?topic=138754.0) -- [Report an Issue](https://github.com/mifki/df-twbt/issues)

## Text and Map Tilesets ##

Main function is to use separate fonts (tilesets) for map tiles in Fortress Mode and for text. These fonts may have different tile size, usually square for the map and non-square for text. There also no limit of minimum 80 columns anymore, so bigger tiles can be used on small screens.

In your `init.txt` set FONT and FULLFONT to the font you want to use for **text**, and GRAPHICS\_FONT and GRAPHICS\_FULLFONT to the font for **map tiles**.

**`twbt tilesize bigger | smaller`**
**`twbt tilesize +<delta> | -<delta>`**  
**`twbt tilesize <w> <h>`**  
These commands adjust the size of map tiles or set the exact size.

**`twbt tilesize reset`**  
This command resets map tile size back to normal.

**`twbt redraw_all 0|1`**
This command controls the full redraw mode, in which all tiles are updated each frame - useful for some tilesets in case of graphics issues when scrolling.

**`twbt hide_stockpiles 0|1`**
If this option is enabled, stockpiles will be hidden unless in [q], [p] or [k] mode. Requires `redraw_all`.

## Overrides ##

Commands described below are to be placed in `data/init/overrides.txt` file and allow to change tile images used for specific buildings, items and tile types.

**Loading additional tilesets**

    [TILESET:font.png:fullscreenfont.png:Id] 

- File names are relative to the `data/art` folder

- `Id` is an arbitrary string to refer this tileset later

**Overrides for items and buildings**
    
    [OVERRIDE:Tile:Kind:Id:Type:Subtype:Tileset:NewTile:NewFg:NewBg]
    
- `Tile` is the original tile number

- `Kind` is `B` for building or `I` for item

- `Id` is an [item ID](#item-ids) or [building ID](#building-ids), or empty to match any ID

- `Type` is an [item type](#item-types) or [building type](#building-types), or empty to match any type

- `Subtype` 

- `Tileset` is a tileset identifier specified previously in `TILESET` command. Two predefined values are `map` for the main graphics font and `text` for the text font.

- `NewTile` is a new tile number

- `NewFg` is a new foreground colour, `1-16`

- `NewBg` is a new background colour, `1-16`

**Note:** Any of `NewTile`, `NewFg` and `NewBg` parameters may be empty to use existing values without changes, but at least one of them must be present. Trailing colons may be omitted for empty parameters. 

**Overrides for tile types**
    
    [OVERRIDE:Tile:T:Type:Tileset:NewTile:NewFg:NewBg]
    
- `Tile`, `Tileset`, `NewTile`, `NewFg` and `NewBg` parameters are the same as for items/buildings overrides

- `Type` is a [tile type](#tile-types). For convenience, you can use textual description that you see with live query mode of mousequery plugin, in this case include it in quotes (for example, "stone stair down").
    

## Multi-level Rendering ##

Copy `shadows.png` to `data/art` folder. Multi-level rendering is disabled by default, you can enable it with `multilevel` command (see below) manually or put it into your `dfhack.init` script.

**`multilevel`**  
Set number of *additional* levels to render. Possible parameters are `more`, `less` or number `0-15`.

**`multilevel shadowcolor <r> <g> <b> <a>`**  
Set shadow colour. Components are in range `0-1`. Default is `0 0 0 0.4`

**`multilevel fogcolor <r> <g> <b>`**  
Set fog colour. Default is `0.1 0.1 0.3`

**`multilevel fogdensity <density> [<start>] [<step>]`**  
Set fog density parameters. Default is `0.15 0 1`

## Additional Colours ##

Not yet.

## Colormap Manipulation ##

**`colormap <colorname> <r> <g> <b>`**  
Change display colours. Colour names are as in `init/colors.txt` without `_R/G/B` suffix. Components are in range `0-255`. If no new values are provided, current values are printed.

**`colormap reload`**  
Reload all colours from `init/colors.txt`.

## Mapshot! ##

*This feature is currently broken.*

**`mapshot`**  
Save an image of entire map in full size to `mapshot.tga` in your DF folder.

## Other Improvements ##

Trade screen divided equally on OS X (DF 0.34.11 only).

## Tables ##
Please note that values from these lists are **case-sensitive** when used in `OVERRIDE` command.

### Item IDs ###
This list comes from `df/items_other_id.h` include file in DFHack source code.

> AMMO, AMULET, ANIMALTRAP, ANVIL, ANY_ARMOR_GLOVES, ANY_ARMOR_HELM, ANY_ARMOR_PANTS, ANY_ARMOR_SHOES, ANY_ARTIFACT, ANY_AUTO_CLEAN, ANY_CAGE_OR_TRAP, ANY_CAN_ROT, ANY_COOKABLE, ANY_CORPSE, ANY_CRITTER, ANY_DEAD_DWARF, ANY_DRINK, ANY_EDIBLE_BONECARN, ANY_EDIBLE_CARNIVORE, ANY_EDIBLE_RAW, ANY_EDIBLE_VERMIN, ANY_EDIBLE_VERMIN_BOX, ANY_ENCASED, ANY_FURNITURE, ANY_GENERIC123, ANY_GENERIC23, ANY_GENERIC24, ANY_GENERIC35, ANY_GENERIC36, ANY_GENERIC37, ANY_GENERIC38, ANY_GENERIC82, ANY_GOOD_FOOD, ANY_IN_CONSTRUCTION, ANY_MELT_DESIGNATED, ANY_MURDERED, ANY_RECENTLY_DROPPED, ANY_REFUSE, ANY_SPIKE, ANY_TRUE_ARMOR, ANY_WEAPON, ANY_WEBS, ARMOR, ARMORSTAND, BACKPACK, BAD, BALLISTAARROWHEAD, BALLISTAPARTS, BAR, BARREL, BED, BIN, BLOCKS, BOOK, BOULDER, BOX, BRACELET, BUCKET, CABINET, CAGE, CATAPULTPARTS, CHAIN, CHAIR, CHEESE, CLOTH, COFFIN, COIN, CORPSE, CORPSEPIECE, CROWN, CRUTCH, DOOR, DRINK, EARRING, EGG, FIGURINE, FISH, FISH_RAW, FLASK, FLOODGATE, FOOD, FOOD_STORAGE, GEM, GLOB, GLOVES, GOBLET, GRATE, HATCH_COVER, HELM, INSTRUMENT, LEAVES *(0.34)*, LIQUID_MISC, MEAT, MILLSTONE, ORTHOPEDIC_CAST, PANTS, PET, PIPE_SECTION, PLANT, PLANT_GROWTH *(0.40)*, POWDER_MISC, QUERN, QUIVER, REMAINS, RING, ROCK, ROUGH, SCEPTER, SEEDS, SHIELD, SHOES, SIEGEAMMO, SKIN_TANNED, SLAB, SMALLGEM, SPLINT, STATUE, TABLE, THREAD, TOOL, TOTEM, TOY, TRACTION_BENCH, TRAPCOMP, TRAPPARTS, VERMIN, WEAPON, WEAPONRACK, WINDOW, WOOD

### Item Types ###
This list comes from `df/item_type.h` include file in DFHack source code.

> AMMO, AMULET, ANIMALTRAP, ANVIL, ARMOR, ARMORSTAND, BACKPACK, BALLISTAARROWHEAD, BALLISTAPARTS, BAR, BARREL, BED, BIN, BLOCKS, BOOK BOULDER, BOX, BRACELET, BUCKET, CABINET, CAGE, CATAPULTPARTS, CHAIN, CHAIR, CHEESE, CLOTH, COFFIN, COIN, CORPSE, CORPSEPIECE, CROWN, CRUTCH, DOOR, DRINK, EARRING, EGG, FIGURINE, FISH, FISH_RAW, FLASK, FLOODGATE, FOOD, GEM, GLOB, GLOVES, GOBLET, GRATE, HATCH_COVER, HELM, INSTRUMENT, LEAVES *(0.34)*, LIQUID_MISC, MEAT, MILLSTONE, ORTHOPEDIC_CAST, PANTS, PET, PIPE_SECTION, PLANT, PLANT_GROWTH *(0.40)*, POWDER_MISC, QUERN, QUIVER, REMAINS, RING, ROCK, ROUGH, SCEPTER, SEEDS, SHIELD, SHOES, SIEGEAMMO, SKIN_TANNED, SLAB, SMALLGEM, SPLINT, STATUE, TABLE, THREAD, TOOL, TOTEM, TOY, TRACTION_BENCH, TRAPCOMP, TRAPPARTS, VERMIN, WEAPON, WEAPONRACK, WINDOW, WOOD

### Building IDs ###
This list comes from `df/buildings_other_id.h` include file in DFHack source code.

> ACTIVITY_ZONE, ANY_ACTUAL, ANY_BARRACKS, ANY_HOSPITAL, ANY_HOSPITAL_STORAGE, ANY_MACHINE, ANY_NOBLE_ROOM, ANY_ROAD, ANY_STORAGE, ANY_ZONE, ARCHERY_TARGET, ARMOR_STAND, AXLE_HORIZONTAL, AXLE_VERTICAL, BARS_FLOOR, BARS_VERTICAL, BED, BOX, BRIDGE, CABINET, CAGE, CHAIN, CHAIR, COFFIN, DOOR, FARM_PLOT, FLOODGATE, FURNACE_ANY, FURNACE_CUSTOM, FURNACE_GLASS_ANY, FURNACE_KILN_ANY, FURNACE_SMELTER_ANY, FURNACE_SMELTER_MAGMA, FURNACE_WOOD, GEAR_ASSEMBLY, GRATE_FLOOR, GRATE_WALL, HATCH, HIVE, NEST, NEST_BOX, ROLLERS, SCREW_PUMP, SHOP, SLAB, STATUE, STOCKPILE, SUPPORT, TABLE, TRACTION_BENCH, TRADE_DEPOT, TRAP, WAGON, WATER_WHEEL, WEAPON_RACK, WEAPON_UPRIGHT WELL, WINDMILL, WINDOW_ANY, WORKSHOP_ANY, WORKSHOP_ASHERY, WORKSHOP_BOWYER, WORKSHOP_BUTCHER, WORKSHOP_CARPENTER, WORKSHOP_CLOTHIER, WORKSHOP_CRAFTSDWARF, WORKSHOP_CUSTOM, WORKSHOP_DYER, WORKSHOP_FARMER, WORKSHOP_FISHERY, WORKSHOP_FORGE_ANY, WORKSHOP_JEWELER, WORKSHOP_KENNEL, WORKSHOP_KITCHEN, WORKSHOP_LEATHER, WORKSHOP_LOOM, WORKSHOP_MAGMA_FORGE, WORKSHOP_MASON, WORKSHOP_MECHANIC, WORKSHOP_MILL_ANY, WORKSHOP_MILLSTONE, WORKSHOP_QUERN, WORKSHOP_SIEGE, WORKSHOP_STILL, WORKSHOP_TANNER, WORKSHOP_TOOL

### Building Types
This list comes from `df/building_type.h` include file in DFHack source code.

> AnimalTrap, ArcheryTarget, Armorstand, AxleHorizontal, AxleVertical, BarsFloor, BarsVertical, Bed, Box, Bridge, Cabinet, Cage, Chain, Chair, Civzone, Coffin, Construction, Door, FarmPlot, Floodgate, Furnace, GearAssembly, GrateFloor, GrateWall, Hatch, Hive, Nest, NestBox, RoadDirt, RoadPaved, Rollers ScrewPump, Shop, SiegeEngine, Slab, Statue, Stockpile, Support, Table, TractionBench, TradeDepot, Trap, Wagon, WaterWheel, Weapon, Weaponrack, Well, Windmill, WindowGem, WindowGlass, Workshop

### Tile Types ###
This list comes from `df/tiletype.h` include file in DFHack source code.

> Ashes1, Ashes2, Ashes3, 

> BrookE, BrookN, BrookNE, BrookNW, BrookS, BrookSE, BrookSW, BrookTop, BrookW, 

> *0.40:* BurningTreeBranches, BurningTreeCapFloor, BurningTreeCapRamp, BurningTreeCapWall, BurningTreeTrunk, BurningTreeTwigs, 

> Campfire, Chasm, 

> ConstructedFloor, ConstructedFloorTrackE, ConstructedFloorTrackEW, ConstructedFloorTrackN, ConstructedFloorTrackNE, ConstructedFloorTrackNEW, ConstructedFloorTrackNS, ConstructedFloorTrackNSE, ConstructedFloorTrackNSEW, ConstructedFloorTrackNSW, ConstructedFloorTrackNW, ConstructedFloorTrackS, ConstructedFloorTrackSE, ConstructedFloorTrackSEW, ConstructedFloorTrackSW, ConstructedFloorTrackW, ConstructedFortification, ConstructedPillar, ConstructedRamp, ConstructedRampTrackE, ConstructedRampTrackEW, ConstructedRampTrackN, ConstructedRampTrackNE, ConstructedRampTrackNEW, ConstructedRampTrackNS, ConstructedRampTrackNSE, ConstructedRampTrackNSEW ConstructedRampTrackNSW, ConstructedRampTrackNW, ConstructedRampTrackS, ConstructedRampTrackSE, ConstructedRampTrackSEW, ConstructedRampTrackSW, ConstructedRampTrackW, ConstructedStairD, ConstructedStairU, ConstructedStairUD, ConstructedWallL2D, ConstructedWallL2U, ConstructedWallLD, ConstructedWallLD2, ConstructedWallLR, ConstructedWallLRD, ConstructedWallLRU, ConstructedWallLRUD, ConstructedWallLU, ConstructedWallLU2, ConstructedWallLUD, ConstructedWallR2D, ConstructedWallR2U, ConstructedWallRD, ConstructedWallRD2, ConstructedWallRU, ConstructedWallRU2, ConstructedWallRUD, ConstructedWallUD, 

> Driftwood, EeriePit, 

> FeatureBoulder, FeatureFloor1, FeatureFloor2, FeatureFloor3, FeatureFloor4, FeatureFloorSmooth, FeatureFloorTrackE, FeatureFloorTrackEW, FeatureFloorTrackN, FeatureFloorTrackNE, FeatureFloorTrackNEW, FeatureFloorTrackNS, FeatureFloorTrackNSE, FeatureFloorTrackNSEW, FeatureFloorTrackNSW, FeatureFloorTrackNW, FeatureFloorTrackS, FeatureFloorTrackSE, FeatureFloorTrackSEW, FeatureFloorTrackSW, FeatureFloorTrackW, FeatureFortification, FeaturePebbles1, FeaturePebbles2, FeaturePebbles3, FeaturePebbles4, FeaturePillar, FeatureRamp, FeatureRampTrackE, FeatureRampTrackEW, FeatureRampTrackN, FeatureRampTrackNE, FeatureRampTrackNEW, FeatureRampTrackNS, FeatureRampTrackNSE, FeatureRampTrackNSEW, FeatureRampTrackNSW, FeatureRampTrackNW, FeatureRampTrackS, FeatureRampTrackSE, FeatureRampTrackSEW, FeatureRampTrackSW, FeatureRampTrackW, FeatureStairD, FeatureStairU, FeatureStairUD, FeatureWall, FeatureWallSmoothL2D, FeatureWallSmoothL2U, FeatureWallSmoothLD, FeatureWallSmoothLD2, FeatureWallSmoothLR, FeatureWallSmoothLRD, FeatureWallSmoothLRU, FeatureWallSmoothLRUD, FeatureWallSmoothLU, FeatureWallSmoothLU2, FeatureWallSmoothLUD, FeatureWallSmoothR2D, FeatureWallSmoothR2U, FeatureWallSmoothRD, FeatureWallSmoothRD2, FeatureWallSmoothRU, FeatureWallSmoothRU2, FeatureWallSmoothRUD, FeatureWallSmoothUD, FeatureWallWorn1, FeatureWallWorn2, FeatureWallWorn3, 

> Fire, 

> FrozenFloor1, FrozenFloor2, FrozenFloor3, FrozenFloor4, FrozenFloorSmooth, FrozenFloorTrackE, FrozenFloorTrackEW, FrozenFloorTrackN, FrozenFloorTrackNE, FrozenFloorTrackNEW, FrozenFloorTrackNS, FrozenFloorTrackNSE, FrozenFloorTrackNSEW, FrozenFloorTrackNSW, FrozenFloorTrackNW, FrozenFloorTrackS, FrozenFloorTrackSE, FrozenFloorTrackSEW, FrozenFloorTrackSW, FrozenFloorTrackW, FrozenFortification, FrozenPillar, FrozenRamp, FrozenRampTrackE, FrozenRampTrackEW, FrozenRampTrackN, FrozenRampTrackNE, FrozenRampTrackNEW, FrozenRampTrackNS, FrozenRampTrackNSE, FrozenRampTrackNSEW, FrozenRampTrackNSW, FrozenRampTrackNW, FrozenRampTrackS, FrozenRampTrackSE, FrozenRampTrackSEW, FrozenRampTrackSW, FrozenRampTrackW, FrozenStairD, FrozenStairU, FrozenStairUD, FrozenWall, FrozenWallSmoothL2D, FrozenWallSmoothL2U, FrozenWallSmoothLD, FrozenWallSmoothLD2, FrozenWallSmoothLR, FrozenWallSmoothLRD, FrozenWallSmoothLRU, FrozenWallSmoothLRUD, FrozenWallSmoothLU, FrozenWallSmoothLU2, FrozenWallSmoothLUD, FrozenWallSmoothR2D, FrozenWallSmoothR2U, FrozenWallSmoothRD, FrozenWallSmoothRD2, FrozenWallSmoothRU, FrozenWallSmoothRU2, FrozenWallSmoothRUD, FrozenWallSmoothUD, FrozenWallWorn1, FrozenWallWorn2, FrozenWallWorn3, 

> FurrowedSoil, 

> GlowingBarrier, GlowingFloor, 

> Grass1StairD, Grass1StairU, Grass1StairUD, Grass2StairD, Grass2StairU, Grass2StairUD, GrassDarkFloor1, GrassDarkFloor2, GrassDarkFloor3, GrassDarkFloor4, GrassDarkRamp, GrassDeadFloor1, GrassDeadFloor2, GrassDeadFloor3, GrassDeadFloor4, GrassDeadRamp, GrassDryFloor1, GrassDryFloor2, GrassDryFloor3, GrassDryFloor4, GrassDryRamp, GrassLightFloor1, GrassLightFloor2, GrassLightFloor3, GrassLightFloor4, GrassLightRamp, 

> LavaBoulder, LavaFloor1, LavaFloor2, LavaFloor3, LavaFloor4, LavaFloorSmooth, LavaFloorTrackE, LavaFloorTrackEW, LavaFloorTrackN, LavaFloorTrackNE, LavaFloorTrackNEW, LavaFloorTrackNS, LavaFloorTrackNSE, LavaFloorTrackNSEW, LavaFloorTrackNSW, LavaFloorTrackNW, LavaFloorTrackS, LavaFloorTrackSE, LavaFloorTrackSEW, LavaFloorTrackSW, LavaFloorTrackW, LavaFortification, LavaPebbles1, LavaPebbles2, LavaPebbles3, LavaPebbles4, LavaPillar, LavaRamp, LavaRampTrackE, LavaRampTrackEW, LavaRampTrackN, LavaRampTrackNE, LavaRampTrackNEW, LavaRampTrackNS, LavaRampTrackNSE, LavaRampTrackNSEW, LavaRampTrackNSW, LavaRampTrackNW, LavaRampTrackS, LavaRampTrackSE, LavaRampTrackSEW, LavaRampTrackSW, LavaRampTrackW, LavaStairD, LavaStairU, LavaStairUD, LavaWall, LavaWallSmoothL2D, LavaWallSmoothL2U, LavaWallSmoothLD, LavaWallSmoothLD2, LavaWallSmoothLR, LavaWallSmoothLRD, LavaWallSmoothLRU, LavaWallSmoothLRUD, LavaWallSmoothLU, LavaWallSmoothLU2, LavaWallSmoothLUD, LavaWallSmoothR2D, LavaWallSmoothR2U, LavaWallSmoothRD, LavaWallSmoothRD2, LavaWallSmoothRU, LavaWallSmoothRU2, LavaWallSmoothRUD, LavaWallSmoothUD, LavaWallWorn1, LavaWallWorn2, LavaWallWorn3, 

> MagmaFlow, 

> MineralBoulder, MineralFloor1, MineralFloor2, MineralFloor3, MineralFloor4, MineralFloorSmooth, MineralFloorTrackE, MineralFloorTrackEW, MineralFloorTrackN, MineralFloorTrackNE, MineralFloorTrackNEW, MineralFloorTrackNS, MineralFloorTrackNSE, MineralFloorTrackNSEW, MineralFloorTrackNSW, MineralFloorTrackNW, MineralFloorTrackS, MineralFloorTrackSE, MineralFloorTrackSEW, MineralFloorTrackSW, MineralFloorTrackW, MineralFortification, MineralPebbles1, MineralPebbles2, MineralPebbles3, MineralPebbles4, MineralPillar, MineralRamp, MineralRampTrackE, MineralRampTrackEW, MineralRampTrackN, MineralRampTrackNE, MineralRampTrackNEW, MineralRampTrackNS, MineralRampTrackNSE, MineralRampTrackNSEW, MineralRampTrackNSW, MineralRampTrackNW, MineralRampTrackS, MineralRampTrackSE, MineralRampTrackSEW, MineralRampTrackSW, MineralRampTrackW, MineralStairD, MineralStairU, MineralStairUD, MineralWall, MineralWallSmoothL2D, MineralWallSmoothL2U, MineralWallSmoothLD, MineralWallSmoothLD2, MineralWallSmoothLR, MineralWallSmoothLRD, MineralWallSmoothLRU, MineralWallSmoothLRUD, MineralWallSmoothLU, MineralWallSmoothLU2, MineralWallSmoothLUD, MineralWallSmoothR2D, MineralWallSmoothR2U, MineralWallSmoothRD, MineralWallSmoothRD2, MineralWallSmoothRU, MineralWallSmoothRU2, MineralWallSmoothRUD, MineralWallSmoothUD, MineralWallWorn1, MineralWallWorn2, MineralWallWorn3, 

> MurkyPool, MurkyPoolRamp, OpenSpace, RampTop, 

> RiverE, RiverN, RiverNE, RiverNW, RiverRampE, RiverRampN, RiverRampNE, RiverRampNW, RiverRampS, RiverRampSE, RiverRampSW, RiverRampW, RiverS, RiverSE, RiverSource, RiverSW, RiverW, 

> Sapling, SaplingDead, SemiMoltenRock, Shrub, ShrubDead, 

> SoilFloor1, SoilFloor2, SoilFloor3, SoilFloor4, SoilRamp, SoilStairD, SoilStairU, SoilStairUD, SoilWall, SoilWetFloor1, SoilWetFloor2, SoilWetFloor3, SoilWetFloor4, 

> StoneBoulder, StoneFloor1, StoneFloor2, StoneFloor3, StoneFloor4, StoneFloorSmooth, StoneFloorTrackE, StoneFloorTrackEW, StoneFloorTrackN, StoneFloorTrackNE, StoneFloorTrackNEW, StoneFloorTrackNS, StoneFloorTrackNSE, StoneFloorTrackNSEW, StoneFloorTrackNSW, StoneFloorTrackNW, StoneFloorTrackS, StoneFloorTrackSE, StoneFloorTrackSEW, StoneFloorTrackSW, StoneFloorTrackW, StoneFortification, StonePebbles1, StonePebbles2, StonePebbles3, StonePebbles4, StonePillar, StoneRamp, StoneRampTrackE, StoneRampTrackEW, StoneRampTrackN, StoneRampTrackNE, StoneRampTrackNEW, StoneRampTrackNS, StoneRampTrackNSE, StoneRampTrackNSEW, StoneRampTrackNSW, StoneRampTrackNW, StoneRampTrackS, StoneRampTrackSE, StoneRampTrackSEW, StoneRampTrackSW, StoneRampTrackW, StoneStairD, StoneStairU, StoneStairUD, StoneWall, StoneWallSmoothL2D, StoneWallSmoothL2U, StoneWallSmoothLD, StoneWallSmoothLD2, StoneWallSmoothLR, StoneWallSmoothLRD, StoneWallSmoothLRU, StoneWallSmoothLRUD, StoneWallSmoothLU, StoneWallSmoothLU2, StoneWallSmoothLUD, StoneWallSmoothR2D, StoneWallSmoothR2U, StoneWallSmoothRD, StoneWallSmoothRD2, StoneWallSmoothRU, StoneWallSmoothRU2, StoneWallSmoothRUD, StoneWallSmoothUD, StoneWallWorn1, StoneWallWorn2, StoneWallWorn3, 

> *0.34:* Tree, TreeDead,

> *0.40:* TreeBranches, TreeBranchesSmooth, TreeBranchEW, TreeBranchNE, TreeBranchNEW, TreeBranchNS, TreeBranchNSE, TreeBranchNSEW, TreeBranchNSW, TreeBranchNW, TreeBranchSE, TreeBranchSEW, TreeBranchSW, TreeCapFloor1, TreeCapFloor2, TreeCapFloor3, TreeCapFloor4, TreeCapPillar, TreeCapRamp, TreeCapWallE, TreeCapWallN, TreeCapWallNE, TreeCapWallNW, TreeCapWallS, TreeCapWallSE, TreeCapWallSW, TreeCapWallW, TreeDeadBranches, TreeDeadBranchesSmooth, TreeDeadBranchEW, TreeDeadBranchNE, TreeDeadBranchNEW, TreeDeadBranchNS, TreeDeadBranchNSE, TreeDeadBranchNSEW, TreeDeadBranchNSW, TreeDeadBranchNW, TreeDeadBranchSE, TreeDeadBranchSEW, TreeDeadBranchSW, TreeDeadCapFloor1, TreeDeadCapFloor2, TreeDeadCapFloor3, TreeDeadCapFloor4, TreeDeadCapPillar, TreeDeadCapRamp, TreeDeadCapWallE, TreeDeadCapWallN, TreeDeadCapWallNE, TreeDeadCapWallNW, TreeDeadCapWallS, TreeDeadCapWallSE, TreeDeadCapWallSW, TreeDeadCapWallW, TreeDeadRoots, TreeDeadRootSloping, TreeDeadTrunkBranchE, TreeDeadTrunkBranchN, TreeDeadTrunkBranchS, TreeDeadTrunkBranchW, TreeDeadTrunkE, TreeDeadTrunkEW, TreeDeadTrunkInterior, TreeDeadTrunkN, TreeDeadTrunkNE, TreeDeadTrunkNEW, TreeDeadTrunkNS, TreeDeadTrunkNSE, TreeDeadTrunkNSEW, TreeDeadTrunkNSW, TreeDeadTrunkNW, TreeDeadTrunkPillar, TreeDeadTrunkS, TreeDeadTrunkSE, TreeDeadTrunkSEW, TreeDeadTrunkSloping, TreeDeadTrunkSW, TreeDeadTrunkW, TreeDeadTwigs, TreeRoots, TreeRootSloping, TreeTrunkBranchE, TreeTrunkBranchN, TreeTrunkBranchS, TreeTrunkBranchW, TreeTrunkE, TreeTrunkEW, TreeTrunkInterior, TreeTrunkN, TreeTrunkNE, TreeTrunkNEW, TreeTrunkNS, TreeTrunkNSE, TreeTrunkNSEW, TreeTrunkNSW, TreeTrunkNW, TreeTrunkPillar, TreeTrunkS, TreeTrunkSE, TreeTrunkSEW, TreeTrunkSloping, TreeTrunkSW, TreeTrunkW, TreeTwigs, 

> *0.40:* UnderworldGateStairD, UnderworldGateStairU, UnderworldGateStairUD, 

> Void, Waterfall
