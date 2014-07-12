## Text Will Be Text ##

This is a plugin for [Dwarf Fortress](http://bay12games.com) / [DFHack](http://github.com/dfhack/dfhack) that improves various aspects of rendering.

Originally I wrote a small plugin because I was tired seeing coffins instead of zeroes and all that stuff. It has greately evolved since then. Requires OpenGL PRINT_MODE (STANDARD or VBO).

### Text and Map Tilesets ###

Main function is to use separate fonts (tilesets) for map tiles in Fortress Mode and for text.

In your `init.txt` set FONT and FULLFONT to the font you want to use for **text**, and GRAPHICS\_FONT and GRAPHICS\_FULLFONT to the font for **map tiles**.

### Overrides ###

Allows to override tile numbers for buildings and items, see `overrides.txt` for details.

### Multi-Level Rendering ###

Copy `shadows.png` to `data/art` folder. **Note:** multilevel rendering is disabled by default for performance reasons, you need to set number of additional levels you want to render with the `multilevel` command (see below). You can add it to your `dfhack.init` to be executed automatically.

`multilevel` command sets the number of additional levels to render. Possible parameters are `more`, `less` or number `0-15`.

`multilevel shadowcolor <r> <g> <b> <a>` command sets shadow colour. Components are in range `0-1`. Default is `0 0 0 0.4`.

`multilevel fogcolor <r> <g> <b>` command sets fog colour. Default is `0.1 0.1 0.3`.

`multilevel fogdensity <d>` command sets fog density in range `0-1`. Default is `0.15`.

### Additional Colours ###

Not yet.

### Mapshot! ###

`mapshot` command saves an image of entire map in full size to `mapshot.tga` in DF folder.

### Other Improvements ###

Trade screen divided equally on OS X.

### Authors and Links ###

Vitaly Pronkin aka mifki <pronvit@me.com>

[Home Page / Sources](https://github.com/mifki/df-twbt) -- [Latest Release](https://github.com/mifki/df-twbt/releases) -- [Discussion](http://www.bay12forums.com/smf/index.php?topic=138754.0) -- [Report an Issue](https://github.com/mifki/df-twbt/issues)
