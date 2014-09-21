// p_display - Disable original renderer::display
// Original code will check screentexpos et al. for changes but we don't want that
// because map is not rendered this way now. But we can't completely disable graphics
// because it's used on status screen to show professions at least.
// To find this address, look for a function with two SDL_GetTicks calls inside,
// there will be two calls with the same argument right before an increment between 
// SDL_SemWait and SDL_SemPost near the end - they are renderer->display() and renderer->render(). 

// p_render_lower_levels - Disables standard rendering of lower levels
/*
To find this address, find a block like (on OS X)

    00c5c140 F644246007                      test       byte [ss:esp+0x60], 0x7               ; XREF=0xc5b3d4
    00c5c145 0F858FF2FFFF                    jne        0xc5b3da
                                           ; Basic Block Input Regs: esp -  Killed Regs: <nothing>
    00c5c14b 83BC24F800000002                cmp        dword [ss:esp+0xf8], 0x2
    00c5c153 0F8517030000                    jne        0xc5c470
                                           ; Basic Block Input Regs: esp -  Killed Regs: <nothing>
    00c5c159 F744246000000030                test       dword [ss:esp+0x60], 0x30000000
    00c5c161 0F8509030000                    jne        0xc5c470

then go to 0xc5c470, there will be a block like (call of a function with enormous number of arguments)

    00c5c470 0FBF842484000000                movsx      eax, word [ss:esp+0x84]               ; XREF=0xc5c153, 0xc5c161
    00c5c478 8B9424DC000000                  mov        edx, dword [ss:esp+0xdc]
    00c5c47f 8B8C24D8000000                  mov        ecx, dword [ss:esp+0xd8]
    00c5c486 8BB424D4000000                  mov        esi, dword [ss:esp+0xd4]
    00c5c48d 89442420                        mov        dword [ss:esp+0x20], eax
    00c5c491 8B8424E0000000                  mov        eax, dword [ss:esp+0xe0]
    00c5c498 89542418                        mov        dword [ss:esp+0x18], edx
    00c5c49c 8B7C2458                        mov        edi, dword [ss:esp+0x58]
    00c5c4a0 8B6C2450                        mov        ebp, dword [ss:esp+0x50]
    00c5c4a4 8B9424C0000000                  mov        edx, dword [ss:esp+0xc0]
    00c5c4ab 8944241C                        mov        dword [ss:esp+0x1c], eax
    00c5c4af 8B442454                        mov        eax, dword [ss:esp+0x54]
    00c5c4b3 894C2414                        mov        dword [ss:esp+0x14], ecx
    00c5c4b7 89742410                        mov        dword [ss:esp+0x10], esi
    00c5c4bb 897C240C                        mov        dword [ss:esp+0xc], edi
    00c5c4bf 896C2408                        mov        dword [ss:esp+0x8], ebp
    00c5c4c3 89442404                        mov        dword [ss:esp+0x4], eax
    00c5c4c7 891424                          mov        dword [ss:esp], edx
    00c5c4ca E831520000                      call       0x00c61700

and 0x00c61700 will be your address

On Windows we're patching that function with
    mov eax, dword [ss:esp+0x0c]
    mov byte [ds:eax], 0x00
    retn 0x1c    

On other systems with
    mov eax, dword [ss:esp+0x14]
    mov byte [ds:eax], 0x00
    ret
*/

#define MAX_PATCH_LEN 32

struct patchdef {
    unsigned long addr;
    int len;
    bool hasdata;
    unsigned char data[MAX_PATCH_LEN];
};

static void apply_patch(MemoryPatcher *mp, patchdef &p)
{
    static unsigned char nops[32];
    if (!nops[0])
        memset(nops, 0x90, sizeof(nops));

    long addr = p.addr;
    #ifdef WIN32
        addr += Core::getInstance().vinfo->getRebaseDelta();
    #endif

    unsigned char *data = p.hasdata ? p.data : nops;

    if (mp)
        mp->write((void*)addr, data, p.len);
    else
        memcpy((void*)addr, data, p.len);
}



#if defined(DF_04012)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x00b73290
        #define A_RENDER_MAP      0x009bd480
        #define A_RENDER_UPDOWN   0x007ff980

        static patchdef p_display = { 0x00655d71, 5 };

        static patchdef p_dwarfmode_render = { 0x0062373f, 6 };
        
        static patchdef p_advmode_render[] = {
            { 0x005a2f75, 2+5+5 }, { 0x005a2fc0, 1+5+5 }, { 0x005a300f, 1+5+5 }, { 0x005a3064, 1+5+5 }, { 0x005a34f9, 1+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00c9aac0, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x00f80eb0

        #define A_RENDER_MAP      0x009ae010
        #define A_RENDER_UPDOWN   0x00768e50

        static patchdef p_display = { 0x00f15ee1, 5 };

        static patchdef p_dwarfmode_render = { 0x003e9e6a, 5 };
        
        static patchdef p_advmode_render[] = {
            { 0x003a7d70, 5+3+5 }, { 0x003a83cd, 5+3+5 }, { 0x003a87f9, 5+3+5 }, { 0x003a8796, 5+3+5 }, { 0x003a887a, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00c71700, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else
        #define A_RENDER_MAP      0x08a00f00
        #define A_RENDER_UPDOWN   0x087c54c0

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x0836ac3f, 5 };
        
        static patchdef p_advmode_render[] = {
            { 0x083273b1, 5+7+5 }, { 0x083279bc, 5+7+5 }, { 0x08327df1, 5+7+5 }, { 0x083272fd, 5+7+5 }
        };

        static patchdef p_render_lower_levels = {
            0x08cefd50, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #endif

#elif defined(DF_04013)
    #define LEGACY_MODE_ONLY
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x00b73020
        #define A_RENDER_MAP      0x009bd480
        #define A_RENDER_UPDOWN   0x007ff980

        static patchdef p_display = { 0x00655d71, 5 };

        static patchdef p_dwarfmode_render = { 0x0062373f, 6 };
        
        static patchdef p_advmode_render[] = {
            { 0x005a2f75, 2+5+5 }, { 0x005a2fc0, 1+5+5 }, { 0x005a300f, 1+5+5 }, { 0x005a3064, 1+5+5 }, { 0x005a34f9, 1+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00c9aac0, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x00f82e90

        #define A_RENDER_MAP      0x009ae010
        #define A_RENDER_UPDOWN   0x00768e50

        static patchdef p_display = { 0x00f15ee1, 5 };

        static patchdef p_dwarfmode_render = { 0x003e9e6a, 5 };
        
        static patchdef p_advmode_render[] = {
            { 0x003a7d70, 5+3+5 }, { 0x003a83cd, 5+3+5 }, { 0x003a87f9, 5+3+5 }, { 0x003a8796, 5+3+5 }, { 0x003a887a, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00c71700, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else
        #define A_RENDER_MAP      0x08a00f00
        #define A_RENDER_UPDOWN   0x087c54c0

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x0836ac3f, 5 };
        
        static patchdef p_advmode_render[] = {
            { 0x083273b1, 5+7+5 }, { 0x083279bc, 5+7+5 }, { 0x08327df1, 5+7+5 }, { 0x083272fd, 5+7+5 }
        };

        static patchdef p_render_lower_levels = {
            0x08cefd50, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #endif
#else
    #error Unsupported DF version
#endif