**This instructions apply to 64-bit versions of Dwarf Fortress.**

I use [Hopper Disassembler](http://www.hopperapp.com), it's available for OS X and Linux and can open executables for all platforms. You can use trial version that's is limited to 30 minutes which is enough for our needs.

Note: usually addresses don't change much, and usually they increase. So if some address you found is completely different from the address in previous version, it's likely incorrect.

1. **Windows & macOS only** Find a function that references string "Tileset not found". The address of this function is `A_LOAD_MULTI_PDIM`.

2. Find a function that references string "Here we have". Rename it to `render_map`. Its address is `A_RENDER_MAP`.

3. Find a function that references string "Following". Rename it to `dwarfmode_render_main`. 

4. Find a call of `render_map` from `dwarfmode_render_main`. The address of the call instruction is `p_dwarfmode_render`.

5. Open a list of references to `render_map`. There will be a function referencing it four times. The addresses of the four call instructions are `p_advmode_render`.

5+. At each of the four addresses there are either `call, call` (Windows) or `call, mov, call` / `call, lea, call` (Linux and macOS) instructions. Make sure that the total length of these instructions matches values specified for each address in `p_advmode_render`.

6. Go to any of the four call instructions from the last step. The address of a function called right after `render_map` is `A_RENDER_UPDOWN` (use function address, not a call instruction address). 

7. Look for `0x30000000` in disassembly in the second half of the code, closer to the end.

    You need to find the following code:
    
        compare with 0x7
        jump ADDR
        compare with 0x2
        jump ANOTHER_ADDR
        compare with 0x30000000
        jump THE_SAME_ADDR
    
    Go to the address after the comparison with `0x30000000`. Look for the first `call` instruction after that point, address of the called function is `p_render_lower_levels`. **On Windows** it may be a `jmp` instruction instead.
    
7. **Windows & macOS only** Find references to `SDL_GetTicks`, look for a function that calls `SDL_GetTicks` in its very beginning and then again after some time. Go to its end, and look for the following code:
    
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
    
    Address of that call instruction above is `p_display`.
    
    `p_display` is not required on Linux. This may cause problems though, so better would be to build a special version of `libgraphics.so` with a call to `renderer->display()` removed.
