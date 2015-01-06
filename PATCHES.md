I use [Hopper Disassembler](http://www.hopperapp.com), it's available for OS X and Linux. You can use trial version that's is limited to 30 minutes which is enough.

Note: usually addresses don't change much, and usually they increase. So if some address you found is completely different from the address in previous version, better check again if it's correct.

1. To find the address for `A_LOAD_MULTI_PDIM`:

	**On Windows** find a function that references string "`Tileset not found`".
	**On OS X** find a function that calls `IMG_Load` in its very beginning, and next `SDL_SetAlpha`, `SDL_CreateRGBSurface`, and `SDL_SetAlpha` again.
	**On Linux** not required.

2. Go to the address of vtable for `viewscreen_dwarfmodest` (from symbols.xml), you need *third* DWORD at that address, it's the address of viewscreen's `render()` method. Go to that function.

	Look for the *first* call instruction (Windows, Linux) or *second* call instruction (OS X), go to that function, rename it to `dwarfmode_render_main` for convenience.

	There will be jump over one or two instructions close to the beginning (easily visible in UI). Shortly after that there will be a call of a function with zero (Windows) or zero and an address (OS X, Linux) as its arguments. 

	**On Windows** it's `xor, push, call`. Address of the push instruction is `p_dwarfmode_render`.

	**On OS X** it's

	    mov [esp+4], 0
	    mov [esp], eax
	    call ...

	**On Linux** it's 
	
	    xor edi, edi
	    ...
	    mov [esp+4], edi
	    mov [esp], SOME_ADDRESS
	    call ...

	Address of the call instruction is `p_dwarfmode_render`.
	
	Rename the called function to `render_map`, it's address is `A_RENDER_MAP`.

3. The same way go to the `render()` method of `viewscreen_dungeonmodest`, rename it to `advmode_render`.

4. Find calls of `render_map()` from `advmode_render()`, there should be five (four on Linux). Go to any of them. Rename the very next called function to `render_updown`, its address is `A_RENDER_UPDOWN`.

5. Visit all calls of `render_map()` and `render_updown()` in `advmode_render()`.

	**On Windows** each if them is `push, call, call`. Addresses of the push instructions are `p_advmode_render`. **Important!** push instruction may use variable number of bytes, adjust patch length accordingly.
	
	**On OS X** and **Linux** each of them is `mov, call, mov, call`. Addresses of the first call instruction are `p_advmode_render`.

6. Look for `0x30000000` in disassembly, close to the end of the code. 

	You're looking for the pattern
	
	    compare with 0x7
	    jump ADDR
	    compare with 0x2
	    jump ANOTHER_ADDR
	    compare with 0x30000000
	    jump THE_SAME_ADDR
	
	Go to the address after comparison with `0x30000000`. There will be a call of a function with many arguments (i.e. you're looking for the first call instruction), address of that function is `p_render_lower_levels`.

7. The last one is tricky. First, find references to `SDL_GetTicks`.

	**On Windows** look for a function that does `mov edi, [SDL_GetTicks]` in its very beginning.
	
	Somewhere closer to the end of that function there's a pattern like
	
	    call SOME_ADDRESS
	    ...
	    call ANOTHER_ADDRESS	<---- You need this instruction
	    ...
	    call eax
	    ...
	    call ebp
	    ...
	    inc ...
	    ...
	    call ebx
	
	**On OS X** look for a function that calls `SDL_GetTicks` in its very beginning and then again after some time. Go to its end, and look for the pattern
	
	    call SDL_SemPost
	    ...
	    call SDL_SemPost
	    ...
	    call SOME_ADDRESS
	    ...
	    call ANOTHER_ADDRESS	<---- You need this instruction
	    ...
	    call [edx+...]
	    ...
	    call SDL_SemWait
	    ...
	    add ..., 0x1
	    ...
	    call SDL_SemPost
	
	Address of that call instruction above is `p_display`. To check, go to that function, and one of the last instructions should be `dec`.
	
	This one is not required `on Linux`. This may cause problems though, so better would be to build a special version of `libgraphics.so` with a call to `renderer->display()` removed.
