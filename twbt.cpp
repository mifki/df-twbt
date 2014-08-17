//
//  twbt.cpp
//  twbt
//
//  Created by Vitaly Pronkin on 14/05/14.
//  Copyright (c) 2014 mifki. All rights reserved.
//

#include <sys/stat.h>
#include <stdint.h>
#include <math.h>
#include <iostream>
#include <map>
#include <vector>

#if defined(WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
  
    #define GLEW_STATIC
    #include "glew/glew.h"
    #include "glew/wglew.h"

    float roundf(float x)
    {
       return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f);
    }

#elif defined(__APPLE__)
    #include <OpenGL/gl.h>

#else
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "VTableInterpose.h"
#include "MemAccess.h"
#include "VersionInfo.h"
#include "TileTypes.h"
#include "modules/Maps.h"
#include "modules/World.h"
#include "modules/MapCache.h"
#include "modules/Gui.h"
#include "modules/Screen.h"
#include "modules/Buildings.h"
#include "df/construction.h"
#include "df/block_square_event_frozen_liquidst.h"
#include "df/tiletype.h"
#include "df/graphic.h"
#include "df/enabler.h"
#include "df/d_init.h"
#include "df/renderer.h"
#include "df/interfacest.h"
#include "df/world_raws.h"
#include "df/building.h"
#include "df/building_workshopst.h"
#include "df/building_def_workshopst.h"
#include "df/building_type.h"
#include "df/buildings_other_id.h"
#include "df/item.h"
#include "df/item_type.h"
#include "df/items_other_id.h"
#include "df/unit.h"
#include "df/viewscreen_dwarfmodest.h"
#include "df/viewscreen_setupadventurest.h"
#include "df/viewscreen_dungeonmodest.h"
#include "df/viewscreen_choose_start_sitest.h"
#include "df/viewscreen_new_regionst.h"
#include "df/viewscreen_layer_export_play_mapst.h"
#include "df/viewscreen_layer_world_gen_paramst.h"
#include "df/viewscreen_overallstatusst.h"
#include "df/viewscreen_petst.h"
#include "df/viewscreen_movieplayerst.h"
#include "df/viewscreen_layer_militaryst.h"
#include "df/viewscreen_tradegoodsst.h"
#include "df/ui_sidebar_mode.h"
#include "df/ui_advmode.h"

#include "renderer_twbt.h"

using df::global::world;
using std::string;
using std::vector;
using df::global::enabler;
using df::global::gps;
using df::global::ui;
using df::global::init;
using df::global::d_init;
using df::global::gview;

struct texture_fullid {
    unsigned int texpos;
    float r, g, b;
    float br, bg, bb;
};

struct gl_texpos {
    GLfloat left, right, top, bottom;
};

#ifdef WIN32
    // On Windows there's no convert_magenta parameter. Arguments are pushed onto stack,
    // except for tex_pos and filename, which go into ecx and edx. Simulating this with __fastcall.
    typedef void (__fastcall *LOAD_MULTI_PDIM)(long *tex_pos, const string &filename, void *tex, long dimx, long dimy, long *disp_x, long *disp_y);
#else
    typedef void (*LOAD_MULTI_PDIM)(void *tex, const string &filename, long *tex_pos, long dimx, long dimy, bool convert_magenta, long *disp_x, long *disp_y);
#endif

LOAD_MULTI_PDIM load_multi_pdim;

static void load_tileset(const string &filename, long *tex_pos, long dimx, long dimy, long *disp_x, long *disp_y)
{
#ifdef WIN32
    load_multi_pdim(tex_pos, filename, &enabler->textures, dimx, dimy, disp_x, disp_y);
#else
    load_multi_pdim(&enabler->textures, filename, tex_pos, dimx, dimy, true, disp_x, disp_y);
#endif
}


struct tileset {
    long small_texpos[16*16], large_texpos[16*16];
};
static vector< struct tileset > tilesets;

struct override {
    char kind;
    int id, type, subtype;
    long small_texpos, large_texpos;
    std::string subtypename;
};
static vector< struct override > *overrides[256];

long *text_texpos, *map_texpos;

long cursor_small_texpos, cursor_large_texpos;

static bool enabled;
static bool has_textfont, has_overrides;
static color_ostream *out2;
static df::item_flags bad_item_flags;

static int maxlevels = 0;
static bool multi_rendered;
static float fogdensity = 0.15f;
static float fogcolor[4] = { 0.1f, 0.1f, 0.3f, 1 };
static float shadowcolor[4] = { 0, 0, 0, 0.4f };

static int small_map_dispx, small_map_dispy;
static int large_map_dispx, large_map_dispy;

static int tdimx, tdimy;
static int gwindow_x, gwindow_y, gwindow_z;

static float addcolors[][3] = { {1,0,0} };

static int8_t *depth;
static GLfloat *shadowtex;
static GLfloat *shadowvert;
static GLfloat *fogcoord;
static long shadow_texpos[8];
static bool shadowsloaded;
static int gmenu_w;
static uint8_t skytile;
static uint8_t chasmtile;

// Buffers for map rendering
static uint8_t *_gscreen[2];
static int32_t *_gscreentexpos[2];
static int8_t *_gscreentexpos_addcolor[2];
static uint8_t *_gscreentexpos_grayscale[2];
static uint8_t *_gscreentexpos_cf[2];
static uint8_t *_gscreentexpos_cbr[2];

// Current buffers
static uint8_t *gscreen;
static int32_t *gscreentexpos;
static int8_t *gscreentexpos_addcolor;
static uint8_t *gscreentexpos_grayscale, *gscreentexpos_cf, *gscreentexpos_cbr;

// Previous buffers to determine changed tiles
static uint8_t *gscreen_old;
static int32_t *gscreentexpos_old;
static int8_t *gscreentexpos_addcolor_old;
static uint8_t *gscreentexpos_grayscale_old, *gscreentexpos_cf_old, *gscreentexpos_cbr_old;

// Buffers for rendering lower levels before merging    
static uint8_t *mscreen;
static int32_t *mscreentexpos;
static int8_t *mscreentexpos_addcolor;
static uint8_t *mscreentexpos_grayscale;
static uint8_t *mscreentexpos_cf;
static uint8_t *mscreentexpos_cbr;

#include "tileupdate_text.hpp"
#include "tileupdate_map.hpp"

// Disables standard rendering of lower levels
/* To find this address, find a block like (on OS X)

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
*/
static void patch_rendering(bool enable_lower_levels)
{
    static bool ready = false;
    static unsigned char orig[15];

#if defined(DF_03411)
    #ifdef WIN32
        void *addr = (void*)(0x00b56370 + Core::getInstance().vinfo->getRebaseDelta());

        // mov eax, dword [ss:esp+0x0c]
        // mov byte [ds:eax], 0x00
        // retn 0x1c    
        unsigned char patch[] = { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 };

    #elif defined(__APPLE__)
        void *addr = (void*)0x00af6c30;

        // mov eax, dword [ss:esp+0x14]
        // mov byte [ds:eax], 0x00
        // ret
        unsigned char patch[] = { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 };
        //unsigned char patch[] = { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc7,0x00,0x20,0x00,0x00,0x00, 0xC3 };

    #else
        #define NO_RENDERING_PATCH
    #endif

#elif defined(DF_04008)
    #ifdef WIN32
        void *addr = (void*)(0x00c91b00 + Core::getInstance().vinfo->getRebaseDelta());

        // mov eax, dword [ss:esp+0x0c]
        // mov byte [ds:eax], 0x00
        // retn 0x1c    
        unsigned char patch[] = { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 };

    #elif defined(__APPLE__)
        void *addr = (void*)0x00c61700;

        // mov eax, dword [ss:esp+0x14]
        // mov byte [ds:eax], 0x00
        // ret
        unsigned char patch[] = { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 };

    #else
        #define NO_RENDERING_PATCH
    #endif        
#endif

#ifndef NO_RENDERING_PATCH
    if (!ready)
    {
        (new MemoryPatcher(Core::getInstance().p))->verifyAccess((void*)addr, sizeof(patch), true);
        memcpy(orig, (void*)addr, sizeof(patch));
        ready = true;
    }

    if (enable_lower_levels)
        memcpy((void*)addr, orig, sizeof(patch));
    else
        memcpy((void*)addr, patch, sizeof(patch));
#endif
}

static void replace_renderer()
{
    if (enabled)
        return;

    MemoryPatcher p(Core::getInstance().p);

#if defined(DF_04008)
    //XXX: This is a crazy work-around for vtable address for df::renderer not being available yet
    //in dfhack for 0.40.xx, which prevents its subclasses form being instantiated. We're overwriting
    //original vtable anyway, so any value will go.
    unsigned char zz[] = { 0xff, 0xff, 0xff, 0xff };
#ifdef WIN32
    p.write((char*)&df::renderer::_identity + 72, zz, 4);
#else
    p.write((char*)&df::renderer::_identity + 64, zz, 4);
#endif
#endif

    renderer_opengl *oldr = (renderer_opengl*)enabler->renderer;
    renderer_cool *newr = new renderer_cool;

    void **vtable_old = ((void ***)oldr)[0];
    void **vtable_new = ((void ***)newr)[0];

    /*for (int i = 0; i < 20; i++)
        *out2 << "$ " << i << " " << vtable_old[i] << std::endl;
    for (int i = 0; i < 24; i++)
        *out2 << "$ " << i << " " << vtable_new[i] << std::endl;*/

#define DEFIDX(n) int IDX_##n = vmethod_pointer_to_idx(&renderer_cool::n);

    DEFIDX(draw)
    DEFIDX(update_tile)
    DEFIDX(get_mouse_coords)
    DEFIDX(update_tile_old)
    DEFIDX(reshape_gl)
    DEFIDX(reshape_gl_old)
    DEFIDX(_last_vmethod)

    void *get_mouse_coords_new = vtable_new[IDX_get_mouse_coords];
    void *draw_new             = vtable_new[IDX_draw];
    void *reshape_gl_new       = vtable_new[IDX_reshape_gl];
    void *update_tile_new      = vtable_new[IDX_update_tile];    

    p.verifyAccess(vtable_new, sizeof(void*)*IDX__last_vmethod, true);
    memcpy(vtable_new, vtable_old, sizeof(void*)*IDX__last_vmethod);

    vtable_new[IDX_draw] = draw_new;

    vtable_new[IDX_update_tile] = update_tile_new;
    vtable_new[IDX_update_tile_old] = vtable_old[IDX_update_tile];

    vtable_new[IDX_reshape_gl] = reshape_gl_new;
    vtable_new[IDX_reshape_gl_old] = vtable_old[IDX_reshape_gl];

    vtable_new[IDX_get_mouse_coords] = get_mouse_coords_new;
    
    memcpy(&newr->screen, &oldr->screen, (char*)&newr->dummy-(char*)&newr->screen);

    newr->reshape_graphics();

    enabler->renderer = (df::renderer*)newr;

    unsigned char nop6[] = { 0x90,0x90,0x90,0x90,0x90,0x90 };

#if defined(DF_03411)
    #ifdef WIN32
        // On Windows original map rendering function must be called at least once to initialize something

        // Disable original renderer::display
        // See below how to find this address
        p.write((void*)(0x005be941 + Core::getInstance().vinfo->getRebaseDelta()), nop6, 5);

    #elif defined(__APPLE__)

        // Disable original map rendering
        p.write((void*)0x002e0e0a, nop6, 5);

        // Disable original renderer::display
        // Original code will check screentexpos et al. for changes but we don't want that
        // because map is not rendered this way now. But we can't completely disable graphics
        // because it's used on status screen to show professions at least.
        // To find this address, look for a function with two SDL_GetTicks calls inside,
        // there will be two calls with the same argument right before an increment between 
        // SDL_SemWait and SDL_SemPost near the end - they are renderer->display() and renderer->render(). 
        p.write((void*)0x00c92fe1, nop6, 5);

        // Adv. mode
        // Second patched call is rendering of up/down map views

        // Main rendering mode
        p.write((void*)0x002cbbb0, nop6, 5);
        p.write((void*)(0x002cbbb0+5+3), nop6, 5);

        // Another rendering, after a movement
        p.write((void*)0x002cc225, nop6, 5);
        p.write((void*)(0x002cc225+5+3), nop6, 5);

        // When an alert is shown
        p.write((void*)0x002cc288, nop6, 5);
        p.write((void*)(0x002cc288+5+3), nop6, 5);

        // Hero died
        p.write((void*)0x002cbf8d, nop6, 5);
        p.write((void*)(0x002cbf8d+5+3), nop6, 5);

    #else
        #error Linux not supported yet
    #endif

#elif defined(DF_04008)
    #ifdef WIN32
        // On Windows original map rendering function must be called at least once to initialize something

        // Disable original renderer::display
        // See below how to find this address
        p.write((void*)(0x00653721 + Core::getInstance().vinfo->getRebaseDelta()), nop6, 5);

    #elif defined(__APPLE__)

        // Disable original map rendering
        p.write((void*)0x003e76aa, nop6, 5);

        // Disable original renderer::display
        // Original code will check screentexpos et al. for changes but we don't want that
        // because map is not rendered this way now. But we can't completely disable graphics
        // because it's used on status screen to show professions at least.
        // To find this address, look for a function with two SDL_GetTicks calls inside,
        // there will be two calls with the same argument right before an increment between 
        // SDL_SemWait and SDL_SemPost near the end - they are renderer->display() and renderer->render(). 
        p.write((void*)0x00f03511, nop6, 5);

        // Adv. mode
        // Second patched call is rendering of up/down map views

        // Main rendering mode
        p.write((void*)0x003a5710, nop6, 5);
        p.write((void*)(0x003a5710+5+3), nop6, 5);

        // Another rendering, after a movement
        p.write((void*)0x003a6136, nop6, 5);
        p.write((void*)(0x003a6136+5+3), nop6, 5);

        // When an alert is shown
        p.write((void*)0x003a6199, nop6, 5);
        p.write((void*)(0x003a6199+5+3), nop6, 5);

        // Hero died
        p.write((void*)0x003a5d6d, nop6, 5);
        p.write((void*)(0x003a5d6d+5+3), nop6, 5);

    #else
        #error Linux not supported yet
    #endif        
#endif

    enabled = true;   
}

static void restore_renderer()
{
    /*if (!enabled)
        return;

    enabled = false;

    gps->force_full_display_count = true;*/
}

#include "renderer.hpp"
#include "dwarfmode.hpp"
#include "dungeonmode.hpp"
#include "tradefix.hpp"
#include "config.hpp"
#include "commands.hpp"
#include "plugin.hpp"