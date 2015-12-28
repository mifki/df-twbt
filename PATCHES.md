I use [Hopper Disassembler](http://www.hopperapp.com), it's available for OS X and Linux. You can use trial version that's is limited to 30 minutes which is enough.

You can use Hopper to find the patches for all versions of DF. So you can run Hopper on OSX or Linux and use it to disassemble all of OSX, Windows and Linux versions.

Note: usually addresses don't change much, and usually they increase. So if some address you found is completely different from the address in previous version, better check again if it's correct.

The first time you do the patching on a new version it's recommended to first follow the instructions for a previous version that is already completed. That means you already have the found addresses which you can use to verify that you are understanding and following the instructions correctly. You can then refer back to the known-good version when searching in the new version, which can be very helpful when the new version has changed enough that it doesn't quite match the patterns described below.

1. To find the address for `A_LOAD_MULTI_PDIM`:

	**On Windows** find a function that references string "`Tileset not found`".
	**On OS X** find a function that calls `IMG_Load` in its very beginning, and next `SDL_SetAlpha`, `SDL_CreateRGBSurface`, and `SDL_SetAlpha` again.
	**On Linux** not required.

2. Go to the address of vtable for `viewscreen_dwarfmodest` (from symbols.xml), you need *third* DWORD at that address, it's the address of viewscreen's `render()` method. Go to that function.

 Windows: Be aware that if you're using Hopper, it may appear that you have jumped into the middle of a function, not the end, because you see only a bunch of `dd` instructions. This is because Hopper cannot decode the executable properly at this point. You are in fact at the start of a function. It doesn't matter - just follow the instructions below, relative to the point indicated by the third DWORD. The disassembler IDA should render this part of the code correctly.

	Look for the *first* call instruction from that point (Windows, Linux) or *second* call instruction (OS X), go to that function, rename it to `dwarfmode_render_main` for convenience.

	There will be jump over one or two instructions close to the beginning (easily visible in UI). Shortly after that there will be a call of a function with the following:

  * Windows: no arguments
  * OS X: two arguments, zero and an address
  * Linux: two arguments, a register and an address

  On Windows, the zero-argument call, which follows the pattern explained below, will be found right after a call to a method with several arguments (two as of 40.24, three as of 42.03).

	**On Windows**
  
    Pattern is `add, xor, push, call` or `add, push, call`.
    
    Address of the push instruction is `p_dwarfmode_render`.

	**On OS X** it's

	    mov [esp+4], 0
	    mov [esp], eax
	    call ...

	    Address of the call instruction is `p_dwarfmode_render`.

	**On Linux** it's 
	
	    xor edi, edi
      call UNIMPORTANT_ADDRESS
      mov dword  .., <register>      # argument
      mov dword .., <address>        # argument
	    call <address of render_map>  <- p_dwarfmode_render

	    Address of the call instruction is `p_dwarfmode_render`.
	
	Rename the called function to `render_map`, its address is `A_RENDER_MAP`.

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

On Windows, after jumping to "THE_SAME_ADDR" you will likely see the call to `p_render_lower_levels` occurring after four `mov <register>, dword..` and several `push` instructions. The instruction immediately before the call will likely be `mov ecx, ebx`. Note that "THE_SAME_ADDR" may not jump to the start of a function on Windows, it might jump to a later point in the same function in which you found `0x30000000`.

7. The last one is `p_display`, which is only needed on Windows and OSX.

It can be tricky. First, find references to `SDL_GetTicks`.

	**On Windows** look for a function that does `mov edi, [SDL_GetTicks]` in its very beginning.
	
	Somewhere closer to the end of that function there's a pattern like:

      add esp, 0x4
      ...
      add esp, 0x4
      ...
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
	
	`p_display` is not required `on Linux`. This may cause problems though, so better would be to build a special version of `libgraphics.so` with a call to `renderer->display()` removed.
