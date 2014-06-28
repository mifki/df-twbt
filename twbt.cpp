//
//  twbt.cpp
//  twbt
//
//  Created by Vitaly Pronkin on 14/05/14.
//  Copyright (c) 2014 mifki. All rights reserved.
//

#include <sys/stat.h>
#include <stdint.h>
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
#include <VTableInterpose.h>
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
#include "df/renderer.h"
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

using df::global::world;
using std::string;
using std::vector;
using df::global::enabler;
using df::global::gps;
using df::global::ui;
using df::global::init;

struct texture_fullid {
    int texpos;
    float r, g, b;
    float br, bg, bb;
};

struct gl_texpos {
    GLfloat left, right, top, bottom;
};

DFHACK_PLUGIN("twbt");

void (*load_multi_pdim)(void *tex,const string &filename, long *tex_pos, long dimx, long dimy, bool convert_magenta, long *disp_x, long *disp_y);

#ifdef WIN32
__declspec(naked) void load_multi_pdim_x(void *tex, const string &filename, long *tex_pos, long dimx, long dimy, bool convert_magenta, long *disp_x, long *disp_y)
{
    __asm {
        push ebp
        mov ebp, esp

        push disp_y
        push disp_x
        push dimy
        push dimx
        mov ecx, tex_pos
        push tex
        mov edx, filename

        call load_multi_pdim

        mov esp, ebp
        pop ebp
        ret
    }    
}
#else
#define load_multi_pdim_x load_multi_pdim
#endif        

struct tileset {
    string small_font_path;
    string large_font_path;
    long small_texpos[16*16], large_texpos[16*16];
};

struct tileref {
    int tilesetidx;
    int tile;
};

struct override {
    bool building;
    int id, type, subtype;
    struct tileref newtile;
};





static vector< struct tileset > tilesets;

static bool enabled, texloaded;
static bool has_textfont, has_overrides;
static color_ostream *out2;
static vector< struct override > *overrides[256];
static struct tileref override_defs[256];
static df::item_flags bad_item_flags;

df::viewscreen *sepview = 0;

bool is_text_tile(int x, int y, bool &is_map)
{
    const int tile = x * gps->dimy + y;
    df::viewscreen * ws = Gui::getCurViewscreen();

    int32_t w = gps->dimx, h = gps->dimy;

    is_map = false;

    if (!has_textfont)
        return false;    

    if (!x || !y || x == w - 1 || y == h - 1)
       return true;

#define IS_SCREEN(_sc) df::_sc::_identity.is_direct_instance(ws) 

    if (IS_SCREEN(viewscreen_dwarfmodest))
    {
        //*out2 << ((long**)ws)[0][2] << std::endl;
        //*out2 << (long)enabler->renderer->screen << std::endl;
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
                    return true;
            }
            else
                return true;
        }

        is_map = (x > 0 && x < menu_left);

        return false;
    }
    
    if (IS_SCREEN(viewscreen_setupadventurest))
    {
        df::viewscreen_setupadventurest *s = static_cast<df::viewscreen_setupadventurest*>(ws);
        if (s->subscreen != df::viewscreen_setupadventurest::Nemesis)
            return true;
        else if (x < 58 || x >= 78 || y == 0 || y >= 21)
            return true;

        return false;
    }
    
    if (IS_SCREEN(viewscreen_dungeonmodest))
    {
        //df::viewscreen_dungeonmodest *s = strict_virtual_cast<df::viewscreen_dungeonmodest>(ws);
        //TODO

        if (y >= h-2)
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

float addcolors[][3] = { {1,0,0} };
unsigned char shadows[200*200];
GLfloat shadowtex[200*200*2*6];
GLfloat shadowvert[200*200*2*6];
long shadow_texpos[8];
bool shadowsloaded;

void screen_to_texid2(df::renderer *r, int x, int y, struct texture_fullid &ret) {
    const int tile = x * gps->dimy + y;
    const unsigned char *s = r->screen + tile*4;

    int ch;
    int bold;
    int fg;
    int bg;

    ch   = s[0];
    bold = (s[3]&0x0f) * 8;
    fg   = (s[1] + bold);
    bg   = s[2] % 16;

    const long texpos             = r->screentexpos[tile];
    const char addcolor           = r->screentexpos_addcolor[tile];
    const unsigned char grayscale = r->screentexpos_grayscale[tile];
    const unsigned char cf        = r->screentexpos_cf[tile];
    const unsigned char cbr       = r->screentexpos_cbr[tile];

    if (texpos) {
      ret.texpos = texpos;
      if (grayscale) {
        ret.r = enabler->ccolor[cf][0];
        ret.g = enabler->ccolor[cf][1];
        ret.b = enabler->ccolor[cf][2];
        ret.br = enabler->ccolor[cbr][0];
        ret.bg = enabler->ccolor[cbr][1];
        ret.bb = enabler->ccolor[cbr][2];
      } else if (addcolor) {
        goto use_ch;
      } else {
        ret.r = ret.g = ret.b = 1;
        ret.br = ret.bg = ret.bb = 0;
      }
      return;
    }
  
  ret.texpos = enabler->fullscreen ?
    init->font.large_font_texpos[ch] :
    init->font.small_font_texpos[ch];

 use_ch:
 if (fg < 16)
 {
  ret.r = enabler->ccolor[fg][0];
  ret.g = enabler->ccolor[fg][1];
  ret.b = enabler->ccolor[fg][2];
}
else
{
  ret.r = addcolors[fg-16][0];
  ret.g = addcolors[fg-16][1];
  ret.b = addcolors[fg-16][2];
}
  ret.br = enabler->ccolor[bg][0];
  ret.bg = enabler->ccolor[bg][1];
  ret.bb = enabler->ccolor[bg][2];
}
unsigned char depth[200*200*4];
void write_tile_arrays(df::renderer *r, int x, int y, GLfloat *fg, GLfloat *bg, GLfloat *tex)
{
    struct texture_fullid ret;
    screen_to_texid2(r, x, y, ret);
    const int tile = x * gps->dimy + y;
    float a = 1;//+(((r->screen[tile*4+3]&0xf0)>>4))*0.2;
    for (int i = 0; i < 6; i++) {
        *(fg++) = ret.r/a;
        *(fg++) = ret.g/a;
        *(fg++) = ret.b/a;
        *(fg++) = 1;
        
        *(bg++) = ret.br/a;
        *(bg++) = ret.bg/a;
        *(bg++) = ret.bb/a;
        *(bg++) = 1;
    }
    
    bool is_map;
    if (is_text_tile(x, y, is_map))
    {
        const int tile = x * gps->dimy + y;
        const unsigned char *s = r->screen + tile*4;
        ret.texpos = enabler->fullscreen ? tilesets[1].large_texpos[s[0]] : tilesets[1].small_texpos[s[0]];
    }
    else if (is_map && has_overrides)
    {
        const unsigned char *s = r->screen + tile*4;
        int s0 = s[0];
        if (overrides[s0])
        {
            int xx = *df::global::window_x + x-1;
            int yy = *df::global::window_y + y-1;
            int zz = *df::global::window_z;
            bool matched = false;

            // Items
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

                        ret.texpos = enabler->fullscreen ?
                            tilesets[o.newtile.tilesetidx].large_texpos[o.newtile.tile] :
                            tilesets[o.newtile.tilesetidx].small_texpos[o.newtile.tile];

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

                        ret.texpos = enabler->fullscreen ?
                            tilesets[o.newtile.tilesetidx].large_texpos[o.newtile.tile] :
                            tilesets[o.newtile.tilesetidx].small_texpos[o.newtile.tile];

                        matched = true;
                        break;
                    }
                }
            }

            // Default
            if (!matched && override_defs[s0].tile)
                ret.texpos = enabler->fullscreen ?
                    tilesets[override_defs[s0].tilesetidx].large_texpos[override_defs[s0].tile] :
                    tilesets[override_defs[s0].tilesetidx].small_texpos[override_defs[s0].tile];
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

float fogcoord[200*200*6];
#include "renderer.hpp"



void hook()
{
    if (enabled)
        return;

    //TODO: check for opengl renderer, graphics, show msg otherwise

    renderer_opengl *oldr = (renderer_opengl*)enabler->renderer;
    renderer_cool *newr = new renderer_cool;

    long **vtable_old = (long **)oldr;
    long **vtable_new = (long **)newr;

#ifdef WIN32
    long draw_new = vtable_new[0][13];//14 on osx
#else
    long draw_new = vtable_new[0][14];//14 on osx
#endif
    long update_tile_new = vtable_new[0][0];

    /*for (int i = 0; i < 20; i++)
        *out2 << "$ " << i << " " << (long)vtable_old[0][i] << std::endl;
    for (int i = 0; i < 24; i++)
        *out2 << "$ " << i << " " << (long)vtable_new[0][i] << std::endl;*/

#ifdef WIN32
    HANDLE process = ::GetCurrentProcess();
    DWORD protection = PAGE_READWRITE;
    DWORD oldProtection;
    if ( ::VirtualProtectEx( process, vtable_new[0], 18*sizeof(void*), protection, &oldProtection ) )
    {
        memcpy(vtable_new[0], vtable_old[0], sizeof(void*)*16);
        vtable_new[0][13] = draw_new;
        vtable_new[0][0] = update_tile_new;
        vtable_new[0][16] = vtable_old[0][0];
        VirtualProtectEx( process, vtable_new[0], 18*sizeof(void*), oldProtection, &oldProtection );
    }
#else
    memcpy(vtable_new[0], vtable_old[0], sizeof(void*)*17);
    vtable_new[0][14] = draw_new;
    vtable_new[0][0] = update_tile_new;
    vtable_new[0][17] = vtable_old[0][0];
#endif
    
    memcpy(&newr->screen, &oldr->screen, (char*)&newr->dummy-(char*)&newr->screen);
    enabler->renderer = newr;
    //free(oldr);

    //*out2 << (char*)&newr->vertexes-(char*)newr << std::endl;

    enabled = true;   
}

void unhook()
{
    if (!enabled)
        return;

    //TODO: !!!

    enabled = false;
    gps->force_full_display_count = true;
}

unsigned char screen2[200*200*4];
    int32_t screentexpos2[200*200];
    int8_t screentexpos_addcolor2[200*200];
    uint8_t screentexpos_grayscale2[200*200];
    uint8_t screentexpos_cf2[200*200];
    uint8_t screentexpos_cbr2[200*200];

#define CHECK(dx,dy) \
{\
    const int tile = (x+dx) * gps->dimy + (y+dy); \
    {\
    df::map_block *block = world->map.block_index[(xx+dx) >> 4][(yy+dy) >> 4][zz]; \
    if (block) {\
        df::tiletype tt = block->tiletype[(xx+dx)&15][(yy+dy)&15];\
        k = tt != df::tiletype::OpenSpace && tt != df::tiletype::RampTop;\
    }}\
}

static int _min(int a, int b)
{
    return (a < b) ? a : b;
}

struct zzz : public df::viewscreen_dwarfmodest
{
    typedef df::viewscreen_dwarfmodest interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, render, ())
    {
        INTERPOSE_NEXT(render)();

        if (shadowsloaded)
            render_more_layers();
    }

    void render_more_layers()
    {
        int32_t w = gps->dimx, h = gps->dimy;
        uint8_t menu_width, area_map_width;
        Gui::getMenuWidth(menu_width, area_map_width);
        int32_t menu_left = w - 1;

        bool menuforced = (ui->main.mode != df::ui_sidebar_mode::Default || df::global::cursor->x != -30000);

        if ((menuforced || menu_width == 1) && area_map_width == 2) // Menu + area map
        {
            menu_left = w - 56;
        }
        else if (menu_width == 2 && area_map_width == 2) // Area map only
        {
            menu_left = w - 25;
        }
        else if (menu_width == 1) // Wide menu
            menu_left = w - 56;
        else if (menuforced || (menu_width == 2 && area_map_width == 3)) // Menu only
            menu_left = w - 32; 


        uint8_t *sctop = enabler->renderer->screen;
        int32_t *screentexpostop = enabler->renderer->screentexpos;
        int8_t *screentexpos_addcolortop = enabler->renderer->screentexpos_addcolor;
        uint8_t *screentexpos_grayscaletop = enabler->renderer->screentexpos_grayscale;
        uint8_t *screentexpos_cftop = enabler->renderer->screentexpos_cf;
        uint8_t *screentexpos_cbrtop = enabler->renderer->screentexpos_cbr;

        gps->screen = screen2;
        gps->screen_limit = gps->screen + gps->dimx * gps->dimy * 4;
        gps->screentexpos = screentexpos2;
        gps->screentexpos_addcolor = screentexpos_addcolor2;
        gps->screentexpos_grayscale = screentexpos_grayscale2;
        gps->screentexpos_cf = screentexpos_cf2;
        gps->screentexpos_cbr = screentexpos_cbr2;

        //this->*this->interpose_render.get_first_interpose(&df::viewscreen_dwarfmodest::_identity).saved_chain;

        void (*render_map)(void *, int) = (void (*)(void *, int))0x0084b4c0;

        bool empty_tiles_left;
        int p = 1;
        int x0 = 1;
        int zz0 = *df::global::window_z;        
        do
        {
            //TODO: if z=0 should just render and use for all tiles always
            if (*df::global::window_z == 0)
                break;

            (*df::global::window_z)--;

            (*df::global::window_x) += x0-1;
            init->display.grid_x -= x0-1;

            INTERPOSE_NEXT(render)();
            //render_map(df::global::cursor_unit_list, 1);

            (*df::global::window_x) -= x0-1;
            init->display.grid_x += x0-1;

            empty_tiles_left = false;
            int x00 = x0;
            int zz = zz0 - p + 1;

            //*out2 << p << " " << x0 << std::endl;
            
            GLfloat *vertices = ((renderer_opengl*)enabler->renderer)->vertexes;
            //TODO: test this
            int x1 = _min(menu_left, world->map.x_count-*df::global::window_x+1);
            int y1 = _min(h-1, world->map.y_count-*df::global::window_y+1);
            for (int x = x0; x < x1; x++)
            {
                for (int y = 1; y < y1; y++)
                {
                    const int tile = x * gps->dimy + y;
                    const int tile2 = (x-(x00-1)) * gps->dimy + y;

                    if ((sctop[tile*4+3]&0xf0))
                        continue;

                    unsigned char ch = sctop[tile*4+0];
                    if (ch != 31 && ch != 249 && ch != 250 && ch != 254 && ch != 32 && ch != 0 && !(ch >= '1' && ch <= '7'))
                        continue;

                    int xx = *df::global::window_x + x-1;
                    int yy = *df::global::window_y + y-1;
                    if (xx < 0 || yy < 0)
                        continue;

                    //TODO: check for z=0
                    bool e0,h,h0;
                    //*out2 << xx << " " << world->map.x_count << " " << yy << " " << world->map.y_count << " " << *df::global::window_x << " " << *df::global::window_y << std::endl;
                    df::map_block *block0 = world->map.block_index[xx >> 4][yy >> 4][zz0];
                    h0 = block0 && block0->designation[xx&15][yy&15].bits.hidden;
                    if (h0)
                        continue;
                    e0 = !block0 || (block0->tiletype[xx&15][yy&15] == df::tiletype::OpenSpace || block0->tiletype[xx&15][yy&15] == df::tiletype::RampTop);
                    if (!(e0))
                        continue;

                    ch = screen2[tile2*4+0];
                    if (!(ch!=31&&ch != 249 && ch != 250 && ch != 254 && ch != 32 && ch != 0 && !(ch >= '1' && ch <= '7')))
                    {
                        df::map_block *block1 = world->map.block_index[xx >> 4][yy >> 4][zz-1];
                        if (!block1)
                        {
                            //TODO: skip all other y's in this block
                            empty_tiles_left = true;
                            continue;
                        }

                        //TODO: check for hidden also
                        df::tiletype t1 = block1->tiletype[xx&15][yy&15];
                        if (t1 == df::tiletype::OpenSpace || t1 == df::tiletype::RampTop)
                        {
                            empty_tiles_left = true;
                            continue;
                        }
                    }

                    //*out2 << p << " !" << std::endl;
                    *((int*)sctop+tile) = *((int*)screen2+tile2);
                    if (*(screentexpos2+tile))
                    {
                        *(screentexpostop+tile) = *(screentexpos2+tile2);
                        *(screentexpos_addcolortop+tile) = *(screentexpos_addcolor2+tile2);
                        *(screentexpos_grayscaletop+tile) = *(screentexpos_grayscale2+tile2);
                        *(screentexpos_cftop+tile) = *(screentexpos_cf2+tile2);
                        *(screentexpos_cbrtop+tile) = *(screentexpos_cbr2+tile2);
                    }

                    int k = false;
                    int kk = 0;
                    do
                    {
                        if (xx>0)
                        {
                            CHECK(-1,0)
                            kk |= k<<0;
                        }
                        if (yy < world->map.y_count-1)
                        {
                            CHECK(0,1)
                            kk |= k<<1;
                        }
                        if (yy > 0)
                        {
                            CHECK(0,-1)
                            kk |= k<<2;
                        }
                        if (xx < world->map.x_count-1)
                        {
                            CHECK(1,0)
                            kk |= k<<3;
                        }
                        if (xx < world->map.x_count-1 && yy < world->map.y_count-1 && !(kk & (1<<1)) && !(kk & (1<<3)))
                        {
                            CHECK(1,1)
                            kk |= k << 4;
                        }

                        if (xx > 0 && yy < world->map.y_count-1 && !(kk & (1<<0)) && !(kk & (1<<1)))
                        {
                            CHECK(-1,1)
                            kk |= k<<5;
                        }
                        if (xx > 0 && yy > 0 && !(kk & (1<<0)) && !(kk & (1<<2)))
                        {
                            CHECK(-1,-1)
                            kk |= k<<6;
                        }
                        if (xx < world->map.x_count-1 && yy > 0 && !(kk & (1<<2)) && !(kk & (1<<3)))
                        {
                            CHECK(1,-1)
                            kk |= k<<7;
                        }

                        shadows[tile] = kk;

                    } while(0);
                    sctop[tile*4+3] = (0x10*p) | (sctop[tile*4+3]&0x0f);
                }
                if (!empty_tiles_left)
                    x0 = x + 1;
            }

            if (p++ >= 10)
                break;
        } while(empty_tiles_left);

        (*df::global::window_z) = zz0;

        gps->screen = sctop;
        gps->screen_limit = gps->screen + gps->dimx * gps->dimy * 4;
        gps->screentexpos = screentexpostop;
        gps->screentexpos_addcolor = screentexpos_addcolortop;
        gps->screentexpos_grayscale = screentexpos_grayscaletop;
        gps->screentexpos_cf = screentexpos_cftop;
        gps->screentexpos_cbr = screentexpos_cbrtop;
    }    
};

//TODO: Priority?
IMPLEMENT_VMETHOD_INTERPOSE(zzz, render);

#include "config.hpp"

command_result mapshot (color_ostream &out, std::vector <std::string> & parameters)
{
    CoreSuspender suspend;

    domapshot = 10;

    return CR_OK;    
}

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
    load_multi_pdim = (void (*)(void *tex, const string &filename, long *tex_pos, long dimx,
        long dimy, bool convert_magenta, long *disp_x, long *disp_y)) (0x00a52670+(Core::getInstance().vinfo->getRebaseDelta()));    
#else
    load_multi_pdim = (void (*)(void *tex, const string &filename, long *tex_pos, long dimx,
        long dimy, bool convert_magenta, long *disp_x, long *disp_y)) 0x00cfbbb0;    
#endif
    //_ZN8textures15load_multi_pdimERKSsPlllbS2_S2_

    bad_item_flags.whole = 0;
    bad_item_flags.bits.in_building = true;
    bad_item_flags.bits.garbage_collect = true;
    bad_item_flags.bits.removed = true;
    bad_item_flags.bits.dead_dwarf = true;
    bad_item_flags.bits.murder = true;
    bad_item_flags.bits.construction = true;
    bad_item_flags.bits.in_inventory = true;
    bad_item_flags.bits.in_chest = true;

    //Main tileset
    struct tileset ts;
    memcpy(ts.small_texpos, df::global::init->font.small_font_texpos, sizeof(ts.small_texpos));
    memcpy(ts.large_texpos, df::global::init->font.large_font_texpos, sizeof(ts.large_texpos));
    tilesets.push_back(ts);

    memset(override_defs, sizeof(struct tileref)*256, 0);

    has_textfont = get_font_paths();
    has_overrides |= load_overrides();
    if (has_textfont || has_overrides)
        hook();

    INTERPOSE_HOOK(zzz, render).apply(1);

    commands.push_back(PluginCommand(
        "mapshot", "Mapshot!",
        mapshot, false, /* true means that the command can't be used from non-interactive user interface */
        // Extended help string. Used by CR_WRONG_USAGE and the help command:
        ""
    ));        

    return CR_OK;
}

DFhackCExport command_result plugin_shutdown ( color_ostream &out )
{
    if (enabled)
        unhook();

    return CR_OK;
}