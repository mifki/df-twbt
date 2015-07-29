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
#include <algorithm>

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
    #include <dlfcn.h>
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
#include "df/descriptor_color.h"
#include "df/building.h"
#include "df/building_workshopst.h"
#include "df/building_def_workshopst.h"
#include "df/building_type.h"
#include "df/buildings_other_id.h"
#include "df/machine.h"
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
#include "df/building_drawbuffer.h"

#include "renderer_twbt.h"

extern "C" {
    DFSDL_Surface *IMG_Load(const char *file);
    void SDL_FreeSurface(void*);
}

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

struct tileset {
    long small_texpos[16*16], large_texpos[16*16];
};
static vector< struct tileset > tilesets;

struct override {
    int type, subtype;
    long small_texpos, large_texpos;
    char bg, fg;
    std::string subtypename;
};

struct tile_overrides {
    vector< struct override > tiletype_overrides;
};

static struct tile_overrides *overrides[256];

long *text_texpos, *map_texpos;

long cursor_small_texpos, cursor_large_texpos;

static bool enabled;
static bool has_textfont, has_overrides;
static color_ostream *out2;
static df::item_flags bad_item_flags;

static int maxlevels = 0;
static bool multi_rendered;
static float fogdensity = 0.15f;
static float fogstart = 0;
static float fogstep = 1;
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

static long gscreen_over[256*256];

#include "patches.hpp"

#ifdef WIN32
    // On Windows there's no convert_magenta parameter. Arguments are pushed onto stack,
    // except for tex_pos and filename, which go into ecx and edx. Simulating this with __fastcall.
    typedef void (__fastcall *LOAD_MULTI_PDIM)(long *tex_pos, const string &filename, void *tex, long dimx, long dimy, long *disp_x, long *disp_y);

    // On Windows there's no parameter pointing to the map_renderer structure
    typedef void (_stdcall *RENDER_MAP)(int);
    typedef void (_stdcall *RENDER_UPDOWN)();

#else
    typedef void (*LOAD_MULTI_PDIM)(void *tex, const string &filename, long *tex_pos, long dimx, long dimy, bool convert_magenta, long *disp_x, long *disp_y);

    typedef void (*RENDER_MAP)(void*, int);
    typedef void (*RENDER_UPDOWN)(void*);    
#endif

LOAD_MULTI_PDIM _load_multi_pdim;
RENDER_MAP _render_map;
RENDER_UPDOWN _render_updown;

#ifdef WIN32
    #define render_map() _render_map(0)
    #define render_updown() _render_updown()
    #define load_tileset(filename,tex_pos,dimx,dimy,disp_x,disp_y) _load_multi_pdim(tex_pos, filename, &enabler->textures, dimx, dimy, disp_x, disp_y)
#else
    #define render_map() _render_map(df::global::map_renderer, 0)
    #define render_updown() _render_updown(df::global::map_renderer)
    #define load_tileset(filename,tex_pos,dimx,dimy,disp_x,disp_y) _load_multi_pdim(&enabler->textures, filename, tex_pos, dimx, dimy, true, disp_x, disp_y)
#endif

static void patch_rendering(bool enable_lower_levels)
{
#ifndef NO_RENDERING_PATCH
    static bool ready = false;
    static unsigned char orig[MAX_PATCH_LEN];

    long addr = p_render_lower_levels.addr;
    #ifdef WIN32
        addr += Core::getInstance().vinfo->getRebaseDelta();
    #endif

    if (!ready)
    {
        (new MemoryPatcher(Core::getInstance().p))->makeWritable((void*)addr, sizeof(p_render_lower_levels.len));
        memcpy(orig, (void*)addr, p_render_lower_levels.len);
        ready = true;
    }

    if (enable_lower_levels)
        memcpy((void*)addr, orig, p_render_lower_levels.len);
    else
        apply_patch(NULL, p_render_lower_levels);
#endif
}

static void replace_renderer()
{
    if (enabled)
        return;

    MemoryPatcher p(Core::getInstance().p);

    //XXX: This is a crazy work-around for vtable address for df::renderer not being available yet
    //in dfhack for 0.40.xx, which prevents its subclasses form being instantiated. We're overwriting
    //original vtable anyway, so any value will go.
    unsigned char zz[] = { 0xff, 0xff, 0xff, 0xff };
#ifdef WIN32
    p.write((char*)&df::renderer::_identity + 72, zz, 4);
#else
    p.write((char*)&df::renderer::_identity + 64, zz, 4);
#endif

    renderer_opengl *oldr = (renderer_opengl*)enabler->renderer;
    renderer_cool *newr = new renderer_cool;

    void **vtable_old = ((void ***)oldr)[0];
    void ** volatile vtable_new = ((void ***)newr)[0];

#undef DEFIDX
#define DEFIDX(n) int IDX_##n = vmethod_pointer_to_idx(&renderer_cool::n);

    DEFIDX(draw)
    DEFIDX(update_tile)
    DEFIDX(get_mouse_coords)
    DEFIDX(get_mouse_coords_old)
    DEFIDX(update_tile_old)
    DEFIDX(reshape_gl)
    DEFIDX(reshape_gl_old)
    DEFIDX(zoom)
    DEFIDX(zoom_old)
    DEFIDX(_last_vmethod)

    void *get_mouse_coords_new = vtable_new[IDX_get_mouse_coords];
    void *draw_new             = vtable_new[IDX_draw];
    void *reshape_gl_new       = vtable_new[IDX_reshape_gl];
    void *update_tile_new      = vtable_new[IDX_update_tile];    
    void *zoom_new             = vtable_new[IDX_zoom];

    p.verifyAccess(vtable_new, sizeof(void*)*IDX__last_vmethod, true);
    memcpy(vtable_new, vtable_old, sizeof(void*)*IDX__last_vmethod);

    vtable_new[IDX_draw] = draw_new;

    vtable_new[IDX_update_tile] = update_tile_new;
    vtable_new[IDX_update_tile_old] = vtable_old[IDX_update_tile];

    vtable_new[IDX_reshape_gl] = reshape_gl_new;
    vtable_new[IDX_reshape_gl_old] = vtable_old[IDX_reshape_gl];

    vtable_new[IDX_get_mouse_coords] = get_mouse_coords_new;
    vtable_new[IDX_get_mouse_coords_old] = vtable_old[IDX_get_mouse_coords];

    vtable_new[IDX_zoom] = zoom_new;
    vtable_new[IDX_zoom_old] = vtable_old[IDX_zoom];
    
    memcpy(&newr->screen, &oldr->screen, (char*)&newr->dummy-(char*)&newr->screen);

    newr->reshape_graphics();

    enabler->renderer = (df::renderer*)newr;

    // Disable original renderer::display
    #ifndef NO_DISPLAY_PATCH
        apply_patch(&p, p_display);    
    #endif

    // On Windows original map rendering function must be called at least once to initialize something (?)
    #ifndef WIN32
        // Disable dwarfmode map rendering
        apply_patch(&p, p_dwarfmode_render);

        // Disable advmode map rendering
        for (int j = 0; j < sizeof(p_advmode_render)/sizeof(patchdef); j++)
            apply_patch(&p, p_advmode_render[j]);
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

static bool advmode_needs_map(int m)
{
    //XXX: these are mostly numerical becuase enumeration seems to be incorrect in dfhack for 0.40.xx

    return (m == df::ui_advmode_menu::Default ||
            m == df::ui_advmode_menu::Look ||
            m == df::ui_advmode_menu::ThrowAim ||
            m == df::ui_advmode_menu::ConversationAddress ||
            m == 14 ||
            m == df::ui_advmode_menu::Fire ||
            m == df::ui_advmode_menu::Jump ||   // (j)
            m == df::ui_advmode_menu::Unk30 ||
            m == 21 ||                          // (S)
            m == 3  || m == 4 ||                // (k)
            m == 20 ||                          // (m)
            m == 16 ||                          // (g)
            m == 37 || m == 38 ||               // (A)
            m == 19 ||                          // companions
            m == 29 ||                          // sleep/wait
            m == 23 ||                          // move carefully/climbing
            m == 33 ||                          // falling/grab screen (?)
            m == 43 ||                          // dodge direction choice
            m == 39 ||                          // target for striking choice
            m == 18                             // combat preferences
            );
}

#include "tileupdate_text.hpp"
#include "tileupdate_map.hpp"
#include "renderer.hpp"
#include "dwarfmode.hpp"
#include "dungeonmode.hpp"
#include "legacy/renderer_legacy.hpp"
#include "legacy/twbt_legacy.hpp"
#include "config.hpp"
#include "commands.hpp"
#include "macros.hpp"
//#include "items.hpp"
#include "buildings.hpp"
#include "plugin.hpp"