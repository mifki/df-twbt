## Text Will Be Text ##

This is a plugin for [Dwarf Fortress](http://bay12games.com) / [DFHack](http://github.com/dfhack/dfhack) that improves various aspects of rendering.

I wrote this small plugin because I was tired seeing coffins instead of zeroes and all that stuff. It allows to specify separate fonts for map tiles and for text. GRAPHICS\_FONT / GRAPHICS\_FULLFONT will continue to be used for map area(s), and FONT / FULLFONT will be used for everything else. Requires OpenGL PRINT_MODE (STANDARD, VBO and so on). Adventurer mode and possibly some other screens need some more work. Non-fullscreen dfhack overlays turn entire screen to ASCII, sorry. Just press eg. ESC twice to restore.

### Overrides ###

Allows to override tile numbers for buildings and items, see overrides.txt for details.

### Multi-Level Rendering ###

Copy `shadows.png` to `data/art` folder.

### Additional Colours ###

Not yet.

### Mapshot! ###

`mapshot` command saves an image of entire map in full size to mapshot.tga in DF folder.

### Other Improvements ###

Trade screen divided equally on OS X.

### Authors and Links ###

Vitaly Pronkin aka mifki <pronvit@me.com>

[Home Page / Sources](https://github.com/mifki/df-twbt) -- [Latest Release](https://github.com/mifki/df-twbt/releases) -- [Discussion](http://www.bay12forums.com/smf/index.php?topic=138754.0) -- [Report an Issue](https://github.com/mifki/df-twbt/issues)
