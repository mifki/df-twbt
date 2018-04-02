// p_display - Disable original renderer::display
// Original code will check screentexpos et al. for changes but we don't want that
// because map is not rendered this way now. But we can't completely disable graphics
// because it's used on status screen to show professions at least.
// To find this address, look for a function with two SDL_GetTicks calls inside,
// there will be two calls with the same argument right before an increment between
// SDL_SemWait and SDL_SemPost near the end - they are renderer->display() and renderer->render().
// (The called function must have conditional `dec` at the end.)

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

then go to 0xc5c470, there will be a block like (call of a function with large number of arguments)
(on Windows it's jmp instead of call)

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
    mov rax, qword [rsp+0x28]
    mov byte [ds:rax], 0x0
    ret      

On OS X with
    mov byte [r8], 0
    ret
*/

#define MAX_PATCH_LEN 32

struct patchdef {
    intptr_t addr;
    int len;
    bool hasdata;
    unsigned char data[MAX_PATCH_LEN];
};

static void apply_patch(MemoryPatcher *mp, patchdef &p)
{
    static unsigned char nops[32];
    if (!nops[0])
        memset(nops, 0x90, sizeof(nops));

    intptr_t addr = p.addr;
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
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x00b73020

        #define A_RENDER_MAP      0x009bcd50
        #define A_RENDER_UPDOWN   0x007ff150

        static patchdef p_display = { 0x00655d21, 5 };

        static patchdef p_dwarfmode_render = { 0x0062365f, 6 };

        static patchdef p_advmode_render[] = {
            { 0x005a2de5, 2+5+5 }, { 0x005a2e30, 1+5+5 }, { 0x005a2e7f, 1+5+5 }, { 0x005a2ed4, 1+5+5 }, { 0x005a3369, 1+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00c9b0f0, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x00f82e90

        #define A_RENDER_MAP      0x009ae9f0
        #define A_RENDER_UPDOWN   0x007696f0

        static patchdef p_display = { 0x00f17ec1, 5 };

        static patchdef p_dwarfmode_render = { 0x003ea43a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x003a8340, 5+3+5 }, { 0x003a899d, 5+3+5 }, { 0x003a8dc9, 5+3+5 }, { 0x003a8d66, 5+3+5 }, { 0x003a8e4a, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00c729c0, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else
        #define A_RENDER_MAP      0x08a01360
        #define A_RENDER_UPDOWN   0x087c5780

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x0836aedf, 5 };

        static patchdef p_advmode_render[] = {
            { 0x08327651, 5+7+5 }, { 0x08327c5c, 5+7+5 }, { 0x08328091, 5+7+5 }, { 0x0832759d, 5+7+5 }
        };

        static patchdef p_render_lower_levels = {
            0x08cf0d20, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #endif

#elif defined(DF_04014)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x00b94670

        #define A_RENDER_MAP      0x009dd9f0
        #define A_RENDER_UPDOWN   0x00815760

        static patchdef p_display = { 0x00669be1, 5 };

        static patchdef p_dwarfmode_render = { 0x0063742f, 6 };

        static patchdef p_advmode_render[] = {
            { 0x005b5265, 2+5+5 }, { 0x005b52b0, 2+5+5 }, { 0x005b5301, 2+5+5 }, { 0x005b5358, 2+5+5 }, { 0x005b5805, 2+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00cb7a50, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x00fc1ea0

        #define A_RENDER_MAP      0x009f1c60
        #define A_RENDER_UPDOWN   0x00796b10

        static patchdef p_display = { 0x00f56b01, 5 };

        static patchdef p_dwarfmode_render = { 0x004130ea, 5 };

        static patchdef p_advmode_render[] = {
            { 0x003d0c70, 5+3+5 }, { 0x003d12cd, 5+3+5 }, { 0x003d1696, 5+3+5 }, { 0x003d16f9, 5+3+5 }, { 0x003d177a, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00cad6b0, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else
        #define A_RENDER_MAP      0x08a3f2d0
        #define A_RENDER_UPDOWN   0x087f1440

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x08391e3f, 5 };

        static patchdef p_advmode_render[] = {
            { 0x0834e05d, 5+7+5 }, { 0x0834e111, 5+7+5 }, { 0x0834e71c, 5+7+5 }, { 0x0834eb51, 5+7+5 }
        };

        static patchdef p_render_lower_levels = {
            0x08d246b0, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #endif

#elif defined(DF_04015)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x00b960e0

        #define A_RENDER_MAP      0x009dedf0
        #define A_RENDER_UPDOWN   0x00815e50

        static patchdef p_display = { 0x00669621, 5 };

        static patchdef p_dwarfmode_render = { 0x00636dbf, 6 };

        static patchdef p_advmode_render[] = {
            { 0x005b4dc5, 2+5+5 }, { 0x005b4e10, 2+5+5 }, { 0x005b4e61, 2+5+5 }, { 0x005b4eb8, 2+5+5 }, { 0x005b5365, 2+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00cba3a0, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x00fc5be0

        #define A_RENDER_MAP      0x009f35c0
        #define A_RENDER_UPDOWN   0x00797510

        static patchdef p_display = { 0x00f5a841, 5 };

        static patchdef p_dwarfmode_render = { 0x00412c2a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x003d0690, 5+3+5 }, { 0x003d0ced, 5+3+5 }, { 0x003d1119, 5+3+5 }, { 0x003d10b6, 5+3+5 }, { 0x003d119a, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00cb11e0, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else
        #define A_RENDER_MAP      0x08a42250
        #define A_RENDER_UPDOWN   0x087f2e30

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x08391f0f, 5 };

        static patchdef p_advmode_render[] = {
            { 0x0834e191, 5+7+5 }, { 0x0834e79c, 5+7+5 }, { 0x0834ebd1, 5+7+5 }, { 0x0834e0dd, 5+7+5 }
        };

        static patchdef p_render_lower_levels = {
            0x08d29380, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #endif

#elif defined(DF_04016)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x00b98910

        #define A_RENDER_MAP      0x009e1430
        #define A_RENDER_UPDOWN   0x00818280

        static patchdef p_display = { 0x0066b941, 5 };

        static patchdef p_dwarfmode_render = { 0x0063915f, 6 };

        static patchdef p_advmode_render[] = {
            { 0x005b6f65, 2+5+5 }, { 0x005b6fb0, 2+5+5 }, { 0x005b7001, 2+5+5 }, { 0x005b7058, 2+5+5 }, { 0x005b7505, 2+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00cbcb20, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x00fca9e0

        #define A_RENDER_MAP      0x009f7ef0
        #define A_RENDER_UPDOWN   0x0079bbe0

        static patchdef p_display = { 0x00f5f641, 5 };

        static patchdef p_dwarfmode_render = { 0x0041730a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x003d4d70, 5+3+5 }, { 0x003d53cd, 5+3+5 }, { 0x003d57f9, 5+3+5 }, { 0x003d5796, 5+3+5 }, { 0x003d587a, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00cb5d50, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else
        #error Wait for 0.40.17 on Linux please

    #endif

#elif defined(DF_04019)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x00ba53f0

        #define A_RENDER_MAP      0x009ed960
        #define A_RENDER_UPDOWN   0x00820ad0

        static patchdef p_display = { 0x006732d1, 5 };

        static patchdef p_dwarfmode_render = { 0x0064013f, 6 };

        static patchdef p_advmode_render[] = {
            { 0x005b8f35, 2+5+5 }, { 0x005b8f80, 2+5+5 }, { 0x005b8fd1, 2+5+5 }, { 0x005b9028, 2+5+5 }, { 0x005b94d5, 2+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00ccb5d0, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x00f8f8b0

        #define A_RENDER_MAP      0x009d2a10
        #define A_RENDER_UPDOWN   0x00778850

        static patchdef p_display = { 0x00f24a21, 5 };

        static patchdef p_dwarfmode_render = { 0x004102ea, 5 };

        static patchdef p_advmode_render[] = {
            { 0x003c8860, 5+3+5 }, { 0x003c8ebd, 5+3+5 }, { 0x003c92e9, 5+3+5 }, { 0x003c9286, 5+3+5 }, { 0x003c936a, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00c82e20, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else
        #define A_RENDER_MAP      0x08a30380
        #define A_RENDER_UPDOWN   0x087e3af0

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x083944ef, 5 };

        static patchdef p_advmode_render[] = {
            { 0x0834b0f1, 5+7+5 }, { 0x0834b6fc, 5+7+5 }, { 0x0834bb2a, 5+7+5 }, { 0x0834b03d, 5+7+5 }
        };

        static patchdef p_render_lower_levels = {
            0x08d0ee00, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #endif

#elif defined(DF_04023)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x00bb39b0

        #define A_RENDER_MAP      0x009fa970
        #define A_RENDER_UPDOWN   0x00820f90

        static patchdef p_display = { 0x00675171, 5 };

        static patchdef p_dwarfmode_render = { 0x006401ff, 6 };

        static patchdef p_advmode_render[] = {
            { 0x005b8d25, 2+5+5 }, { 0x005b8d70, 2+5+5 }, { 0x005b8dc1, 2+5+5 }, { 0x005b8e18, 2+5+5 }, { 0x005b92c5, 2+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00cd7190, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x00f9f7f0

        #define A_RENDER_MAP      0x009e58e0
        #define A_RENDER_UPDOWN   0x007778d0

        static patchdef p_display = { 0x00f34661, 5 };

        static patchdef p_dwarfmode_render = { 0x0040ff4a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x003c82c0, 5+3+5 }, { 0x003c891d, 5+3+5 }, { 0x003c8ce6, 5+3+5 }, { 0x003c8d49, 5+3+5 }, { 0x003c8dca, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00c933f0, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else
        #define A_RENDER_MAP      0x08a423b0
        #define A_RENDER_UPDOWN   0x087e1500

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x083947ff, 5 };

        static patchdef p_advmode_render[] = {
            { 0x0834b2dd, 5+7+5 }, { 0x0834b391, 5+7+5 }, { 0x0834b99c, 5+7+5 }, { 0x0834bdca, 5+7+5 }
        };

        static patchdef p_render_lower_levels = {
            0x08d1e310, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #endif

#elif defined(DF_04024)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x00bb4190

        #define A_RENDER_MAP      0x009fad60
        #define A_RENDER_UPDOWN   0x008215c0

        static patchdef p_display = { 0x00675571, 5 };

        static patchdef p_dwarfmode_render = { 0x0064039f, 6 };

        static patchdef p_advmode_render[] = {
            { 0x005b9005, 2+5+5 }, { 0x005b9050, 2+5+5 }, { 0x005b90a1, 2+5+5 }, { 0x005b90f8, 2+5+5 }, { 0x005b95a5, 2+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00cd72f0, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x00fa1500

        #define A_RENDER_MAP      0x009e6170
        #define A_RENDER_UPDOWN   0x007780a0

        static patchdef p_display = { 0x00f35db1, 5 };

        static patchdef p_dwarfmode_render = { 0x0041016a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x003c8550, 5+3+5 }, { 0x003c8bad, 5+3+5 }, { 0x003c8f76, 5+3+5 }, { 0x003c8fd9, 5+3+5 }, { 0x003c905a, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00c94100, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else
        #define A_RENDER_MAP      0x08a43270
        #define A_RENDER_UPDOWN   0x087e1d30

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x08394a0f, 5 };

        static patchdef p_advmode_render[] = {
            { 0x0834b4dd, 5+7+5 }, { 0x0834b591, 5+7+5 }, { 0x0834bb9c, 5+7+5 }, { 0x0834bfca, 5+7+5 }
        };

        static patchdef p_render_lower_levels = {
            0x08d1f8f0, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #endif

#elif defined(DF_04203) // Thanks to TheBloke
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x00cbaec0

        #define A_RENDER_MAP      0x00ae3d20
        #define A_RENDER_UPDOWN   0x008c25a0

        static patchdef p_display = { 0x006dd861, 5 };

        static patchdef p_dwarfmode_render = { 0x006a47ce, 6 };

        static patchdef p_advmode_render[] = {
            { 0x00607635, 2+5+5 }, { 0x00607680, 2+5+5 }, { 0x006076d1, 2+5+5 }, { 0x00607728, 2+5+5 }, { 0x00607b9b, 1+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00ddc220, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x01195030

        #define A_RENDER_MAP      0x00b32d30
        #define A_RENDER_UPDOWN   0x00891eb0

        static patchdef p_display = { 0x01128401, 5 };

        static patchdef p_dwarfmode_render = { 0x004bba3a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x0046e650, 5+3+5 }, { 0x0046eccd, 5+3+5 }, { 0x0046f096, 5+3+5 }, { 0x0046f0f9, 5+3+5 }, { 0x0046f17a, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00df5a10, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else 
        #define A_RENDER_MAP      0x08b909b0
        #define A_RENDER_UPDOWN   0x088f80c0

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x008404392, 5 };

        static patchdef p_advmode_render[] = {
            { 0x083ac44d, 5+7+5 }, { 0x083ac501, 5+7+5 }, { 0x083acb1c, 5+7+5 }, { 0x083acf4a, 5+7+5 }
        };

        static patchdef p_render_lower_levels = {
            0x08e85ce0, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #endif

#elif defined(DF_04204)
    #ifdef WIN32 // Thanks to figment
        #define A_LOAD_MULTI_PDIM (0x00CBD850)

        #define A_RENDER_MAP      (0x00AE5D10)
        #define A_RENDER_UPDOWN   (0x008C3DE0)

        static patchdef p_display = { 0x006DD8F1, 5 };

        static patchdef p_dwarfmode_render = { 0x006A48FE, 6 };

        static patchdef p_advmode_render[] = {
            { 0x006072B5, 2+5+5 }, { 0x00607300, 2+5+5 }, { 0x00607351, 2+5+5 }, { 0x006073A8, 2+5+5 }, { 0x0060781B, 1+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00DDEF70, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x011998c0

        #define A_RENDER_MAP      0x00b36130
        #define A_RENDER_UPDOWN   0x00894690

        static patchdef p_display = { 0x0112cc61, 5 };

        static patchdef p_dwarfmode_render = { 0x004bd0da, 5 };

        static patchdef p_advmode_render[] = {
            { 0x0046fcf0, 5+3+5 }, { 0x0047036d, 5+3+5 }, { 0x00470736, 5+3+5 }, { 0x00470799, 5+3+5 }, { 0x0047081a, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00dfa060, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else // Thanks to topis
        #define A_RENDER_MAP      0x08B93710
        #define A_RENDER_UPDOWN   0x088FA210

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x008404622, 5 };

        static patchdef p_advmode_render[] = {
            { 0x083AC6DD, 5+7+5 }, { 0x083AC791, 5+7+5 }, { 0x083ACDAC, 5+7+5 }, { 0x083AD1DA, 5+7+5 }
        };

        static patchdef p_render_lower_levels = {
            0x08E89BC0, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #endif

#elif defined(DF_04205)
    #ifdef WIN32 // thanks to figment
        #define A_LOAD_MULTI_PDIM (0x00CC2580)
        #define A_RENDER_MAP      (0x00AEABF0)
        #define A_RENDER_UPDOWN   (0x008C8180)

        static patchdef p_display = { 0x006E1C41, 5 };

        static patchdef p_dwarfmode_render = { 0x006A890E, 6 };

        static patchdef p_advmode_render[] = {
            { 0x0060B365, 2+5+5 }, { 0x0060B3B0, 2+5+5 }, { 0x0060B401, 2+5+5 }, { 0x0060B458, 2+5+5 }, { 0x0060B8CB, 1+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00DE4DB0, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x011a35f0

        #define A_RENDER_MAP      0x00b3e200
        #define A_RENDER_UPDOWN   0x0089c540

        static patchdef p_display = { 0x01136991, 5 };

        static patchdef p_dwarfmode_render = { 0x004c475a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x00476d40, 5+3+5 }, { 0x004773bd, 5+3+5 }, { 0x00477786, 5+3+5 }, { 0x004777e9, 5+3+5 }, { 0x0047786a, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00e02830, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else // thanks to topis & indivisible
        #define A_RENDER_MAP    0x08b9b720

        #define A_RENDER_UPDOWN 0x08901f00
        #define NO_DISPLAY_PATCH
        
        static patchdef p_dwarfmode_render = { 0x0840b502, 5 };
        
        static patchdef p_advmode_render[] = {
            { 0x083b306d, 5+7+5 }, { 0x083b3121, 5+7+5 }, { 0x083b373c, 5+7+5 }, { 0x083b3b6a, 5+7+5 }
        };
        
        static patchdef p_render_lower_levels = {
            0x08e92a20, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };
    #endif

#elif defined(DF_04206)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM (0x00CD83A0)

        #define A_RENDER_MAP      (0x00AFFC20)
        #define A_RENDER_UPDOWN   (0x008DB2A0)

        static patchdef p_display = { 0x006E7351, 5 };

        static patchdef p_dwarfmode_render = { 0x006AAA5E, 6 };

        static patchdef p_advmode_render[] = {
            { 0x0060D9B5, 2+5+5 }, { 0x0060DA00, 2+5+5 }, { 0x0060DA51, 2+5+5 }, { 0x0060DAA8, 2+5+5 }, { 0x0060DF1B, 1+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00DF9F60, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x011C8F40

        #define A_RENDER_MAP      0x00B63CB0
        #define A_RENDER_UPDOWN   0x008BDC2D

        static patchdef p_display = { 0x0115BF01, 5 };

        static patchdef p_dwarfmode_render = { 0x004C9E3A, 5 };

        static patchdef p_advmode_render[] = {
            { 0x0047D170, 5+3+5 }, { 0x0047D7ED, 5+3+5 }, { 0x0047DBB6, 5+3+5 }, { 0x0047DC19, 5+3+5 }, { 0x0047DC9A, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00E27EB0, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else
        #define A_RENDER_MAP 0x08bbefb0
        #define A_RENDER_UPDOWN 0x08922960
        #define NO_DISPLAY_PATCH
        static patchdef p_dwarfmode_render = { 0x0840feb2, 5 };
        static patchdef p_advmode_render[] = {
            { 0x083b839d, 5+7+5 }, { 0x083b8451, 5+7+5 }, { 0x083b8a6c, 5+7+5 }, { 0x083b8e9a, 5+7+5 }
        };
        static patchdef p_render_lower_levels = {
            0x08eb6860, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };
    #endif

#elif defined(DF_04303)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x00cfa020
        #define A_RENDER_MAP      0x00b1f6c0
        #define A_RENDER_UPDOWN   0x008f8a70

        static patchdef p_display = { 0x006f7351, 5 };

        static patchdef p_dwarfmode_render = { 0x006ba48e, 6 };

        static patchdef p_advmode_render[] = {
            { 0x00611395, 2+5+5 }, { 0x006113e0, 2+5+5 }, { 0x00611431, 2+5+5 }, { 0x00611488, 2+5+5 }, { 0x006118fd, 1+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00e1cea0, 15, true, { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x0120cbb0

        #define A_RENDER_MAP      0x00b9ea70
        #define A_RENDER_UPDOWN   0x008f3180

        static patchdef p_display = { 0x0119e101, 5 };

        static patchdef p_dwarfmode_render = { 0x004e442a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x004832b2, 5+3+5 }, { 0x0048396d, 5+3+5 }, { 0x00483d48, 5+3+5 }, { 0x00483dab, 5+3+5 }, { 0x00483e2c, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00e65740, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };

    #else
        #define A_RENDER_MAP 0x08bf9b00
        #define A_RENDER_UPDOWN 0x08955b40

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x08429f12, 5 };

        static patchdef p_advmode_render[] = {
            { 0x083c167d, 5+7+5 }, { 0x083c1731, 5+7+5 }, { 0x083c1d5c, 5+7+5 }, { 0x083c218a, 5+7+5 }
        };

        static patchdef p_render_lower_levels = {
            0x08ef43c0, 13, true, { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 }
        };
    #endif

#elif defined(DF_04305)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x140a36ad0

        #define A_RENDER_MAP      0x1408170b0
        #define A_RENDER_UPDOWN   0x140593f30

        static patchdef p_display = { 0x14035c4cb, 5 };

        static patchdef p_dwarfmode_render = { 0x14031266a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x0140265a6b, 5+5 }, { 0x140265abc, 5+5 }, { 0x140265b06, 5+5 }, { 0x140265ff4, 5+5 },
        };

        static patchdef p_render_lower_levels = {
            0x140b7f500, 9, true, { 0x48, 0x8B, 0x44, 0x24, 0x28,  0xC6, 0x00, 0x00,  0xC3 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x10106a9b0

        #define A_RENDER_MAP      0x100a7eb10
        #define A_RENDER_UPDOWN   0x100815cd0

        static patchdef p_display = { 0x1010003cb, 5 };

        static patchdef p_dwarfmode_render = { 0x100457447, 5 };

        static patchdef p_advmode_render[] = {
            { 0x100404a31, 5+3+5 }, { 0x100404aca, 5+7+5 }, { 0x10040501a, 5+3+5 }, { 0x10040532b, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x100d075c0, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xC3 }
        };

    #else
        #define A_RENDER_MAP 0x00d537c0
        #define A_RENDER_UPDOWN 0x00b345f0

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x00720ae5, 5 };

        static patchdef p_advmode_render[] = {
            { 0x006aa202, 5+5+5 }, { 0x006aa802, 5+5+5 }, { 0x006aa841, 5+5+5 }, { 0x006aa89d, 5+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x00fc8fa0, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xC3 }
        };
    #endif


#elif defined(DF_04402)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x140ab8b20

        #define A_RENDER_MAP      0x14088a3e0
        #define A_RENDER_UPDOWN   0x1405dfa20

        static patchdef p_display = { 0x14038a7cb, 5 };

        static patchdef p_dwarfmode_render = { 0x14034020a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x14028c1db, 5+5 }, { 0x14028c22c, 5+5 }, { 0x14028c276, 5+5 }, { 0x14028c743, 5+5 },
        };

        static patchdef p_render_lower_levels = {
            0x140c04fc0, 9, true, { 0x48, 0x8B, 0x44, 0x24, 0x28,  0xC6, 0x00, 0x00,  0xC3 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x1011474f0

        #define A_RENDER_MAP      0x100ad96d0
        #define A_RENDER_UPDOWN   0x1008633e0

        static patchdef p_display = { 0x1010dbdbb, 5 };

        static patchdef p_dwarfmode_render = { 0x10046dee7, 5 };

        static patchdef p_advmode_render[] = {
            { 0x100416651, 5+3+5 }, { 0x1004166ea, 5+7+5 }, { 0x100416c4a, 5+3+5 }, { 0x100416f5b, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x100d73840, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xC3 }
        };
    
    #else
        #define A_RENDER_MAP 0xde8750
        #define A_RENDER_UPDOWN 0xbbca30

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x714185, 5 };

        static patchdef p_advmode_render[] = {
            { 0x69667a, 5+5+5 }, { 0x696c92, 5+5+5 }, { 0x696cd1, 5+5+5 }, { 0x696d2d, 5+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x1070150, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xC3 }
        };
    #endif

#elif defined(DF_04403)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x140ab9a40

        #define A_RENDER_MAP      0x14088b250
        #define A_RENDER_UPDOWN   0x1405e0400

        static patchdef p_display = { 0x14038b12b, 5 };

        static patchdef p_dwarfmode_render = { 0x140340b3a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x14028cafb, 5+5 }, { 0x14028cb4c, 5+5 }, { 0x14028cb96, 5+5 }, { 0x14028d063, 5+5 },
        };

        static patchdef p_render_lower_levels = {
            0x140c05f20, 9, true, { 0x48, 0x8B, 0x44, 0x24, 0x28,  0xC6, 0x00, 0x00,  0xC3 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x101149460

        #define A_RENDER_MAP      0x100adb140
        #define A_RENDER_UPDOWN   0x100864a20

        static patchdef p_display = { 0x1010ddcfb, 5 };

        static patchdef p_dwarfmode_render = { 0x10046e657, 5 };

        static patchdef p_advmode_render[] = {
            { 0x100416d91, 5+3+5 }, { 0x100416e2a, 5+7+5 }, { 0x10041738a, 5+3+5 }, { 0x10041769b, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x100d75590, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xC3 }
        };
    
    #else
        #define A_RENDER_MAP 0xde9f40
        #define A_RENDER_UPDOWN 0xbbd540

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x7132f5, 5 };

        static patchdef p_advmode_render[] = {
            { 0x692c4a, 5+5+5 }, { 0x693262, 5+5+5 }, { 0x6932a1, 5+5+5 }, { 0x6932fd, 5+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x1071ba0, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xC3 }
        };

    #endif

#elif defined(DF_04404)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x140aba570

        #define A_RENDER_MAP      0x14088bd30
        #define A_RENDER_UPDOWN   0x1405e0de0

        static patchdef p_display = { 0x14038bb1b, 5 };

        static patchdef p_dwarfmode_render = { 0x1403414ea, 5 };

        static patchdef p_advmode_render[] = {
            { 0x14028d49b, 5+5 }, { 0x14028d4ec, 5+5 }, { 0x14028d536, 5+5 }, { 0x14028da03, 5+5 },
        };

        static patchdef p_render_lower_levels = {
            0x140c06ac0, 9, true, { 0x48, 0x8B, 0x44, 0x24, 0x28,  0xC6, 0x00, 0x00,  0xC3 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x10114a230

        #define A_RENDER_MAP      0x100adbd90 //"Here we have"
        #define A_RENDER_UPDOWN   0x1008657c0

        static patchdef p_display = { 0x1010deacb, 5 };

        static patchdef p_dwarfmode_render = { 0x10046e867, 5 }; // "Following"

        static patchdef p_advmode_render[] = {
            { 0x100416fa1, 5+3+5 }, { 0x10041703a, 5+7+5 }, { 0x10041759a, 5+3+5 }, { 0x1004178ab, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x100d75c60, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xC3 }
        };
    
    #else
        #define A_RENDER_MAP      0xdeb560
        #define A_RENDER_UPDOWN   0xbbe9e0

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x713c55, 5 };

        static patchdef p_advmode_render[] = {
            { 0x69350a, 5+5+5 }, { 0x693b22, 5+5+5 }, { 0x693b61, 5+5+5 }, { 0x693bbd, 5+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x10795e0, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xC3 }
        };

    #endif

#elif defined(DF_04405)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x140aba650

        #define A_RENDER_MAP      0x14088be10
        #define A_RENDER_UPDOWN   0x1405e0ec0

        static patchdef p_display = { 0x14038bbfb, 5 };

        static patchdef p_dwarfmode_render = { 0x1403415ca, 5 };

        static patchdef p_advmode_render[] = {
            { 0x14028d57b, 5+5 }, { 0x14028d5cc, 5+5 }, { 0x14028d616, 5+5 }, { 0x14028dae3, 5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x140c06ba0, 9, true, { 0x48, 0x8b, 0x44, 0x24, 0x28, 0xc6, 0x00, 0x00, 0xc3 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x10114a230

        #define A_RENDER_MAP      0x100adbd90
        #define A_RENDER_UPDOWN   0x1008657c0

        static patchdef p_display = { 0x1010deacb, 5 };

        static patchdef p_dwarfmode_render = { 0x10046e867, 5 };

        static patchdef p_advmode_render[] = {
            { 0x100416fa1, 5+3+5 }, { 0x10041703a, 5+7+5 }, { 0x10041759a, 5+3+5 }, { 0x1004178ab, 5+3+5 }
        };

        static patchdef p_render_lower_levels = {
            0x100d75c60, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xc3 }
        };

    #else
        #define A_RENDER_MAP      0xdeb760
        #define A_RENDER_UPDOWN   0xbbebe0

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x713e55, 5 };

        static patchdef p_advmode_render[] = {
            { 0x69376a, 5+5+5 }, { 0x693d82, 5+5+5 }, { 0x693dc1, 5+5+5 }, { 0x693e1d, 5+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x10797e0, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xc3 }
        };

    #endif

#elif defined(DF_04407)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x140ad0290

        #define A_RENDER_MAP      0x14089fc80
        #define A_RENDER_UPDOWN   0x1405ef390

        static patchdef p_dwarfmode_render = { 0x14034b05a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x14029755b, 10 }, { 0x1402975ac, 10 }, { 0x1402975f6, 10 }, { 0x140297ac3, 10 },
        };

        static patchdef p_display = { 0x14039528b, 5 };

        static patchdef p_render_lower_levels = {
            0x140c1a900, 9, true, { 0x48, 0x8b, 0x44, 0x24, 0x28, 0xc6, 0x00, 0x00, 0xc3 }
        };

    #elif defined(__APPLE__)
        #define A_LOAD_MULTI_PDIM 0x101170120

        #define A_RENDER_MAP      0x100b027f0
        #define A_RENDER_UPDOWN   0x10087f270

        static patchdef p_dwarfmode_render = { 0x10048250a, 5 };

        static patchdef p_advmode_render[] = {
            { 0x10042a9c1, 13 }, { 0x10042aa5a, 17 }, { 0x10042afba, 13 }, { 0x10042b2cb, 13 },
        };

        static patchdef p_display = { 0x10110419b, 5 };

        static patchdef p_render_lower_levels = {
            0x100d9f490, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xc3 }
        };
    
    #else
        #define A_RENDER_MAP      0xdfad70
        #define A_RENDER_UPDOWN   0xbcbcc0

        #define NO_DISPLAY_PATCH

        static patchdef p_dwarfmode_render = { 0x7108b5, 5 };

        static patchdef p_advmode_render[] = {
            { 0x6dfd5a, 5+5+5 }, { 0x6e0372, 5+5+5 }, { 0x6e03b1, 5+5+5 }, { 0x6e040d, 5+5+5 }
        };

        static patchdef p_render_lower_levels = {
            0x107d3f0, 5, true, { 0x41, 0xc6, 0x00, 0x00, 0xc3 }
        };

    #endif        

#elif defined(DF_04409)
    #ifdef WIN32
        #define A_LOAD_MULTI_PDIM 0x140ad0a90	// Done

        #define A_RENDER_MAP      0x1408a03f0	// Done
        #define A_RENDER_UPDOWN   0x1405efb00	// Done

        static patchdef p_dwarfmode_render = { 0x14034b5ca, 5 };	// Done

        static patchdef p_advmode_render[] = {	// Done
            { 0x140297aab, 10 },         
            { 0x140297afc, 10 },
            { 0x140297b46, 10 },           
            { 0x140298013, 10 },
        };

        static patchdef p_display = { 0x1403957FB, 5 };  // Done

        static patchdef p_render_lower_levels = {
            0x140c1b1d0, 9, true, { 0x48, 0x8b, 0x44, 0x24, 0x28, 0xc6, 0x00, 0x00, 0xc3 } // Done
        };
    #elif defined(__APPLE__)
     	#error Mac OSX is not yet supported for DF 44.09    
    #else
        #error Linux is not yet supported for DF 44.09
    #endif 
#else

    #error Unsupported DF version
#endif
