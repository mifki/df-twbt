# Text Will Be Text #

This is a plugin for [Dwarf Fortress](http://bay12games.com) / [DFHack](http://github.com/dfhack/dfhack) that improves various aspects the game interface.

Originally I wrote a small plugin because I was tired seeing coffins instead of zeroes and all that stuff. It has greately evolved since then. The primary goal is to improve Fortress Mode. Adventure Mode is generally supported but may have issues.

*Note:* To activate the plugin, set PRINT_MODE to TWBT (or TWBT_LEGACY for legacy mode with text and map tiles of the same size) in your `data/init/init.txt` file. The installation package also includes several other plugins which required changes to be compatible with TWBT.

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

## Building Overrides ##

Place `<override>.png` files in `data/art/tiles` folder. Possible file names listed below.

&nbsp;|&nbsp;
---|---
Archery Target|archerytarget
Armor Stand|armorstand
Floor Bars|bars_floor
Vertical Bars|bars_vertical
Bed|bed<br>bed-dorm
Box|box
Cabinet|cabinet
Cage|cage
Chain|chain
Chair|chair
Door|door<br>door-locked
Floodgate|floodgate
Floor Grate|grate_floor
Wall Grate|grate_wall
Hatch|hatch
Hive|hive
Next Box|nestbox<br>nestbox-claimed
Slab|slab
Statue|statue
Support|support
Table|table
Traction Bench|tractionbench
Weapon Rack|weaponrack
Well|well
Gem Window|window_gem
Glass Window|window_glass


## Tile Overrides ##

Commands described below are to be placed in `data/init/overrides.txt` file and allow to change tile images used for specific buildings, items and tile types.

**Loading additional tilesets**

    [TILESET:font.png:fullscreenfont.png:Id] 

- File names are relative to the `data/art` folder

- `Id` is an arbitrary string to refer this tileset later

**Overrides for tile types**
    
    [OVERRIDE:Tile:T:Type:Tileset:NewTile:NewFg:NewBg]
    
- `Tile` is the original tile number

- `Type` is a [tile type](#tile-types). For convenience, you can use textual description that you see with live query mode of mousequery plugin, in this case include it in quotes (for example, "stone stair down").

- `Tileset` is a tileset identifier specified previously in `TILESET` command. Two predefined values are `map` for the main graphics font and `text` for the text font.

- `NewTile` is a new tile number

- `NewFg` is a new foreground colour, `1-16`

- `NewBg` is a new background colour, `1-16`

**Note:** Any of `NewTile`, `NewFg` and `NewBg` parameters may be empty to use existing values without changes, but at least one of them must be present. Trailing colons may be omitted for empty parameters. 

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

## Tables ##
Please note that values from these lists are **case-sensitive** when used in the `OVERRIDE` command.

### Item Types ###
This list comes from `df/item_type.h` include file in DFHack source code.

> AMMO, AMULET, ANIMALTRAP, ANVIL, ARMOR, ARMORSTAND, BACKPACK, BALLISTAARROWHEAD, BALLISTAPARTS, BAR, BARREL, BED, BIN, BLOCKS, BOOK BOULDER, BOX, BRACELET, BUCKET, CABINET, CAGE, CATAPULTPARTS, CHAIN, CHAIR, CHEESE, CLOTH, COFFIN, COIN, CORPSE, CORPSEPIECE, CROWN, CRUTCH, DOOR, DRINK, EARRING, EGG, FIGURINE, FISH, FISH_RAW, FLASK, FLOODGATE, FOOD, GEM, GLOB, GLOVES, GOBLET, GRATE, HATCH_COVER, HELM, INSTRUMENT, LIQUID_MISC, MEAT, MILLSTONE, ORTHOPEDIC_CAST, PANTS, PET, PIPE_SECTION, PLANT, PLANT_GROWTH, POWDER_MISC, QUERN, QUIVER, REMAINS, RING, ROCK, ROUGH, SCEPTER, SEEDS, SHIELD, SHOES, SIEGEAMMO, SKIN_TANNED, SLAB, SMALLGEM, SPLINT, STATUE, TABLE, THREAD, TOOL, TOTEM, TOY, TRACTION_BENCH, TRAPCOMP, TRAPPARTS, VERMIN, WEAPON, WEAPONRACK, WINDOW, WOOD

### Building Types
This list comes from `df/building_type.h` include file in DFHack source code.

> AnimalTrap, ArcheryTarget, Armorstand, AxleHorizontal, AxleVertical, BarsFloor, BarsVertical, Bed, Box, Bridge, Cabinet, Cage, Chain, Chair, Civzone, Coffin, Construction, Door, FarmPlot, Floodgate, Furnace, GearAssembly, GrateFloor, GrateWall, Hatch, Hive, Nest, NestBox, RoadDirt, RoadPaved, Rollers ScrewPump, Shop, SiegeEngine, Slab, Statue, Stockpile, Support, Table, TractionBench, TradeDepot, Trap, Wagon, WaterWheel, Weapon, Weaponrack, Well, Windmill, WindowGem, WindowGlass, Workshop

### Tile Types ###
This list comes from `df/tiletype.h` include file in DFHack source code.

> Ashes1, Ashes2, Ashes3, 

> BrookE, BrookN, BrookNE, BrookNW, BrookS, BrookSE, BrookSW, BrookTop, BrookW, 

> BurningTreeBranches, BurningTreeCapFloor, BurningTreeCapRamp, BurningTreeCapWall, BurningTreeTrunk, BurningTreeTwigs, 

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

> TreeBranches, TreeBranchesSmooth, TreeBranchEW, TreeBranchNE, TreeBranchNEW, TreeBranchNS, TreeBranchNSE, TreeBranchNSEW, TreeBranchNSW, TreeBranchNW, TreeBranchSE, TreeBranchSEW, TreeBranchSW, TreeCapFloor1, TreeCapFloor2, TreeCapFloor3, TreeCapFloor4, TreeCapPillar, TreeCapRamp, TreeCapWallE, TreeCapWallN, TreeCapWallNE, TreeCapWallNW, TreeCapWallS, TreeCapWallSE, TreeCapWallSW, TreeCapWallW, TreeDeadBranches, TreeDeadBranchesSmooth, TreeDeadBranchEW, TreeDeadBranchNE, TreeDeadBranchNEW, TreeDeadBranchNS, TreeDeadBranchNSE, TreeDeadBranchNSEW, TreeDeadBranchNSW, TreeDeadBranchNW, TreeDeadBranchSE, TreeDeadBranchSEW, TreeDeadBranchSW, TreeDeadCapFloor1, TreeDeadCapFloor2, TreeDeadCapFloor3, TreeDeadCapFloor4, TreeDeadCapPillar, TreeDeadCapRamp, TreeDeadCapWallE, TreeDeadCapWallN, TreeDeadCapWallNE, TreeDeadCapWallNW, TreeDeadCapWallS, TreeDeadCapWallSE, TreeDeadCapWallSW, TreeDeadCapWallW, TreeDeadRoots, TreeDeadRootSloping, TreeDeadTrunkBranchE, TreeDeadTrunkBranchN, TreeDeadTrunkBranchS, TreeDeadTrunkBranchW, TreeDeadTrunkE, TreeDeadTrunkEW, TreeDeadTrunkInterior, TreeDeadTrunkN, TreeDeadTrunkNE, TreeDeadTrunkNEW, TreeDeadTrunkNS, TreeDeadTrunkNSE, TreeDeadTrunkNSEW, TreeDeadTrunkNSW, TreeDeadTrunkNW, TreeDeadTrunkPillar, TreeDeadTrunkS, TreeDeadTrunkSE, TreeDeadTrunkSEW, TreeDeadTrunkSloping, TreeDeadTrunkSW, TreeDeadTrunkW, TreeDeadTwigs, TreeRoots, TreeRootSloping, TreeTrunkBranchE, TreeTrunkBranchN, TreeTrunkBranchS, TreeTrunkBranchW, TreeTrunkE, TreeTrunkEW, TreeTrunkInterior, TreeTrunkN, TreeTrunkNE, TreeTrunkNEW, TreeTrunkNS, TreeTrunkNSE, TreeTrunkNSEW, TreeTrunkNSW, TreeTrunkNW, TreeTrunkPillar, TreeTrunkS, TreeTrunkSE, TreeTrunkSEW, TreeTrunkSloping, TreeTrunkSW, TreeTrunkW, TreeTwigs, 

> UnderworldGateStairD, UnderworldGateStairU, UnderworldGateStairUD, 

> Void, Waterfall