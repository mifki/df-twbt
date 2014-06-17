## Text Will Be Text

I wrote this small plugin for myself because I was tired seeing coffins instead of zeroes and all that stuff. It allows to specify separate fonts for map tiles and for text. GRAPHICS_FONT/GRAPHICS_FULLFONT will continue to be used for map area(s), and FONT/FULLFONT will be used for everything else. Requires OpenGL PRINT_MODE (STANDARD, VBO and so on). Adventurer mode and possibly some other screens need some more work. Non-fullscreen dfhack overlays turn entire screen to ASCII, sorry. Just press eg. ESC twice to restore.

Also it allows to override tile numbers for buildings and items, see overrides.txt for details.

?Also

Also it divides trade screen evenly on wide resolutions to see full item names.

- Vitaly Pronkin aka mifki <pronvit@me.com>

## Version History

**3.00**

* ?colors
* ?hauling
* ?macro
* ?designations
* ?cursor tile
* fix for movie player screens
* making trade screen divide evenly on wide resolutions (OS X only)
* crash on plugin unload fixed

**2.01**

* binaries for dfhack-r4

**2.00**

* overrides

**1.21**

* show dwarves on the overall status (z) screen in graphics

**1.20**

* fixes for various screens and modes
* including slightly modified font from Shizzle tileset in case you don't * have nice ASCII font handy

**1.00**

* initial release

