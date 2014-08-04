# Text Will Be Text #

This is a plugin for [Dwarf Fortress](http://bay12games.com) / [DFHack](http://github.com/dfhack/dfhack) that improves various aspects the game interface.

Originally I wrote a small plugin because I was tired seeing coffins instead of zeroes and all that stuff. It has greately evolved since then. Requires OpenGL PRINT_MODE (STANDARD or VBO). Some functions may not work or have issues in Adventurer Mode.

Our primary goal is to improve Fortress Mode

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

[Home Page / Sources](https://github.com/mifki/df-twbt) -- [Latest Release](https://github.com/mifki/df-twbt/releases) -- [Discussion](http://www.bay12forums.com/smf/index.php?topic=138754.0) -- [Report an Issue](https://github.com/mifki/df-twbt/issues)

## Text and Map Tilesets ##

Main function is to use separate fonts (tilesets) for map tiles in Fortress Mode and for text. These fonts may have different tile size, usually square for the map and non-square for text.

In your `init.txt` set FONT and FULLFONT to the font you want to use for **text**, and GRAPHICS\_FONT and GRAPHICS\_FULLFONT to the font for **map tiles**.

`twbt tilesize` command adjusts the size of map tiles. Possible parameters are `bigger`, `smaller` or exact values `<width> <height>`.

## Overrides ##

Commands described below are to be placed in `data/init/overrides.txt` file and allow to change tile images used for specific buildings, items and tile types.

**Loading additional tilesets**

    [TILESET:font.png:fullscreenfont.png:Id] 

- File names are relative to the `data/art` folder

- `Id` is an arbitrary string to refer this tileset later

**Overrides for items and buildings**
    
    [OVERRIDE:Tile:Kind:Id:Type:Subtype:Tileset:NewTile]
    
- `Tile` is the original tile number

- `Kind` is `B` for building or `I` for item

- `Id` is an [item ID](#item-ids) or [building ID](#building-ids), or empty to match any ID

- `Type` is an [item type](#item-types) or [building type](#building-types), or empty to match any type

- `Subtype` 

- `Tileset` is a tileset identifier specified previously in `TILESET` command. Two predefined values are `map` for the main graphics font and `text` for the text font.

- `NewTile` is a new tile number

**Overrides for tile types**
    
    [OVERRIDE:Tile:T:Type:Tileset:NewTile]
    
- `Tile`, `Tileset` and `NewTile` parameters are the same as for items/buildings overrides

- `Type` is a [tile type](#tile-types). For convenience, you can use textual description that you see with live query mode of mousequery plugin, in this case include it in quotes (for example, "stone stair down").
    

## Multi-level Rendering ##

Copy `shadows.png` to `data/art` folder. Multi-level rendering is disabled by default, you can enable it manually with `multilevel` command (see below), or put it into your `dfhack.init` script.

**`multilevel`**  
Set number of *additional* levels to render. Possible parameters are `more`, `less` or number `0-15`.

**`multilevel shadowcolor <r> <g> <b> <a>`**  
Set shadow colour. Components are in range `0-1`. Default is `0 0 0 0.4`

**`multilevel fogcolor <r> <g> <b>`**  
Set fog colour. Default is `0.1 0.1 0.3`

**`multilevel fogdensity <d>`**  
Set fog density. Default is `0.15`

## Additional Colours ##

Not yet.

## Colormap Manipulation ##

**`colormap <colorname> <r> <g> <b>`**  
Change display colours. Colour names are as in `init/colors.txt` without `_R/G/B` suffix. Components are in range `0-255`. If no new values are provided, current values are printed.

**`colormap reload`**  
Reload all colours from `init/colors.txt`.

## Mapshot! ##

**`mapshot`**  
Save an image of entire map in full size to `mapshot.tga` in your DF folder.

## Other Improvements ##

Trade screen divided equally on OS X.

## Tables ##
Please note that values from these lists are **case-sensitive** when used in `OVERRIDE` command.

### Item IDs ###
This list comes from `df/items_other_id.h` include file in DFHack source code.

> AMMO, AMULET, ANIMALTRAP, ANVIL, ANY_ARMOR_GLOVES, ANY_ARMOR_HELM, ANY_ARMOR_PANTS, ANY_ARMOR_SHOES, ANY_ARTIFACT, ANY_AUTO_CLEAN, ANY_CAGE_OR_TRAP, ANY_CAN_ROT, ANY_COOKABLE, ANY_CORPSE, ANY_CRITTER, ANY_DEAD_DWARF, ANY_DRINK, ANY_EDIBLE_BONECARN, ANY_EDIBLE_CARNIVORE, ANY_EDIBLE_RAW, ANY_EDIBLE_VERMIN, ANY_EDIBLE_VERMIN_BOX, ANY_ENCASED, ANY_FURNITURE, ANY_GENERIC123, ANY_GENERIC23, ANY_GENERIC24, ANY_GENERIC35, ANY_GENERIC36, ANY_GENERIC37, ANY_GENERIC38, ANY_GENERIC82, ANY_GOOD_FOOD, ANY_IN_CONSTRUCTION, ANY_MELT_DESIGNATED, ANY_MURDERED, ANY_RECENTLY_DROPPED, ANY_REFUSE, ANY_SPIKE, ANY_TRUE_ARMOR, ANY_WEAPON, ANY_WEBS, ARMOR, ARMORSTAND, BACKPACK, BAD BALLISTAARROWHEAD, BALLISTAPARTS, BAR, BARREL, BED, BIN, BLOCKS, BOOK, BOULDER, BOX, BRACELET, BUCKET, CABINET, CAGE, CATAPULTPARTS, CHAIN, CHAIR, CHEESE, CLOTH, COFFIN, COIN, CORPSE, CORPSEPIECE, CROWN, CRUTCH, DOOR, DRINK, EARRING, EGG, FIGURINE, FISH, FISH_RAW, FLASK, FLOODGATE, FOOD, FOOD_STORAGE, GEM, GLOB, GLOVES, GOBLET, GRATE, HATCH_COVER, HELM, INSTRUMENT, LEAVES, LIQUID_MISC, MEAT, MILLSTONE, ORTHOPEDIC_CAST, PANTS, PET, PIPE_SECTION, PLANT, POWDER_MISC, QUERN, QUIVER, REMAINS, RING, ROCK, ROUGH, SCEPTER, SEEDS, SHIELD, SHOES, SIEGEAMMO, SKIN_TANNED, SLAB, SMALLGEM, SPLINT, STATUE, TABLE, THREAD, TOOL, TOTEM, TOY, TRACTION_BENCH, TRAPCOMP, TRAPPARTS, VERMIN, WEAPON, WEAPONRACK, WINDOW, WOOD,

### Item Types ###
This list comes from `df/item_type.h` include file in DFHack source code.

> AMMO, AMULET, ANIMALTRAP, ANVIL, ARMOR, ARMORSTAND, BACKPACK, BALLISTAARROWHEAD, BALLISTAPARTS, BAR, BARREL, BED, BIN, BLOCKS, BOOK BOULDER, BOX, BRACELET, BUCKET, CABINET, CAGE, CATAPULTPARTS, CHAIN, CHAIR, CHEESE, CLOTH, COFFIN, COIN, CORPSE, CORPSEPIECE, CROWN, CRUTCH, DOOR, DRINK, EARRING, EGG, FIGURINE, FISH, FISH_RAW, FLASK, FLOODGATE, FOOD, GEM, GLOB, GLOVES, GOBLET, GRATE, HATCH_COVER, HELM, INSTRUMENT, LEAVES, LIQUID_MISC, MEAT, MILLSTONE, ORTHOPEDIC_CAST, PANTS, PET, PIPE_SECTION, PLANT, POWDER_MISC, QUERN, QUIVER, REMAINS, RING, ROCK, ROUGH, SCEPTER, SEEDS, SHIELD, SHOES, SIEGEAMMO, SKIN_TANNED, SLAB, SMALLGEM, SPLINT, STATUE, TABLE, THREAD, TOOL, TOTEM, TOY, TRACTION_BENCH, TRAPCOMP, TRAPPARTS, VERMIN, WEAPON, WEAPONRACK, WINDOW, WOOD,

### Building IDs ###
This list comes from `df/buildings_other_id.h` include file in DFHack source code.

> ACTIVITY_ZONE, ANY_ACTUAL, ANY_BARRACKS, ANY_HOSPITAL, ANY_HOSPITAL_STORAGE, ANY_MACHINE, ANY_NOBLE_ROOM, ANY_ROAD, ANY_STORAGE, ANY_ZONE, ARCHERY_TARGET, ARMOR_STAND, AXLE_HORIZONTAL, AXLE_VERTICAL, BARS_FLOOR, BARS_VERTICAL, BED, BOX, BRIDGE, CABINET, CAGE, CHAIN, CHAIR, COFFIN, DOOR, FARM_PLOT, FLOODGATE, FURNACE_ANY, FURNACE_CUSTOM, FURNACE_GLASS_ANY, FURNACE_KILN_ANY, FURNACE_SMELTER_ANY, FURNACE_SMELTER_MAGMA, FURNACE_WOOD, GEAR_ASSEMBLY, GRATE_FLOOR, GRATE_WALL, HATCH, HIVE, NEST, NEST_BOX, ROLLERS, SCREW_PUMP, SHOP, SLAB, STATUE, STOCKPILE, SUPPORT, TABLE, TRACTION_BENCH, TRADE_DEPOT, TRAP, WAGON, WATER_WHEEL, WEAPON_RACK, WEAPON_UPRIGHT WELL, WINDMILL, WINDOW_ANY, WORKSHOP_ANY, WORKSHOP_ASHERY, WORKSHOP_BOWYER, WORKSHOP_BUTCHER, WORKSHOP_CARPENTER, WORKSHOP_CLOTHIER, WORKSHOP_CRAFTSDWARF, WORKSHOP_CUSTOM, WORKSHOP_DYER, WORKSHOP_FARMER, WORKSHOP_FISHERY, WORKSHOP_FORGE_ANY, WORKSHOP_JEWELER, WORKSHOP_KENNEL, WORKSHOP_KITCHEN, WORKSHOP_LEATHER, WORKSHOP_LOOM, WORKSHOP_MAGMA_FORGE, WORKSHOP_MASON, WORKSHOP_MECHANIC, WORKSHOP_MILL_ANY, WORKSHOP_MILLSTONE, WORKSHOP_QUERN, WORKSHOP_SIEGE, WORKSHOP_STILL, WORKSHOP_TANNER, WORKSHOP_TOOL,

### Building Types
This list comes from `df/building_type.h` include file in DFHack source code.

> AnimalTrap, ArcheryTarget, Armorstand, AxleHorizontal, AxleVertical, BarsFloor, BarsVertical, Bed, Box, Bridge, Cabinet, Cage, Chain, Chair, Civzone, Coffin, Construction, Door, FarmPlot, Floodgate, Furnace, GearAssembly, GrateFloor, GrateWall, Hatch, Hive, Nest, NestBox, RoadDirt, RoadPaved, Rollers ScrewPump, Shop, SiegeEngine, Slab, Statue, Stockpile, Support, Table, TractionBench, TradeDepot, Trap, Wagon, WaterWheel, Weapon, Weaponrack, Well, Windmill, WindowGem, WindowGlass, Workshop,

### Tile Types ###
This list comes from `df/tiletype.h` include file in DFHack source code.

> Void, RampTop, MurkyPool, MurkyPoolRamp, 
>
> Tree, Shrub, Sapling, TreeDead, ShrubDead, SaplingDead, Driftwood
> 
> OpenSpace, Chasm, EeriePit
> 
> FrozenStairUD, FrozenStairD, FrozenStairU
>
> LavaStairUD, LavaStairD, LavaStairU, SoilStairUD, SoilStairD, SoilStairU
>
> MineralStairUD, MineralStairD, MineralStairU
>
> FeatureStairUD, FeatureStairD, FeatureStairU
> 
> StoneSmooth, LavaFloorSmooth, FeatureFloorSmooth, MineralFloorSmooth, FrozenFloorSmooth
> 
> Grass1StairUD, Grass1StairD, Grass1StairU, Grass2StairUD, Grass2StairD, Grass2StairU,  StoneStairUD, StoneStairD, StoneStairU
> 
> 
> StonePillar, LavaPillar, FeaturePillar, MineralPillar, FrozenPillar, ConstructedPillar
> 
> Waterfall, RiverSource
> 
> StoneWallWorn1, StoneWallWorn2, StoneWallWorn3, StoneWall
> 
> GrassDryRamp, GrassDeadRamp, GrassLightRamp, GrassDarkRamp, StoneRamp,
> LavaRamp, FeatureRamp, MineralRamp, SoilRamp, FrozenRamp, ConstructedRamp
> 
> Campfire, Fire, Ashes1, Ashes2, Ashes3
> 
> FrozenFloor2, FrozenFloor3, FrozenFloor4, FurrowedSoil, FrozenFloor1
> 
> SemiMoltenRock, MagmaFlow, SoilWall, GlowingBarrier, GlowingFloor
> 
> LavaWallSmoothRD2, LavaWallSmoothR2D, LavaWallSmoothR2U, LavaWallSmoothRU2, LavaWallSmoothL2U, LavaWallSmoothLU2, LavaWallSmoothL2D, LavaWallSmoothLD2, LavaWallSmoothLRUD, LavaWallSmoothRUD, LavaWallSmoothLRD, LavaWallSmoothLRU, LavaWallSmoothLUD, LavaWallSmoothRD, LavaWallSmoothRU, LavaWallSmoothLU, LavaWallSmoothLD, LavaWallSmoothUD, LavaWallSmoothLR,
> 
> FeatureWallSmoothRD2, FeatureWallSmoothR2D, FeatureWallSmoothR2U, FeatureWallSmoothRU2, FeatureWallSmoothL2U, FeatureWallSmoothLU2, FeatureWallSmoothL2D, FeatureWallSmoothLD2, FeatureWallSmoothLRUD, FeatureWallSmoothRUD, FeatureWallSmoothLRD, FeatureWallSmoothLRU, FeatureWallSmoothLUD, FeatureWallSmoothRD, FeatureWallSmoothRU, FeatureWallSmoothLU, FeatureWallSmoothLD, FeatureWallSmoothUD, FeatureWallSmoothLR
> 
> StoneWallSmoothRD2, StoneWallSmoothR2D, StoneWallSmoothR2U, StoneWallSmoothRU2, StoneWallSmoothL2U, StoneWallSmoothLU2, StoneWallSmoothL2D, StoneWallSmoothLD2, StoneWallSmoothLRUD, StoneWallSmoothRUD, StoneWallSmoothLRD, StoneWallSmoothLRU, StoneWallSmoothLUD, StoneWallSmoothRD, StoneWallSmoothRU, StoneWallSmoothLU, StoneWallSmoothLD, StoneWallSmoothUD, StoneWallSmoothLR
> 
> StoneFortification, LavaFortification, FeatureFortification, FrozenFortification, MineralFortification, ConstructedFortification
> 
> LavaWallWorn1, LavaWallWorn2, LavaWallWorn3, LavaWall
> 
> FeatureWallWorn1, FeatureWallWorn2, FeatureWallWorn3, FeatureWall
> 
> StoneFloor1, StoneFloor2, StoneFloor3, StoneFloor4
> 
> LavaFloor1, LavaFloor2, LavaFloor3, LavaFloor4
> 
> FeatureFloor1, FeatureFloor2, FeatureFloor3, FeatureFloor4
> 
> GrassDarkFloor1, GrassDarkFloor2, GrassDarkFloor3, GrassDarkFloor4
> 
> SoilFloor1, SoilFloor2, SoilFloor3, SoilFloor4
> 
> SoilWetFloor1, SoilWetFloor2, SoilWetFloor3, SoilWetFloor4
> 
> FrozenWallWorn1, FrozenWallWorn2, FrozenWallWorn3, FrozenWall
> 
> RiverN, RiverS, RiverE, RiverW, RiverNW, RiverNE, RiverSW, RiverSE
> 
> BrookN, BrookS, BrookE, BrookW, BrookNW, BrookNE, BrookSW, BrookSE, BrookTop, 
> 
> GrassDryFloor1, GrassDryFloor2, GrassDryFloor3, GrassDryFloor4
> 
> GrassDeadFloor1, GrassDeadFloor2, GrassDeadFloor3, GrassDeadFloor4
> 
> GrassLightFloor1, GrassLightFloor2, GrassLightFloor3, GrassLightFloor4
> 
> StoneBoulder, LavaBoulder, FeatureBoulder
> 
> StonePebbles1, StonePebbles2, StonePebbles3, StonePebbles4
> 
> LavaPebbles1, LavaPebbles2, LavaPebbles3, LavaPebbles4
> 
> FeaturePebbles1, FeaturePebbles2, FeaturePebbles3, FeaturePebbles4
> 
> MineralWallSmoothRD2, MineralWallSmoothR2D, MineralWallSmoothR2U, MineralWallSmoothRU2, MineralWallSmoothL2U, MineralWallSmoothLU2, MineralWallSmoothL2D, MineralWallSmoothLD2, MineralWallSmoothLRUD, MineralWallSmoothRUD, MineralWallSmoothLRD, MineralWallSmoothLRU, MineralWallSmoothLUD, MineralWallSmoothRD, MineralWallSmoothRU, MineralWallSmoothLU, MineralWallSmoothLD, MineralWallSmoothUD, MineralWallSmoothLR
> 
> 
> MineralWallWorn1, MineralWallWorn2, MineralWallWorn3, MineralWall
> 
> MineralFloor1, MineralFloor2, MineralFloor3, MineralFloor4
> 
> MineralBoulder, MineralPebbles1, MineralPebbles2, MineralPebbles3,
> MineralPebbles4, 
> 
> FrozenWallSmoothRD2, FrozenWallSmoothR2D, FrozenWallSmoothR2U, FrozenWallSmoothRU2, FrozenWallSmoothL2U, FrozenWallSmoothLU2, FrozenWallSmoothL2D, FrozenWallSmoothLD2, FrozenWallSmoothLRUD, FrozenWallSmoothRUD, FrozenWallSmoothLRD, FrozenWallSmoothLRU, FrozenWallSmoothLUD, FrozenWallSmoothRD, FrozenWallSmoothRU, FrozenWallSmoothLU, FrozenWallSmoothLD, FrozenWallSmoothUD, FrozenWallSmoothLR
> 
> RiverRampN, RiverRampS, RiverRampE, RiverRampW, RiverRampNW,
> RiverRampNE, RiverRampSW, RiverRampSE
> 
> ConstructedFloor
> 
> ConstructedWallRD2, ConstructedWallR2D, ConstructedWallR2U, ConstructedWallRU2, ConstructedWallL2U, ConstructedWallLU2, ConstructedWallL2D, ConstructedWallLD2, ConstructedWallLRUD, ConstructedWallRUD, ConstructedWallLRD, ConstructedWallLRU, ConstructedWallLUD, ConstructedWallRD, ConstructedWallRU, ConstructedWallLU, ConstructedWallLD, ConstructedWallUD, ConstructedWallLR
> 
> ConstructedStairUD, ConstructedStairD, ConstructedStairU
> 
> StoneFloorTrackN, StoneFloorTrackS, StoneFloorTrackE, StoneFloorTrackW, StoneFloorTrackNS, StoneFloorTrackNE, StoneFloorTrackNW, StoneFloorTrackSE, StoneFloorTrackSW, StoneFloorTrackEW, StoneFloorTrackNSE, StoneFloorTrackNSW, StoneFloorTrackNEW, StoneFloorTrackSEW, StoneFloorTrackNSEW
> 
> LavaFloorTrackN, LavaFloorTrackS, LavaFloorTrackE, LavaFloorTrackW, LavaFloorTrackNS, LavaFloorTrackNE, LavaFloorTrackNW, LavaFloorTrackSE, LavaFloorTrackSW, LavaFloorTrackEW, LavaFloorTrackNSE, LavaFloorTrackNSW, LavaFloorTrackNEW, LavaFloorTrackSEW, LavaFloorTrackNSEW
> 
> FeatureFloorTrackN, FeatureFloorTrackS, FeatureFloorTrackE, FeatureFloorTrackW, FeatureFloorTrackNS, FeatureFloorTrackNE, FeatureFloorTrackNW, FeatureFloorTrackSE, FeatureFloorTrackSW, FeatureFloorTrackEW, FeatureFloorTrackNSE, FeatureFloorTrackNSW, FeatureFloorTrackNEW, FeatureFloorTrackSEW, FeatureFloorTrackNSEW
> 
> MineralFloorTrackN, MineralFloorTrackS, MineralFloorTrackE, MineralFloorTrackW, MineralFloorTrackNS, MineralFloorTrackNE, MineralFloorTrackNW, MineralFloorTrackSE, MineralFloorTrackSW, MineralFloorTrackEW, MineralFloorTrackNSE, MineralFloorTrackNSW, MineralFloorTrackNEW, MineralFloorTrackSEW, MineralFloorTrackNSEW
> 
> FrozenFloorTrackN, FrozenFloorTrackS, FrozenFloorTrackE, FrozenFloorTrackW, FrozenFloorTrackNS, FrozenFloorTrackNE, FrozenFloorTrackNW, FrozenFloorTrackSE, FrozenFloorTrackSW, FrozenFloorTrackEW, FrozenFloorTrackNSE, FrozenFloorTrackNSW, FrozenFloorTrackNEW, FrozenFloorTrackSEW, FrozenFloorTrackNSEW
> 
> ConstructedFloorTrackN, ConstructedFloorTrackS, ConstructedFloorTrackE, ConstructedFloorTrackW, ConstructedFloorTrackNS, ConstructedFloorTrackNE, ConstructedFloorTrackNW, ConstructedFloorTrackSE, ConstructedFloorTrackSW, ConstructedFloorTrackEW, ConstructedFloorTrackNSE, ConstructedFloorTrackNSW, ConstructedFloorTrackNEW, ConstructedFloorTrackSEW, ConstructedFloorTrackNSEW, 
> 
> StoneRampTrackN, StoneRampTrackS, StoneRampTrackE, StoneRampTrackW, StoneRampTrackNS, StoneRampTrackNE, StoneRampTrackNW, StoneRampTrackSE, StoneRampTrackSW, StoneRampTrackEW, StoneRampTrackNSE, StoneRampTrackNSW, StoneRampTrackNEW, StoneRampTrackSEW, StoneRampTrackNSEW
> 
> LavaRampTrackN, LavaRampTrackS, LavaRampTrackE, LavaRampTrackW, LavaRampTrackNS, LavaRampTrackNE, LavaRampTrackNW, LavaRampTrackSE, LavaRampTrackSW, LavaRampTrackEW, LavaRampTrackNSE, LavaRampTrackNSW, LavaRampTrackNEW, LavaRampTrackSEW, LavaRampTrackNSEW, 
> 
> FeatureRampTrackN, FeatureRampTrackS, FeatureRampTrackE, FeatureRampTrackW, FeatureRampTrackNS, FeatureRampTrackNE, FeatureRampTrackNW, FeatureRampTrackSE, FeatureRampTrackSW, FeatureRampTrackEW, FeatureRampTrackNSE, FeatureRampTrackNSW, FeatureRampTrackNEW, FeatureRampTrackSEW, FeatureRampTrackNSEW
> 
> MineralRampTrackN, MineralRampTrackS, MineralRampTrackE, MineralRampTrackW, MineralRampTrackNS, MineralRampTrackNE, MineralRampTrackNW, MineralRampTrackSE, MineralRampTrackSW, MineralRampTrackEW, MineralRampTrackNSE, MineralRampTrackNSW, MineralRampTrackNEW, MineralRampTrackSEW, MineralRampTrackNSEW
> 
> FrozenRampTrackN, FrozenRampTrackS, FrozenRampTrackE, FrozenRampTrackW, FrozenRampTrackNS, FrozenRampTrackNE, FrozenRampTrackNW, FrozenRampTrackSE, FrozenRampTrackSW, FrozenRampTrackEW, FrozenRampTrackNSE, FrozenRampTrackNSW, FrozenRampTrackNEW, FrozenRampTrackSEW, FrozenRampTrackNSEW
> 
> ConstructedRampTrackN, ConstructedRampTrackS, ConstructedRampTrackE, ConstructedRampTrackW, ConstructedRampTrackNS, ConstructedRampTrackNE, ConstructedRampTrackNW, ConstructedRampTrackSE, ConstructedRampTrackSW, ConstructedRampTrackEW, ConstructedRampTrackNSE, ConstructedRampTrackNSW, ConstructedRampTrackNEW, ConstructedRampTrackSEW, ConstructedRampTrackNSEW