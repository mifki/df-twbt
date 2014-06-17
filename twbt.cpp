//
//  twbt.cpp
//  twbt
//
//  Created by Vitaly Pronkin on 14/05/14.
//  Copyright (c) 2014 mifki. All rights reserved.
//

#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "modules/Maps.h"
#include "modules/World.h"
#include "modules/MapCache.h"
#include "modules/Gui.h"
#include "modules/Screen.h"
#include "modules/Buildings.h"
#include "MemAccess.h"
#include "VersionInfo.h"
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
#include <VTableInterpose.h>
#include <OpenGL/gl.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

typedef float GLfloat;
typedef unsigned int GLuint;

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

vector<string> split(const char *str, char c = ' ')
{
    vector<string> result;

    do
    {
        const char *begin = str;

        while(*str != c && *str)
            str++;

        result.push_back(string(begin, str));
    } while (0 != *str++);

    return result;
}

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
        push 10h
        push 10h
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

// This is from g_src/renderer_opengl.hpp
struct renderer_opengl : df::renderer
{
    void *sdlscreen;
    int dispx, dispy;
    GLfloat *vertexes, *fg, *bg, *tex;
    int zoom_steps, forced_steps;
    int natural_w, natural_h;
    int off_x, off_y, size_x, size_y;

    virtual void allocate(int tiles) {};
    virtual void init_opengl() {};
    virtual void uninit_opengl() {};
    virtual void draw(int vertex_count) {};
    virtual void opengl_renderer_destructor() {};
    virtual void reshape_gl() {};
};

struct renderer_cool : renderer_opengl
{
    // To know the size of renderer_opengl's fields
    void *dummy;

    virtual void update_tile(int x, int y);
    virtual void draw(int vertex_count);

    virtual void update_tile_old(int x, int y) {}; //17
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

    //*out2 << Core::getInstance().p->readClassName(*(void**)ws) << std::endl;

    return true;
}

#define SETTEX(tt) \
    *(tex++) = txt[tt].left; \
    *(tex++) = txt[tt].bottom; \
    *(tex++) = txt[tt].right; \
    *(tex++) = txt[tt].bottom; \
    *(tex++) = txt[tt].left; \
    *(tex++) = txt[tt].top; \
    \
    *(tex++) = txt[tt].left; \
    *(tex++) = txt[tt].top; \
    *(tex++) = txt[tt].right; \
    *(tex++) = txt[tt].bottom; \
    *(tex++) = txt[tt].right; \
    *(tex++) = txt[tt].top;

float addcolors[][3] = { {1,0,0} };
unsigned char shadows[100*100];
unsigned char mod[80*25];
GLfloat shadowtex[100*100*2*6];
GLfloat shadowvert[100*100*2*6];
short elems[100*100];
short elemcnt;


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
unsigned char depth[100*100*4];
void write_tile_arrays(df::renderer *r, int x, int y, GLfloat *fg, GLfloat *bg, GLfloat *tex)
{
    struct texture_fullid ret;
    screen_to_texid2(r, x, y, ret);
    const int tile = x * gps->dimy + y;
    float a = 1 + (float)((r->screen[tile*4+3]&0xf0)>>4)*0.2;
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


void renderer_cool::update_tile(int x, int y)
{
    if (!enabled || !texloaded)
    {
        this->update_tile_old(x, y);
        return;
    }

    const int tile = x*gps->dimy + y;
    mod[tile] = 0;

    GLfloat *_fg  = fg + tile * 4 * 6;
    GLfloat *_bg  = bg + tile * 4 * 6;
    GLfloat *_tex = tex + tile * 2 * 6;

    write_tile_arrays(this, x, y, _fg, _bg, _tex);
}

#define ADDTILEVERT(x,y,w,h) \
    vertex[0]  = x; \
    vertex[1]  = y; \
    vertex[2]  = x+w; \
    vertex[3]  = y; \
    vertex[4]  = x; \
    vertex[5]  = y+h; \
    vertex[6]  = x; \
    vertex[7]  = y+h; \
    vertex[8]  = x+w; \
    vertex[9]  = y; \
    vertex[10] = x+w; \
    vertex[11] = y+h;

void renderer_cool::draw(int vertex_count)
{
    if (!texloaded)
    {
        long dx, dy;
        void *t = &enabler->textures;

        for (int j = 0; j < tilesets.size(); j++)
        {
            struct tileset &ts = tilesets[j];
            if (!ts.small_font_path.length())
                continue;

            load_multi_pdim_x(t, ts.small_font_path, tilesets[j].small_texpos, 16, 16, true, &dx, &dy);
            if (ts.large_font_path != ts.small_font_path)
                load_multi_pdim_x(t, ts.large_font_path, tilesets[j].large_texpos, 16, 16, true, &dx, &dy);
            else
                memcpy(ts.large_texpos, ts.small_texpos, sizeof(ts.large_texpos));
        }

        texloaded = true;
        gps->force_full_display_count = true;
    }

    glVertexPointer(2, GL_FLOAT, 0, vertexes);
    // Render the background colors
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glColorPointer(4, GL_FLOAT, 0, bg);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    // Render the foreground, colors and textures both
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexCoordPointer(2, GL_FLOAT, 0, tex);
    glColorPointer(4, GL_FLOAT, 0, fg);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);


    elemcnt = 0;
for (int tile = 0; tile < gps->dimx*gps->dimy; tile++)
{
    if ((this->screen[tile*4+3]&0xf0))
    {
        unsigned char kk = shadows[tile];
        int x = tile / gps->dimy;
        int y = tile % gps->dimy;
        GLfloat *tex = shadowtex+elemcnt*2;
        float *vertex = shadowvert+elemcnt*2;
        float w = 1+std::max<float>(0, (x - (float)(gps->dimx-32) / 2) / ((gps->dimx - 32) / 2));
        float w2 = 1+std::max<float>(0, ((float)(gps->dimx-32) / 2 - x) / ((gps->dimx - 32) / 2));
        float h = 1+std::max<float>(0, (y - (float)(gps->dimy) / 2) / ((gps->dimy) / 2));
        float h2 = 1+std::max<float>(0, ((float)(gps->dimy) / 2 - y) / ((gps->dimy) / 2));
        //w=w2=h=h2=1;
        if (kk & (1 << 0))
        {
            ADDTILEVERT(x,y,w,1)

            gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;
            SETTEX(0x70);
            elemcnt+=6;
            vertex+=6*2;
        }
        if (kk & (1 << 1))
        {
            ADDTILEVERT(x,y+1-h2,1,h2)

            gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;
            SETTEX(0x71);
            elemcnt+=6;
            vertex+=6*2;            
        }
        if (kk & (1 << 2))
        {
            vertex[0]  = x;   // Upper left
            vertex[1]  = y;
            vertex[2]  = x+1; // Upper right
            vertex[3]  = y;
            vertex[4]  = x;   // Lower left
            vertex[5]  = y+h;
            vertex[6]  = x;   // Lower left again (triangle 2)
            vertex[7]  = y+h;
            vertex[8]  = x+1; // Upper right
            vertex[9]  = y;
            vertex[10] = x+1; // Lower right
            vertex[11] = y+h;            

            gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;
            SETTEX(0x72);
            elemcnt+=6;
            vertex+=6*2;            
        }
        if (kk & (1 << 3))
        {
            vertex[0]  = x+1-w2;   // Upper left
            vertex[1]  = y;
            vertex[2]  = x+1; // Upper right
            vertex[3]  = y;
            vertex[4]  = x+1-w2;   // Lower left
            vertex[5]  = y+1;
            vertex[6]  = x+1-w2;   // Lower left again (triangle 2)
            vertex[7]  = y+1;
            vertex[8]  = x+1; // Upper right
            vertex[9]  = y;
            vertex[10] = x+1; // Lower right
            vertex[11] = y+1;

            gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;
            SETTEX(0x73);
            elemcnt+=6;
            vertex+=6*2;
        }        
        if (kk & (1 << 4))
        {
            //*out2 << "a" << std::endl;
            vertex[0]  = x+1-w2;   // Upper left
            vertex[1]  = y+1-h2;

            vertex[2]  = x+1; // Upper right
            vertex[3]  = y+1-h2;

            vertex[4]  = x+1-w2;   // Lower left
            vertex[5]  = y+1;

            vertex[6]  = x+1-w2;   // Lower left again (triangle 2)
            vertex[7]  = y+1;

            vertex[8]  = x+1; // Upper right
            vertex[9]  = y+1-h2;

            vertex[10] = x+1; // Lower right
            vertex[11] = y+1;            

            gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;
            SETTEX(0x74);
            elemcnt+=6;
            vertex+=6*2;            
        }               
        if (kk & (1 << 5))
        {
            //*out2 << "a" << std::endl;
            vertex[0]  = x;   // Upper left
            vertex[1]  = y+1-h2;

            vertex[2]  = x+w; // Upper right
            vertex[3]  = y+1-h2;

            vertex[4]  = x;   // Lower left
            vertex[5]  = y+1;

            vertex[6]  = x;   // Lower left again (triangle 2)
            vertex[7]  = y+1;

            vertex[8]  = x+w; // Upper right
            vertex[9]  = y+1-h2;

            vertex[10] = x+w; // Lower right
            vertex[11] = y+1;            

            gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;
            SETTEX(0x75);
            elemcnt+=6;
            vertex+=6*2;            
        }            
        if (kk & (1 << 6))
        {
            //*out2 << "a" << std::endl;
            vertex[0]  = x;   // Upper left
            vertex[1]  = y;

            vertex[2]  = x+w; // Upper right
            vertex[3]  = y;

            vertex[4]  = x;   // Lower left
            vertex[5]  = y+h;

            vertex[6]  = x;   // Lower left again (triangle 2)
            vertex[7]  = y+h;

            vertex[8]  = x+w; // Upper right
            vertex[9]  = y;

            vertex[10] = x+w; // Lower right
            vertex[11] = y+h;            

            gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;
            SETTEX(0x76);
            elemcnt+=6;
            vertex+=6*2;            
        }        
        if (kk & (1 << 7))
        {
            //*out2 << "a" << std::endl;
            vertex[0]  = x+1-w2;   // Upper left
            vertex[1]  = y;

            vertex[2]  = x+1; // Upper right
            vertex[3]  = y;

            vertex[4]  = x+1-w2;   // Lower left
            vertex[5]  = y+h;

            vertex[6]  = x+1-w2;   // Lower left again (triangle 2)
            vertex[7]  = y+h;

            vertex[8]  = x+1; // Upper right
            vertex[9]  = y;

            vertex[10] = x+1; // Lower right
            vertex[11] = y+h;

            gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;
            SETTEX(0x77);
            elemcnt+=6;
            vertex+=6*2;            
        }                  
    } 
}


    glDisable(GL_ALPHA_TEST);
    //glDisable(GL_TEXTURE_2D);
    glColor4f(0,0,0,0.5);
    glDisableClientState(GL_COLOR_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, shadowtex);
    glVertexPointer(2, GL_FLOAT, 0, shadowvert);
    //glDrawElements(GL_TRIANGLES, elemcnt, GL_UNSIGNED_SHORT, elems);
    glDrawArrays(GL_TRIANGLES, 0, elemcnt);
    glEnableClientState(GL_COLOR_ARRAY);    
}



/*        static int z = 0;
        if (gps->dimx == 80)
        {
            z = 1;
        float pw = gps->dimx*16;
        float w = 30 + (pw-30*16*0.8) / 16;
        *out2 << w << std::endl;
        enabler->renderer->grid_resize((int)w, gps->dimy);


glViewport(0, 0, gps->dimx*16, 25*16);

static int z = 0;
       if (!z)
       {
        z = 1;

        GLfloat *vertices = (GLfloat*)*(GLfloat**)((char*)enabler->renderer+0x40);
        int x1 = gps->dimx - 31;
        float f = 0.8;
        for (GLfloat x = x1; x < gps->dimx; x++)
        {
            float f2 = (x < gps->dimx-1) ? f : 1;
          for (GLfloat y = 0; y < gps->dimy; y++)
          {
            int tile2 = x * gps->dimy + y;
            GLfloat *vertex = vertices + 6*2*tile2;
            float x2 = x1+(x-x1)*f;
            vertex[0]  = x2;   // Upper left
            vertex[1]  = y;
            vertex[2]  = x2+f2; // Upper right
            vertex[3]  = y;
            vertex[4]  = x2;   // Lower left
            vertex[5]  = y+1;
            vertex[6]  = x2;   // Lower left again (triangle 2)
            vertex[7]  = y+1;
            vertex[8]  = x2+f2; // Upper right
            vertex[9]  = y;
            vertex[10] = x2+f2; // Lower right
            vertex[11] = y+1;
          }
        }
       }



        }
*/


void hook()
{
    if (enabled)
        return;

    //TODO: check for opengl renderer, graphics, show msg otherwise

    renderer_opengl *oldr = (renderer_opengl*)enabler->renderer;
    renderer_cool *newr = new renderer_cool;

    long **vtable_old = (long **)oldr;
    long **vtable_new = (long **)newr;

    long draw_new = vtable_new[0][14];
    long update_tile_new = vtable_new[0][0];

    memcpy(vtable_new[0], vtable_old[0], sizeof(void*)*16);
    vtable_new[0][14] = draw_new;
    vtable_new[0][0] = update_tile_new;
    vtable_new[0][17] = vtable_old[0][0];

    memcpy(&newr->screen, &oldr->screen, (char*)&newr->dummy-(char*)&newr->screen);
    //free (enabler->renderer);
    enabler->renderer = newr;

    *out2 << (char*)&newr->vertexes-(char*)newr << std::endl;

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

bool get_font_paths()
{
    string small_font_path, gsmall_font_path;
    string large_font_path, glarge_font_path;

    std::ifstream fseed("data/init/init.txt");
    if(fseed.is_open())
    {
        string str;

        while(std::getline(fseed,str))
        {
            size_t b = str.find("[");
            size_t e = str.rfind("]");

            if (b == string::npos || e == string::npos || str.find_first_not_of(" ") < b)
                continue;

            str = str.substr(b+1, e-1);
            vector<string> tokens = split(str.c_str(), ':');

            if (tokens.size() != 2)
                continue;
                                
            if(tokens[0] == "FONT")
            {
                small_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "FULLFONT")
            {
                large_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "GRAPHICS_FONT")
            {
                gsmall_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "GRAPHICS_FULLFONT")
            {
                glarge_font_path = "data/art/" + tokens[1];
                continue;
            }                    
        }
    }

    fseed.close();
    
    if (!(small_font_path == gsmall_font_path && large_font_path == glarge_font_path))
    {
        struct tileset ts;
        ts.small_font_path = small_font_path;
        ts.large_font_path = large_font_path;

        tilesets.push_back(ts);
        return true;
    }
    else
    {
        struct tileset ts;
        tilesets.push_back(ts);
        return false;
    }
}

bool load_overrides()
{
    bool found = false;

    std::ifstream fseed("data/init/overrides.txt");
    if(fseed.is_open())
    {
        string str;

        while(std::getline(fseed,str))
        {
            size_t b = str.find("[");
            size_t e = str.rfind("]");

            if (b == string::npos || e == string::npos || str.find_first_not_of(" ") < b)
                continue;

            str = str.substr(b+1, e-1);
            vector<string> tokens = split(str.c_str(), ':');

            if (tokens[0] == "TILESET")
            {
                struct tileset ts;
                ts.small_font_path = "data/art/" + tokens[1];
                ts.large_font_path = "data/art/" + tokens[2];
                tilesets.push_back(ts);
                continue;
            }
            
            if (tokens[0] == "OVERRIDE")
            {
                if (tokens.size() == 8)
                {
                    int tile = atoi(tokens[1].c_str());

                    struct override o;
                    o.building = (tokens[2] == "B");
                    if (o.building)
                    {
                        buildings_other_id::buildings_other_id id;
                        if (find_enum_item(&id, tokens[3]))
                            o.id = id;
                        else
                            o.id = -1;

                        building_type::building_type type;
                        if (find_enum_item(&type, tokens[4]))
                            o.type = type;
                        else
                            o.type = -1;
                    }
                    else
                    {
                        items_other_id::items_other_id id;
                        if (find_enum_item(&id, tokens[3]))
                            o.id = id;
                        else
                            o.id = -1;

                        item_type::item_type type;
                        if (find_enum_item(&type, tokens[4]))
                            o.type = type;
                        else
                            o.type = -1;
                    }

                    if (tokens[5].length() > 0)
                        o.subtype = atoi(tokens[5].c_str());
                    else
                        o.subtype = -1;

                    o.newtile.tilesetidx = atoi(tokens[6].c_str());
                    o.newtile.tile = atoi(tokens[7].c_str());

                    if (!overrides[tile])
                        overrides[tile] = new vector< struct override >;
                    overrides[tile]->push_back(o);
                }
                else if (tokens.size() == 4)
                {
                    int tile = atoi(tokens[1].c_str());
                    override_defs[tile].tilesetidx = atoi(tokens[2].c_str());
                    override_defs[tile].tile = atoi(tokens[3].c_str());
                }

                found = true;
                continue;
            }
        }
    }

    fseed.close();
    return found;
}

unsigned char screen2[100*100*4];
    int32_t screentexpos2[100*100];
    int8_t screentexpos_addcolor2[100*100];
    uint8_t screentexpos_grayscale2[100*100];
    uint8_t screentexpos_cf2[100*100];
    uint8_t screentexpos_cbr2[100*100];

#define CHECK(dx,dy) \
{\
    df::map_block *block = world->map.block_index[(xx+dx) >> 4][(yy+dy) >> 4][zz]; \
    if (block) \
        k = block->tiletype[(xx+dx)&15][(yy+dy)&15] != df::tiletype::OpenSpace;\
}



struct zzz : public df::viewscreen_dwarfmodest
{
    typedef df::viewscreen_dwarfmodest interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, render, ())
    {
        unsigned char *sctop = enabler->renderer->screen;
    int32_t* screentexpostop = enabler->renderer->screentexpos;
    int8_t* screentexpos_addcolortop = enabler->renderer->screentexpos_addcolor;
    uint8_t* screentexpos_grayscaletop = enabler->renderer->screentexpos_grayscale;
    uint8_t* screentexpos_cftop = enabler->renderer->screentexpos_cf;
    uint8_t* screentexpos_cbrtop = enabler->renderer->screentexpos_cbr;

        static int z = 0;
        //elemcnt = 0;

        INTERPOSE_NEXT(render)();
        /*if (!z)
        {
            z = 1;
        for (int i = 1; i < 10; i++)
        {
                unsigned char *olds = enabler->renderer->screen;
                gps->screen = screen2;
                gps->screen_limit = gps->screen + gps->dimx * gps->dimy * 4;
                gps->screentexpos = screentexpos2;
                gps->screentexpos_addcolor = screentexpos_addcolor2;
                gps->screentexpos_grayscale = screentexpos_grayscale2;
                gps->screentexpos_cf = screentexpos_cf2;
                gps->screentexpos_cbr = screentexpos_cbr2;
                (*df::global::window_z)-=i;
                //(*df::global::window_x)+=x0-1;
                //init->display.grid_x -= x0-1;
                INTERPOSE_NEXT(render)();
                //(*df::global::window_x)-=x0-1;
                (*df::global::window_z)+=i;
                //init->display.grid_x += x0-1;
                gps->screen = olds;
                gps->screen_limit = gps->screen + gps->dimx * gps->dimy * 4;
                gps->screentexpos = screentexpostop;
                gps->screentexpos_addcolor = screentexpos_addcolortop;
                gps->screentexpos_grayscale = screentexpos_grayscaletop;
                gps->screentexpos_cf = screentexpos_cftop;
                gps->screentexpos_cbr = screentexpos_cbrtop;            
        }
        z = 0;
    }
        return;*/
//
        if (!z)
        {
            z = 1;


            bool u;
            int p = 1;
            int x0 = 1;
            do
            {
                if (*df::global::window_z-p < 0)
                    break;
                unsigned char *olds = enabler->renderer->screen;
                gps->screen = screen2;
                gps->screen_limit = gps->screen + gps->dimx * gps->dimy * 4;
                gps->screentexpos = screentexpos2;
                gps->screentexpos_addcolor = screentexpos_addcolor2;
                gps->screentexpos_grayscale = screentexpos_grayscale2;
                gps->screentexpos_cf = screentexpos_cf2;
                gps->screentexpos_cbr = screentexpos_cbr2;
                (*df::global::window_z)-=p;
                (*df::global::window_x)+=x0-1;
                init->display.grid_x -= x0-1;
                //*df::global::pause_state = 1;
                INTERPOSE_NEXT(render)();
                //*df::global::pause_state = 0;
                (*df::global::window_x)-=x0-1;
                (*df::global::window_z)+=p;
                init->display.grid_x += x0-1;
                gps->screen = olds;
                gps->screen_limit = gps->screen + gps->dimx * gps->dimy * 4;
                gps->screentexpos = screentexpostop;
                gps->screentexpos_addcolor = screentexpos_addcolortop;
                gps->screentexpos_grayscale = screentexpos_grayscaletop;
                gps->screentexpos_cf = screentexpos_cftop;
                gps->screentexpos_cbr = screentexpos_cbrtop;

                int zz0 = *df::global::window_z;
                u = false;
                int x00 = x0;
                //*out2 << p << " " << x0 << std::endl;
                
                GLfloat *vertices = (GLfloat*)*(GLfloat**)((char*)enabler->renderer+0x40);
                for (int x = x0; x < gps->dimx-32; x++)
                {
                    for (int y = 1; y < gps->dimy-1; y++)
                    {
                        const int tile = x * gps->dimy + y;
                        const int tile2 = (x-(x00-1)) * gps->dimy + y;

                        if ((sctop[tile*4+3]&0xf0))
                            continue;

                        unsigned char ch = sctop[tile*4+0];
                        if (ch != 249 && ch != 250 && ch != 254 && ch != 32 && ch != 0 && !(ch >= '1' && ch <= '7'))
                            continue;

                        int xx = *df::global::window_x + x-1;
                        int yy = *df::global::window_y + y-1;
                        int zz = zz0 - p+1;
                        if (xx < 0 || yy < 0)
                            continue;

                        //TODO: check for z=0
                        bool e0,h,h0;
                        //*out2 << xx << " " << world->map.x_count << " " << yy << " " << world->map.y_count << " " << *df::global::window_x << " " << *df::global::window_y << std::endl;
                        df::map_block *block0 = world->map.block_index[xx >> 4][yy >> 4][zz0];
                        h0 = block0 && block0->designation[xx&15][yy&15].bits.hidden;
                        if (h0)
                            continue;
                        e0 = !block0 || block0->tiletype[xx&15][yy&15] == df::tiletype::OpenSpace;
                        if (!(e0))
                            continue;

                        ch = screen2[tile2*4+0];
                        if (!(ch != 249 && ch != 250 && ch != 254 && ch != 32 && ch != 0 && !(ch >= '1' && ch <= '7')))
                        {
                            df::map_block *block1 = world->map.block_index[xx >> 4][yy >> 4][zz-1];
                            if (!block1)
                            {
                                //TODO: skip all other y's in this block
                                u = true;
                                continue;
                            }
                            //TODO: check for hidden also
                            df::tiletype t1 = block1->tiletype[xx&15][yy&15];
                            if (t1 == df::tiletype::OpenSpace)
                            {
                                if (ch != 249 && ch != 250 && ch != 254 && ch != 32 && ch != 0 && !(ch >= '1' && ch <= '7'))
                                    *out2 << (int)ch << std::endl;
                                u = true;
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
                        sctop[tile*4+3] = (0x10*(p)) | (sctop[tile*4+3]&0x0f);
                    }
                    if (!u)
                        x0 = x + 1;
                }
                if (p++ >= 10)
                    break;
            } while(u);

            z = 0;
        }






        /*if (sepview)
        {
            int ox = gps->dimx, oy = gps->dimy;
            gps->dimx = 30;
            gps->dimy = 25;
            init->display.grid_x = 30;
            init->display.grid_y = 25;

            sepview->resize(30,25);
            sepview->render();
            gps->dimx = ox;
            gps->dimy = oy;
            init->display.grid_x = ox;
            init->display.grid_y = oy;
        }*/
    }    
};

IMPLEMENT_VMETHOD_INTERPOSE(zzz, render);

DFhackCExport command_result plugin_init ( color_ostream &out, vector <PluginCommand> &commands)
{
    out2 = &out;
    
#ifdef WIN32
    load_multi_pdim = (void (*)(void *tex, const string &filename, long *tex_pos, long dimx,
        long dimy, bool convert_magenta, long *disp_x, long *disp_y)) (0x00a52670+(Core::getInstance().vinfo->getRebaseDelta()));    
#else
    load_multi_pdim = (void (*)(void *tex, const string &filename, long *tex_pos, long dimx,
        long dimy, bool convert_magenta, long *disp_x, long *disp_y)) 0x00cfbbb0;    
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

    return CR_OK;
}

DFhackCExport command_result plugin_shutdown ( color_ostream &out )
{
    if (enabled)
        unhook();

    return CR_OK;
}