**This instructions apply to 64-bit versions of Dwarf Fortress.**

I use [Hopper Disassembler](http://www.hopperapp.com), it's available for OS X and Linux and can open executables for all platforms. You can use trial version that's is limited to 30 minutes which is enough for our needs.

Note: usually addresses don't change much, and usually they increase. So if some address you found is completely different from the address in previous version, it's likely incorrect.

1. Find the address for `A_LOAD_MULTI_PDIM`:

    **Windows & macOS:** find a function that references string "Tileset not found" and use its address.
    **Linux:** not required.

2. Go to the address of vtable for `viewscreen_dwarfmodest` (from symbols.xml), you need the *third* QWORD at that address, it's the address of viewscreen's `render()` method. Go to that function.

    Look for the first `call` instruction, go to that function, rename it to `dwarfmode_render_main` for convenience.

    **Windows:** There will be the following code in the beginning (these should be the first `call` instructions):

        lea   rcx, ...
        call  SOME_ADDRESS
        xor   edx, edx
        call  ...    <---- You need this instruction 

    **macOS:** There will be jump over one or two instructions close to the beginning (easily visible in UI), and then the following code:

        mov   esi, 0x1
        call  SOME_ADDRESS
        lea   ...
        xor   esi, esi
        call  ...    <---- You need this instruction

    **Linux:** Look for a call to `drawborder()` in the beginning, there will be the following code:
    
        call  drawborder
        xor   esi, esi
        mov   ...
        call  ...    <---- You need this instruction
    
    **All platforms:** Address of the call instruction is `p_dwarfmode_render`. Rename the called function to `render_map`, its address is `A_RENDER_MAP`.

3. The same way go to the `render()` method of `viewscreen_dungeonmodest`, rename it to `advmode_render`.

4. Find calls of `render_map()` from `advmode_render()`, there should be four of them. Go to any of them. Rename the very next called function to `render_updown`, its address is `A_RENDER_UPDOWN`.

5. Visit all calls of `render_map()` and `render_updown()` in `advmode_render()`.

    **Windows:** each if them is `xor, call, call`. Addresses of the first `call` instructions are `p_advmode_render`.
    
    **macOS & Linux:** each of them is `mov, call, mov, call` or `mov, call, lea, call`. Addresses of the first call instruction are `p_advmode_render`. Adjust the patch length depending on whether `mov` or `lea` instruction is used.

6. Look for `0x30000000` in disassembly in the second half of the code, closes to the end.

    You're looking for the pattern
    
        compare with 0x7
        jump ADDR
        compare with 0x2
        jump ANOTHER_ADDR
        compare with 0x30000000
        jump THE_SAME_ADDR
    
    Go to the address after the comparison with `0x30000000`. Look for the first `call` instruction after that point, address of the called function is `p_render_lower_levels`. **On Windows** it may be a `jmp` instruction instead.
    
7. The last one is `p_display`, which is only needed on Windows and OS X.

    First, find references to `SDL_GetTicks`. Look for a function that calls `SDL_GetTicks` in its very beginning and then again after some time. Go to its end, and look for the following code:
    
        call  SDL_SemPost
        ...
        call  SDL_SemPost
        ...
        call  SOME_ADDRESS
        ...
        call  ANOTHER_ADDRESS    <---- You need this instruction
        ...
        call  [rax+...]
        ...
        call  SDL_SemWait
        ...
        add   ..., 0x1 or inc ...
        ...
        call  SDL_SemPost
    
    Address of that call instruction above is `p_display`. To check, go to the called function, and one of the last instructions should be `dec`.
    
    `p_display` is not required `on Linux`. This may cause problems though, so better would be to build a special version of `libgraphics.so` with a call to `renderer->display()` removed.
