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
#include "df/building.h"
#include "df/building_type.h"
#include "df/buildings_other_id.h"
#include "df/item.h"
#include "df/item_type.h"
#include "df/items_other_id.h"
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
#include "df/ui_sidebar_mode.h"
#include "df/viewscreen_layer_militaryst.h"
#include "df/viewscreen_tradegoodsst.h"

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
    int texpos;
    float r, g, b;
    float br, bg, bb;
};

struct gl_texpos {
    GLfloat left, right, top, bottom;
};

DFHACK_PLUGIN("twbt");


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
    bool building;
    int id, type, subtype;
    long small_texpos, large_texpos;
};
static vector< struct override > *overrides[256];

long *text_texpos, *map_texpos;

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

bool is_text_tile(int x, int y, bool &is_map)
{
    const int tile = x * gps->dimy + y;
    df::viewscreen * ws = Gui::getCurViewscreen();

    int32_t w = gps->dimx, h = gps->dimy;

    is_map = false;

#define IS_SCREEN(_sc) df::_sc::_identity.is_direct_instance(ws)     

    if (IS_SCREEN(viewscreen_dungeonmodest))
    {
        //df::viewscreen_dungeonmodest *s = strict_virtual_cast<df::viewscreen_dungeonmodest>(ws);
        //TODO:

        if (y >= h-2)
            return true;

        return false;
    }    

    if (!x || !y || x == w - 1 || y == h - 1)
       return has_textfont;

    if (IS_SCREEN(viewscreen_dwarfmodest))
    {
        uint8_t menu_width, area_map_width;
        Gui::getMenuWidth(menu_width, area_map_width);
        int32_t menu_left = w - 1, menu_right = w - 1;

        bool menuforced = (ui->main.mode != df::ui_sidebar_mode::Default || df::global::cursor->x != -30000);

        if ((menuforced || menu_width == 1) && area_map_width == 2) // Menu + area map
        {
            menu_left = w - 56;
            menu_right = w - 25;
        }
        else if (menu_width == 2 && area_map_width == 2) // Area map only
        {
            menu_left = menu_right = w - 25;
        }
        else if (menu_width == 1) // Wide menu
            menu_left = w - 56;
        else if (menuforced || (menu_width == 2 && area_map_width == 3)) // Menu only
            menu_left = w - 32; 

        if (x >= menu_left && x <= menu_right)
        {
            if (menuforced && ui->main.mode == df::ui_sidebar_mode::Burrows && ui->burrows.in_define_mode)
            {
                // Make burrow symbols use graphics font
                if ((y != 12 && y != 13 && !(x == menu_left + 2 && y == 2)) || x == menu_left || x == menu_right) 
                    return has_textfont;
            }
            else
                return has_textfont;
        }

        is_map = (x > 0 && x < menu_left);

        return false;
    }

    if (!has_textfont)
        return false;
    
    if (IS_SCREEN(viewscreen_setupadventurest))
    {
        df::viewscreen_setupadventurest *s = static_cast<df::viewscreen_setupadventurest*>(ws);
        if (s->subscreen != df::viewscreen_setupadventurest::Nemesis)
            return true;
        else if (x < 58 || x >= 78 || y == 0 || y >= 21)
            return true;

        return false;
    }
    
    if (IS_SCREEN(viewscreen_choose_start_sitest))
    {
        if (y <= 1 || y >= h - 6 || x == 0 || x >= 57)
            return true;

        return false;
    }
        
    if (IS_SCREEN(viewscreen_new_regionst))
    {
        if (y <= 1 || y >= h - 2 || x <= 37 || x == w - 1)
            return true;

        return false;
    }

    if (IS_SCREEN(viewscreen_layer_export_play_mapst))
    {
        if (x == w - 1 || x < w - 23)
            return true;

        return false;
    }

    if (IS_SCREEN(viewscreen_overallstatusst))
    {
        if ((x == 46 || x == 71) && y >= 8)
            return false;

        return true;
    }

    if (IS_SCREEN(viewscreen_movieplayerst))
    {
        df::viewscreen_movieplayerst *s = static_cast<df::viewscreen_movieplayerst*>(ws);
        return !s->is_playing;
    }    

    /*if (IS_SCREEN(viewscreen_petst))
    {
        if (x == 41 && y >= 7)
            return false;

        return true;
    }*/

    //*out2 << Core::getInstance().p->readClassName(*(void**)ws) << " " << (int)ws->breakdown_level << std::endl;

    return true;
}

static float addcolors[][3] = { {1,0,0} };

static unsigned char depth[256*256];
static GLfloat shadowtex[256*256*2*6];
static GLfloat shadowvert[256*256*2*6];
static float fogcoord[256*256*6];
static long shadow_texpos[8];
static bool shadowsloaded;
static int gmenu_w;
static uint8_t skytile;
static uint8_t chasmtile;

static unsigned char gscreen[256*256*4];
static int32_t gscreentexpos[256*256];
static int8_t gscreentexpos_addcolor[256*256];
static uint8_t gscreentexpos_grayscale[256*256];
static uint8_t gscreentexpos_cf[256*256];
static uint8_t gscreentexpos_cbr[256*256];

static unsigned char mscreen[256*256*4];
static int32_t mscreentexpos[256*256];
static int8_t mscreentexpos_addcolor[256*256];
static uint8_t mscreentexpos_grayscale[256*256];
static uint8_t mscreentexpos_cf[256*256];
static uint8_t mscreentexpos_cbr[256*256];

static void screen_to_texid_text(renderer_cool *r, int tile, struct texture_fullid &ret)
{
    const unsigned char *s = r->screen + tile*4;

    int bold = (s[3] != 0) * 8;
    int fg   = (s[1] + bold) % 16;
    int bg   = s[2] % 16;

    const long texpos = r->screentexpos[tile];

    if (!texpos)
    {
        ret.texpos = text_texpos[s[0]];

        ret.r = enabler->ccolor[fg][0];
        ret.g = enabler->ccolor[fg][1];
        ret.b = enabler->ccolor[fg][2];
        ret.br = enabler->ccolor[bg][0];
        ret.bg = enabler->ccolor[bg][1];
        ret.bb = enabler->ccolor[bg][2];

        return;
    }        

    ret.texpos = texpos;

    if (r->screentexpos_grayscale[tile])
    {
        const unsigned char cf = r->screentexpos_cf[tile];
        const unsigned char cbr = r->screentexpos_cbr[tile];

        ret.r = enabler->ccolor[cf][0];
        ret.g = enabler->ccolor[cf][1];
        ret.b = enabler->ccolor[cf][2];
        ret.br = enabler->ccolor[cbr][0];
        ret.bg = enabler->ccolor[cbr][1];
        ret.bb = enabler->ccolor[cbr][2];
    }
    else if (r->screentexpos_addcolor[tile])
    {
        ret.r = enabler->ccolor[fg][0];
        ret.g = enabler->ccolor[fg][1];
        ret.b = enabler->ccolor[fg][2];
        ret.br = enabler->ccolor[bg][0];
        ret.bg = enabler->ccolor[bg][1];
        ret.bb = enabler->ccolor[bg][2];
    }
    else
    {
        ret.r = ret.g = ret.b = 1;
        ret.br = ret.bg = ret.bb = 0;
    }
}

static void screen_to_texid_map(renderer_cool *r, int tile, struct texture_fullid &ret)
{
    const unsigned char *s = gscreen + tile*4;

    int bold = (s[3] & 0x0f) * 8;
    int fg   = (s[1] + bold) % 16;
    int bg   = s[2] % 16;

    const long texpos = gscreentexpos[tile];

    if (!texpos)
    {
        ret.texpos = map_texpos[s[0]];

        ret.r = enabler->ccolor[fg][0];
        ret.g = enabler->ccolor[fg][1];
        ret.b = enabler->ccolor[fg][2];
        ret.br = enabler->ccolor[bg][0];
        ret.bg = enabler->ccolor[bg][1];
        ret.bb = enabler->ccolor[bg][2];

        return;
    }        

    ret.texpos = texpos;

    if (gscreentexpos_grayscale[tile])
    {
        const unsigned char cf = gscreentexpos_cf[tile];
        const unsigned char cbr = gscreentexpos_cbr[tile];

        ret.r = enabler->ccolor[cf][0];
        ret.g = enabler->ccolor[cf][1];
        ret.b = enabler->ccolor[cf][2];
        ret.br = enabler->ccolor[cbr][0];
        ret.bg = enabler->ccolor[cbr][1];
        ret.bb = enabler->ccolor[cbr][2];
    }
    else if (gscreentexpos_addcolor[tile])
    {
        ret.r = enabler->ccolor[fg][0];
        ret.g = enabler->ccolor[fg][1];
        ret.b = enabler->ccolor[fg][2];
        ret.br = enabler->ccolor[bg][0];
        ret.bg = enabler->ccolor[bg][1];
        ret.bb = enabler->ccolor[bg][2];
    }
    else
    {
        ret.r = ret.g = ret.b = 1;
        ret.br = ret.bg = ret.bb = 0;
    }
}

static void write_tile_arrays_text(renderer_cool *r, int x, int y, GLfloat *fg, GLfloat *bg, GLfloat *tex)
{
    const int tile = x * gps->dimy + y;

    if (df::viewscreen_dwarfmodest::_identity.is_direct_instance(Gui::getCurViewscreen()) && x > 0 && y > 0 && y < gps->dimy-1 && x < gps->dimx-gmenu_w-1)
    {
        const unsigned char *s = r->screen + tile*4;
        if (s[0] == 0)
        {
            memset(fg, 0, sizeof(GLfloat)*6*4);
            memset(bg, 0, sizeof(GLfloat)*6*4);
            return;
        }
    }

    struct texture_fullid ret;
    screen_to_texid_text(r, tile, ret);

    for (int i = 0; i < 6; i++) {
        *(fg++) = ret.r;
        *(fg++) = ret.g;
        *(fg++) = ret.b;
        *(fg++) = 1;
        
        *(bg++) = ret.br;
        *(bg++) = ret.bg;
        *(bg++) = ret.bb;
        *(bg++) = 1;
    }

    //TODO: handle special cases of graphics tiles outside of the map here with is_text_tile as before (aux font)
    
    // Set texture coordinates
    gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;
    *(tex++) = txt[ret.texpos].left;   // Upper left
    *(tex++) = txt[ret.texpos].bottom;
    *(tex++) = txt[ret.texpos].right;  // Upper right
    *(tex++) = txt[ret.texpos].bottom;
    *(tex++) = txt[ret.texpos].left;   // Lower left
    *(tex++) = txt[ret.texpos].top;
    
    *(tex++) = txt[ret.texpos].left;   // Lower left
    *(tex++) = txt[ret.texpos].top;
    *(tex++) = txt[ret.texpos].right;  // Upper right
    *(tex++) = txt[ret.texpos].bottom;
    *(tex++) = txt[ret.texpos].right;  // Lower right
    *(tex++) = txt[ret.texpos].top;
}

static void write_tile_arrays_map(renderer_cool *r, int x, int y, GLfloat *fg, GLfloat *bg, GLfloat *tex)
{
    struct texture_fullid ret;
    const int tile = x * r->gdimy + y;        
    screen_to_texid_map(r, tile, ret);

    for (int i = 0; i < 6; i++) {
        *(fg++) = ret.r;
        *(fg++) = ret.g;
        *(fg++) = ret.b;
        *(fg++) = 1;
        
        *(bg++) = ret.br;
        *(bg++) = ret.bg;
        *(bg++) = ret.bb;
        *(bg++) = 1;
    }
    
    if (has_overrides)
    {
        const unsigned char *s = gscreen + tile*4;
        int s0 = s[0];

        if (overrides[s0])
        {
            int xx = *df::global::window_x + x;
            int yy = *df::global::window_y + y;
            int zz = *df::global::window_z - ((s[3]&0xf0)>>4);
            bool matched = false;

            // Items / buildings
            for (int j = 0; j < overrides[s0]->size(); j++)
            {
                struct override &o = (*overrides[s0])[j];

                if (o.building)
                {
                    auto ilist = world->buildings.other[o.id];
                    for (auto it = ilist.begin(); it != ilist.end(); it++)
                    {
                        df::building *bld = *it;
                        if (zz != bld->z || xx < bld->x1 || xx > bld->x2 || yy < bld->y1 || yy > bld->y2)
                            continue;
                        if (o.type != -1 && bld->getType() != o.type)
                            continue;
                        if (o.subtype != -1 && bld->getSubtype() != o.subtype)
                            continue;

                        ret.texpos = enabler->fullscreen ? o.large_texpos : o.small_texpos;

                        matched = true;
                        break;
                    }
                }
                else
                {
                    auto ilist = world->items.other[o.id];
                    for (auto it = ilist.begin(); it != ilist.end(); it++)
                    {
                        df::item *item = *it;
                        if (!(zz == item->pos.z && xx == item->pos.x && yy == item->pos.y))
                            continue;
                        if (item->flags.whole & bad_item_flags.whole)
                            continue;
                        if (o.type != -1 && item->getType() != o.type)
                            continue;
                        if (o.subtype != -1 && item->getSubtype() != o.subtype)
                            continue;

                        ret.texpos = enabler->fullscreen ? o.large_texpos : o.small_texpos;

                        matched = true;
                        break;
                    }
                }
            }
        }
    }
    
    // Set texture coordinates
    gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;
    *(tex++) = txt[ret.texpos].left;   // Upper left
    *(tex++) = txt[ret.texpos].bottom;
    *(tex++) = txt[ret.texpos].right;  // Upper right
    *(tex++) = txt[ret.texpos].bottom;
    *(tex++) = txt[ret.texpos].left;   // Lower left
    *(tex++) = txt[ret.texpos].top;
    
    *(tex++) = txt[ret.texpos].left;   // Lower left
    *(tex++) = txt[ret.texpos].top;
    *(tex++) = txt[ret.texpos].right;  // Upper right
    *(tex++) = txt[ret.texpos].bottom;
    *(tex++) = txt[ret.texpos].right;  // Lower right
    *(tex++) = txt[ret.texpos].top;
}

#include "renderer.hpp"

// Disables standard rendering of lower levels
//TODO: explain how to find these addresses
static void patch_rendering(bool enable_lower_levels)
{
    static bool ready = false;
    static unsigned char orig[15];

#ifdef WIN32
    void *addr = (void*)(0x00b56370 + Core::getInstance().vinfo->getRebaseDelta());

    // mov eax, dword [ss:esp+0x0c]
    // mov byte [ds:eax], 0x20
    // retn 0x1c    
    unsigned char patch[] = { 0x36,0x8b,0x84,0x24,0x0C,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC2,0x1C,0x00 };

#elif defined(__APPLE__)
    void *addr = (void*)0x00af6c30;

    // mov eax, dword [ss:esp+0x14]
    // mov byte [ds:eax], 0x20
    // ret
    unsigned char patch[] = { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc6,0x00,0x00, 0xC3 };
    //unsigned char patch[] = { 0x36,0x8b,0x84,0x24,0x14,0x00,0x00,0x00, 0x3e,0xc7,0x00,0x20,0x00,0x00,0x00, 0xC3 };

#else
    #define NO_RENDERING_PATCH
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

static void hook()
{
    if (enabled)
        return;

    MemoryPatcher p(Core::getInstance().p);    

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
    enabler->renderer = newr;

    unsigned char nop6[] = { 0x90,0x90,0x90,0x90,0x90,0x90 };

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
    /*p.write((void*)0x002cbbb0, t1, sizeof(t1));
    p.write((void*)0x002cbf8d, t1, sizeof(t1));    
    p.write((void*)0x002cc288, t1, sizeof(t1));    
    p.write((void*)0x002cc225, t1, sizeof(t1));*/
    //p.write((void*)0x002cc306, t1, sizeof(t1));    

#else
    #error Linux not supported yet
#endif

    enabled = true;   
}

static void unhook()
{
    /*if (!enabled)
        return;

    enabled = false;

    gps->force_full_display_count = true;*/
}

#include "dwarfmode.hpp"
#include "dungeonmode.hpp"
#include "tradefix.hpp"
#include "config.hpp"
#include "commands.hpp"


DFhackCExport command_result plugin_init ( color_ostream &out, vector <PluginCommand> &commands)
{
    auto dflags = init->display.flag;
    if (!dflags.is_set(init_display_flags::USE_GRAPHICS))
    {
        out.color(COLOR_RED);
        out << "TWBT: GRAPHICS is not enabled in init.txt" << std::endl;
        out.color(COLOR_RESET);
        return CR_OK;
    }
    if (dflags.is_set(init_display_flags::RENDER_2D) ||
        dflags.is_set(init_display_flags::ACCUM_BUFFER) ||
        dflags.is_set(init_display_flags::FRAME_BUFFER) ||
        dflags.is_set(init_display_flags::TEXT) ||
        dflags.is_set(init_display_flags::VBO) ||
        dflags.is_set(init_display_flags::PARTIAL_PRINT))
    {
        out.color(COLOR_RED);
        out << "TWBT: PRINT_MODE must be set to STANDARD in init.txt" << std::endl;
        out.color(COLOR_RESET);
        return CR_OK;        
    }

    out2 = &out;

#ifdef WIN32
    load_multi_pdim = (LOAD_MULTI_PDIM) (0x00a52670 + Core::getInstance().vinfo->getRebaseDelta());
#elif defined(__APPLE__)
    load_multi_pdim = (LOAD_MULTI_PDIM) 0x00cfbbb0;    
#else
    load_multi_pdim = (LOAD_MULTI_PDIM) dlsym(RTLD_DEFAULT, "_ZN8textures15load_multi_pdimERKSsPlllbS2_S2_");
    #error Linux not supported yet
#endif

    bad_item_flags.whole = 0;
    bad_item_flags.bits.in_building = true;
    bad_item_flags.bits.garbage_collect = true;
    bad_item_flags.bits.removed = true;
    bad_item_flags.bits.dead_dwarf = true;
    bad_item_flags.bits.murder = true;
    bad_item_flags.bits.construction = true;
    bad_item_flags.bits.in_inventory = true;
    bad_item_flags.bits.in_chest = true;

    // Used only if rendering patch is not available
    skytile = d_init->sky_tile;
    chasmtile = d_init->chasm_tile;    

    // Graphics tileset - accessible at index 0
    struct tileset ts;
    memcpy(ts.small_texpos, init->font.small_font_texpos, sizeof(ts.small_texpos));
    memcpy(ts.large_texpos, init->font.large_font_texpos, sizeof(ts.large_texpos));
    tilesets.push_back(ts);

    // We will replace init->font with text font, so let's save graphics tile size
    small_map_dispx = init->font.small_font_dispx, small_map_dispy = init->font.small_font_dispy;
    large_map_dispx = init->font.large_font_dispx, large_map_dispy = init->font.large_font_dispy;

    has_textfont = load_text_font();
    has_overrides = load_overrides();

    if (!has_textfont)
    {
        out.color(COLOR_YELLOW);
        out << "TWBT: FONT and GRAPHICS_FONT are the same" << std::endl;
        out.color(COLOR_RESET);        
    }

    // Load shadows
    struct stat buf;
    if (stat("data/art/shadows.png", &buf) == 0)
    {
        long dx, dy;        
        load_tileset("data/art/shadows.png", shadow_texpos, 8, 1, &dx, &dy);
        shadowsloaded = true;
    }
    else
    {
        out2->color(COLOR_RED);
        *out2 << "TWBT: shadows.png not found in data/art folder" << std::endl;
        out2->color(COLOR_RESET);
    }

    map_texpos = enabler->fullscreen ? tilesets[0].large_texpos : tilesets[0].small_texpos;
    text_texpos = enabler->fullscreen ? tilesets[1].large_texpos : tilesets[1].small_texpos;

    hook();

    INTERPOSE_HOOK(dwarfmode_hook, render).apply(1);
    INTERPOSE_HOOK(dwarfmode_hook, feed).apply(1);
    //INTERPOSE_HOOK(zzz2, render).apply(1);
    //INTERPOSE_HOOK(zzz2, feed).apply(1);

#ifdef __APPLE__
    INTERPOSE_HOOK(traderesize_hook, render).apply(true);
#endif    

    commands.push_back(PluginCommand(
        "mapshot", "Mapshot!",
        mapshot_cmd, false, /* true means that the command can't be used from non-interactive user interface */
        // Extended help string. Used by CR_WRONG_USAGE and the help command:
        ""
    ));        
    commands.push_back(PluginCommand(
        "multilevel", "Multilivel rendering",
        multilevel_cmd, false, /* true means that the command can't be used from non-interactive user interface */
        // Extended help string. Used by CR_WRONG_USAGE and the help command:
        ""
    ));       
    commands.push_back(PluginCommand(
        "colormap", "Colomap manipulation",
        colormap_cmd, false, /* true means that the command can't be used from non-interactive user interface */
        // Extended help string. Used by CR_WRONG_USAGE and the help command:
        ""
    ));       
    commands.push_back(PluginCommand(
        "twbt", "Text Will Be Text",
        twbt_cmd, false, /* true means that the command can't be used from non-interactive user interface */
        // Extended help string. Used by CR_WRONG_USAGE and the help command:
        ""
    ));   

    return CR_OK;
}

DFhackCExport command_result plugin_shutdown ( color_ostream &out )
{
    return CR_FAILURE;

    /*if (enabled)
        unhook();

#ifdef __APPLE__
    INTERPOSE_HOOK(traderesize_hook, render).apply(false);
#endif            

    return CR_OK;*/
}