struct renderer_legacy : renderer_opengl
{
    // To know the size of renderer_opengl's fields
    void *dummy;

    virtual void update_tile(int x, int y);
    virtual void draw(int vertex_count);
    virtual void reshape_gl();

    virtual void update_tile_old(int x, int y) {};
    virtual void reshape_gl_old(){};
};

static unsigned char depth_legacy[256*256];
static float fogcoord_legacy[256*256*6];
static GLfloat shadowtex_legacy[256*256*4*2*6];
static GLfloat shadowvert_legacy[256*256*4*2*6];

bool is_text_tile_legacy(int x, int y, bool &is_map)
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
        if (y <= 1 || y >= h - 7 || x == 0 || x >= w - 28)
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


static void screen_to_texid_legacy(df::renderer *r, int tile, struct texture_fullid &ret)
{
    const unsigned char *s = r->screen + tile*4;

    int bold = (s[3] != 0) * 8;
    int fg   = (s[1] + bold) % 16;
    int bg   = s[2] % 16;

    const long texpos = r->screentexpos[tile];

    if (!texpos)
    {
        int ch = s[0];
        ret.texpos = map_texpos[ch];//enabler->fullscreen ? init->font.large_font_texpos[ch] : init->font.small_font_texpos[ch];

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

static void write_tile_arrays_legacy(df::renderer *r, int x, int y, GLfloat *fg, GLfloat *bg, GLfloat *tex)
{
    const int tile = x * gps->dimy + y;

    struct texture_fullid ret;
    screen_to_texid_legacy(r, tile, ret);

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
    
    bool is_map;
    if (is_text_tile_legacy(x, y, is_map))
    {
        const int tile = x * gps->dimy + y;
        const unsigned char *s = r->screen + tile*4;
        ret.texpos = text_texpos[s[0]];
    }
    else if (is_map && has_overrides)
    {
        const unsigned char *s = r->screen + tile*4;
        int s0 = s[0];

        if (overrides[s0])
        {
            int xx = gwindow_x + x - 1;
            int yy = gwindow_y + y - 1;

            if (xx < world->map.x_count && yy < world->map.y_count)
            {
                if (s0 == 88 && df::global::cursor->x == xx && df::global::cursor->y == yy)
                {
                    long texpos = enabler->fullscreen ? cursor_large_texpos : cursor_small_texpos;
                    if (texpos)
                        ret.texpos = texpos;
                }
                else
                {
                    int zz = gwindow_z - ((s[3]&0xf0)>>4);

                    tile_overrides *to = overrides[s0];

                    // Items
                    for (auto it = to->item_overrides.begin(); it != to->item_overrides.end(); it++)
                    {
                        override_group &og = *it;

                        auto ilist = world->items.other[og.other_id];
                        for (auto it2 = ilist.begin(); it2 != ilist.end(); it2++)
                        {
                            df::item *item = *it2;
                            if (!(zz == item->pos.z && xx == item->pos.x && yy == item->pos.y))
                                continue;
                            if (item->flags.whole & bad_item_flags.whole)
                                continue;

                            for (auto it3 = og.overrides.begin(); it3 != og.overrides.end(); it3++)
                            {
                                override &o = *it3;

                                if (o.type != -1 && item->getType() != o.type)
                                    continue;
                                if (o.subtype != -1 && item->getSubtype() != o.subtype)
                                    continue;

                                ret.texpos = enabler->fullscreen ? o.large_texpos : o.small_texpos;
                                goto matched;
                            }
                        }
                    }

                    // Buildings
                    for (auto it = to->building_overrides.begin(); it != to->building_overrides.end(); it++)
                    {
                        override_group &og = *it;

                        auto ilist = world->buildings.other[og.other_id];
                        for (auto it2 = ilist.begin(); it2 != ilist.end(); it2++)
                        {
                            df::building *bld = *it2;
                            if (zz != bld->z || xx < bld->x1 || xx > bld->x2 || yy < bld->y1 || yy > bld->y2)
                                continue;

                            for (auto it3 = og.overrides.begin(); it3 != og.overrides.end(); it3++)
                            {
                                override &o = *it3;

                                if (o.type != -1 && bld->getType() != o.type)
                                    continue;
                                
                                if (o.subtype != -1)
                                {
                                    int subtype = (og.other_id == buildings_other_id::WORKSHOP_CUSTOM || og.other_id == buildings_other_id::FURNACE_CUSTOM) ?
                                        bld->getCustomType() : bld->getSubtype();

                                    if (subtype != o.subtype)
                                        continue;
                                }

                                ret.texpos = enabler->fullscreen ? o.large_texpos : o.small_texpos;
                                goto matched;
                            }
                        }
                    }

                    // Tile types
                    df::map_block *block = world->map.block_index[xx>>4][yy>>4][zz];
                    if (block)
                    {
                        int tiletype = block->tiletype[xx&15][yy&15];

                        for (auto it3 = to->tiletype_overrides.begin(); it3 != to->tiletype_overrides.end(); it3++)
                        {
                            override &o = *it3;

                            if (tiletype == o.type)
                            {
                                ret.texpos = enabler->fullscreen ? o.large_texpos : o.small_texpos;
                                goto matched;
                            }
                        }
                    }
                }
            }
        }
    }

    matched:;
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

void renderer_legacy::update_tile(int x, int y)
{
    if (!enabled)
    {
        this->update_tile_old(x, y);
        return;
    }

    const int tile = x*gps->dimy + y;

    GLfloat *_fg  = fg + tile * 4 * 6;
    GLfloat *_bg  = bg + tile * 4 * 6;
    GLfloat *_tex = tex + tile * 2 * 6;

    write_tile_arrays_legacy(this, x, y, _fg, _bg, _tex);

    //const int tile = x * gps->dimy + y;
    float d = (float)((screen[tile*4+3]&0xf0)>>4);
    depth_legacy[tile] = d;

    if (fogdensity > 0)
    {
        fogcoord_legacy[tile*6+0] = d;
        fogcoord_legacy[tile*6+1] = d;
        fogcoord_legacy[tile*6+2] = d;
        fogcoord_legacy[tile*6+3] = d;
        fogcoord_legacy[tile*6+4] = d;
        fogcoord_legacy[tile*6+5] = d;
    }
}

void renderer_legacy::reshape_gl()
{
    reshape_gl_old();

    static int last_fullscreen = -1;
    if (last_fullscreen != enabler->fullscreen)
    {
        last_fullscreen = enabler->fullscreen;
        if (last_fullscreen)
        {
            map_texpos = tilesets[0].large_texpos;
            text_texpos = tilesets[1].large_texpos;
        }
        else
        {
            map_texpos = tilesets[0].small_texpos;
            text_texpos = tilesets[1].small_texpos;
        }
    }

    //glShadeModel(GL_FLAT);    
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

void renderer_legacy::draw(int vertex_count)
{
#ifdef WIN32
    // We can't do this in plugin_init() because OpenGL context isn't initialized by that time
    static bool glew_init = false;
    if (!glew_init)
    {
        GLenum err = glewInit();
        if (err != GLEW_OK)
            *out2 << glewGetErrorString(err);
        glew_init = true;
    }
#endif

    static int old_dimx, old_dimy, old_winx, old_winy;
    if (domapshot)
    {
        if (domapshot == 10)
        {
            old_dimx = gps->dimx;
            old_dimy = gps->dimy;
            old_winx = *df::global::window_x;
            old_winy = *df::global::window_y;

            uint8_t menu_width, area_map_width;
            Gui::getMenuWidth(menu_width, area_map_width);
            int32_t menu_w = 0;

            bool menuforced = (ui->main.mode != df::ui_sidebar_mode::Default || df::global::cursor->x != -30000);

            if ((menuforced || menu_width == 1) && area_map_width == 2) // Menu + area map
            {
                menu_w = 55;
            }
            else if (menu_width == 2 && area_map_width == 2) // Area map only
            {
                menu_w = 24;
            }
            else if (menu_width == 1) // Wide menu
                menu_w = 55;
            else if (menuforced || (menu_width == 2 && area_map_width == 3)) // Menu only
                menu_w = 31; 


            grid_resize(world->map.x_count+menu_w+2, world->map.y_count+2);
            *df::global::window_x = 0;
            *df::global::window_y = 0;
            gps->force_full_display_count = 1;
        }
        domapshot--;
    }

GLuint framebuffer, renderbuffer;
GLenum status;
    if (domapshot==5)
    {
// Set the width and height appropriately for your image
        GLuint imageWidth = gps->dimx*dispx,
               imageHeight = gps->dimy*dispy;
        //Set up a FBO with one renderbuffer attachment
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glGenRenderbuffers(1, &renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, imageWidth, imageHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                 GL_RENDERBUFFER, renderbuffer);        
        glViewport(0,0,gps->dimx*dispx,gps->dimy*dispy);

        /*glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glOrtho(0,gps->dimx,gps->dimy,0,-1,1);*/
    }

    if (fogdensity)
    {
        glEnable(GL_FOG);
        glFogfv(GL_FOG_COLOR, fogcolor);
        glFogf(GL_FOG_DENSITY, fogdensity);
        glFogi(GL_FOG_COORD_SRC, GL_FOG_COORD);
        glFogCoordPointer(GL_FLOAT, 0, fogcoord_legacy);
        glEnableClientState(GL_FOG_COORD_ARRAY);
    }

    glVertexPointer(2, GL_FLOAT, 0, vertexes);

    // Render background colors
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_BLEND);
    glColorPointer(4, GL_FLOAT, 0, bg);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    // Render foreground
    //glAlphaFunc(GL_NOTEQUAL, 0);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexCoordPointer(2, GL_FLOAT, 0, tex);
    glColorPointer(4, GL_FLOAT, 0, fg);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    if (fogdensity > 0)
    {
        glDisableClientState(GL_FOG_COORD_ARRAY);
        glDisable(GL_FOG);    
    }

    if (maxlevels)
    {
        // Prepare and render shadows
        short elemcnt = 0;
        //TODO: don't do this if view not moved and tiles with shadows not changed
        static df::viewscreen *prevws = NULL;
        df::viewscreen *ws = Gui::getCurViewscreen();
        if (ws != prevws)
        {
            gps->force_full_display_count = true;
            prevws = ws;
        }
        if (df::viewscreen_dwarfmodest::_identity.is_direct_instance(ws))
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

            gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;

            for (int tile = 0; tile < gps->dimx*gps->dimy; tile++)
            {
                int xx = tile / gps->dimy;
                int yy = tile % gps->dimy;

                int d = depth_legacy[tile];
                if (d)
                {
                    GLfloat *tex = shadowtex_legacy+elemcnt*2;

                    bool top=false, left=false, btm=false, right=false;
                    if (xx > 1 && (depth_legacy[((xx-1)*gps->dimy + yy)]) < d)
                    {
                        memcpy(shadowvert_legacy+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                        SETTEX(shadow_texpos[0]);
                        elemcnt+=6;
                        left = true;
                    }
                    if (yy < h-2 && (depth_legacy[((xx)*gps->dimy + yy+1)]) < d)
                    {
                        memcpy(shadowvert_legacy+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                        SETTEX(shadow_texpos[1]);
                        elemcnt+=6;
                        btm = true;
                    }
                    if (yy > 1 && (depth_legacy[((xx)*gps->dimy + yy-1)]) < d)
                    {
                        memcpy(shadowvert_legacy+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                        SETTEX(shadow_texpos[2]);
                        elemcnt+=6;
                        top = true;
                    }
                    if (xx < menu_left-1 && (depth_legacy[((xx+1)*gps->dimy + yy)]) < d)
                    {
                        memcpy(shadowvert_legacy+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                        SETTEX(shadow_texpos[3]);
                        elemcnt+=6;
                        right = true;
                    }

                    if (!right && !btm && xx < menu_left-1 && yy < h-2 && (depth_legacy[((xx+1)*gps->dimy + yy+1)]) < d)
                    {
                        memcpy(shadowvert_legacy+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                        SETTEX(shadow_texpos[4]);
                        elemcnt+=6;
                    }
                    if (!left && !btm && xx > 1 && yy < h-2 && (depth_legacy[((xx-1)*gps->dimy + yy+1)]) < d)
                    {
                        memcpy(shadowvert_legacy+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                        SETTEX(shadow_texpos[5]);
                        elemcnt+=6;
                    }
                    if (!left && !top && xx > 1 && yy > 1 && (depth_legacy[((xx-1)*gps->dimy + yy-1)]) < d)
                    {
                        memcpy(shadowvert_legacy+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                        SETTEX(shadow_texpos[6]);
                        elemcnt+=6;
                    }
                    if (!top && !right && xx < menu_left-1 && yy > 1 && (depth_legacy[((xx+1)*gps->dimy + yy-1)]) < d)
                    {
                        memcpy(shadowvert_legacy+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                        SETTEX(shadow_texpos[7]);
                        elemcnt+=6;
                    }
                }            
            }
        }

        if (elemcnt)
        {
            glDisableClientState(GL_COLOR_ARRAY);
            glColor4fv(shadowcolor);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            //glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
            glTexCoordPointer(2, GL_FLOAT, 0, shadowtex_legacy);
            glVertexPointer(2, GL_FLOAT, 0, shadowvert_legacy);
            glDrawArrays(GL_TRIANGLES, 0, elemcnt);
            glEnableClientState(GL_COLOR_ARRAY);    
        }
    }

    if (domapshot==1)
    {
        int w = world->map.x_count*dispx;
        int h = world->map.y_count*dispy;

        unsigned char *data = (unsigned char*) malloc(w*h*3);
        
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glReadPixels(dispx,dispy,w,h,GL_BGR,GL_UNSIGNED_BYTE,data);


#pragma pack(push,1)
typedef struct _TgaHeader
{
  unsigned char IDLength;        /* 00h  Size of Image ID field */
  unsigned char ColorMapType;    /* 01h  Color map type */
  unsigned char ImageType;       /* 02h  Image type code */
  unsigned short CMapStart;       /* 03h  Color map origin */
  unsigned short CMapLength;      /* 05h  Color map length */
  unsigned char CMapDepth;       /* 07h  Depth of color map entries */
  unsigned short XOffset;         /* 08h  X origin of image */
  unsigned short YOffset;         /* 0Ah  Y origin of image */
  unsigned short Width;           /* 0Ch  Width of image */
  unsigned short Height;          /* 0Eh  Height of image */
  unsigned char PixelDepth;      /* 10h  Image pixel size */
  unsigned char ImageDescriptor; /* 11h  Image descriptor byte */
} TGAHEAD;
#pragma pop

TGAHEAD hdr;
memset(&hdr, 0, sizeof(hdr));
hdr.ImageType = 2;
hdr.Width = w;
hdr.Height = h;
hdr.PixelDepth = 24;


*out2 << w << " "<<h<<std::endl;
        std::ofstream img("mapshot.tga", std::ofstream::binary);
        img.write((const char*)&hdr, sizeof(hdr));
/*        for (int j = 0; j<w*h*3; j++)
        {
            unsigned char c = data[j+0];
            data[0] = data[j+2];
            data[j+2] = c;
        }*/
        img.write((const char*)data, w*h*3);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
// Delete the renderbuffer attachment
glDeleteRenderbuffers(1, &renderbuffer);

        grid_resize(old_dimx, old_dimy);
        *df::global::window_x = old_winx;
        *df::global::window_y = old_winy;
        gps->force_full_display_count = 1;
        domapshot = 0;        
    }
}
