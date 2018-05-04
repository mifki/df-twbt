static volatile int domapshot = 0;

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

static void write_tile_vertexes_oblique(GLfloat x, GLfloat y, GLfloat *vertex, float d)
{
    vertex[0]  = x;   // Upper left
    vertex[1]  = y/2.5 + d*0.1;
    vertex[2]  = x + 1; // Upper right
    vertex[3]  = y/2.5 + d*0.1;
    vertex[4]  = x;   // Lower left
    vertex[5]  = y/2.5 + d*0.1 + 1;
    vertex[6]  = x;   // Lower left again (triangle 2)
    vertex[7]  = y/2.5 + d*0.1 + 1;
    vertex[8]  = x + 1; // Upper right
    vertex[9]  = y/2.5 + d*0.1;
    vertex[10] = x + 1; // Lower right
    vertex[11] = y/2.5 + d*0.1 + 1;
}

static void write_tile_vertexes(GLfloat x, GLfloat y, GLfloat *vertex, float d)
{
    vertex[0]  = x;   // Upper left
    vertex[1]  = y;
    vertex[2]  = x + 1; // Upper right
    vertex[3]  = y;
    vertex[4]  = x;   // Lower left
    vertex[5]  = y + 1;
    vertex[6]  = x;   // Lower left again (triangle 2)
    vertex[7]  = y + 1;
    vertex[8]  = x + 1; // Upper right
    vertex[9]  = y;
    vertex[10] = x + 1; // Lower right
    vertex[11] = y + 1;
}

renderer_cool::renderer_cool()
{
    dummy = 'TWBT';
    gvertexes = 0, gfg = 0, gtex = 0;
    gdimx = 0, gdimy = 0, gdimxfull = 0, gdimyfull = 0;
    gdispx = 0, gdispy = 0;
    goff_x = 0, goff_y = 0, gsize_x = 0, gsize_y = 0;
    needs_reshape = needs_zoom = 0;
}

void renderer_cool::update_all()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    for (int x = 0; x < tdimx; ++x)
        for (int y = 0; y < tdimy; ++y)
            update_tile(x, y);    
}

void renderer_cool::update_tile(int x, int y)
{
    if (!enabled)
    {
        this->update_tile_old(x, y);
        return;
    }

    //XXX: sometimes this might be called while gps->dimx/y are set to map dim > text dim, and this will crash
    //XXX: better not to use NO_DISPLAY_PATCH, but if we must, let's check x/y here
#ifdef NO_DISPLAY_PATCH
    if (x >= tdimx || y >= tdimy)
        return;
#endif

    const int tile = x * tdimy + y;

    GLfloat *_fg  = fg + tile * 4 * 6;
    GLfloat *_bg  = bg + tile * 4 * 6;
    GLfloat *_tex = tex + tile * 2 * 6;

    write_tile_arrays_text(this, x, y, _fg, _bg, _tex);
}

void renderer_cool::update_map_tile(int x, int y)
{
    const int tile = x * gdimy + y;

    GLfloat *_bg_under      = gfg  + (tile*6+0) * 4 * 6;
    GLfloat *_tex_bg_under  = gtex + (tile*6+0) * 2 * 6;
    GLfloat *_fg_under      = gfg  + (tile*6+1) * 4 * 6;
    GLfloat *_tex_under     = gtex + (tile*6+1) * 2 * 6;
    GLfloat *_fg_top_under  = gfg  + (tile*6+2) * 4 * 6;
    GLfloat *_tex_top_under = gtex + (tile*6+2) * 2 * 6;

    GLfloat *_bg            = gfg  + (tile*6+3) * 4 * 6;
    GLfloat *_tex_bg        = gtex + (tile*6+3) * 2 * 6;
    GLfloat *_fg            = gfg  + (tile*6+4) * 4 * 6;
    GLfloat *_tex           = gtex + (tile*6+4) * 2 * 6;
    GLfloat *_fg_top        = gfg  + (tile*6+5) * 4 * 6;
    GLfloat *_tex_top       = gtex + (tile*6+5) * 2 * 6;

    write_tile_arrays_under(this, x, y, _fg_under, _bg_under, _tex_under, _tex_bg_under, _fg_top_under, _tex_top_under);
    write_tile_arrays_map(this, x, y, _fg, _bg, _tex, _tex_bg, _fg_top, _tex_top);

    if (maxlevels)
    {
        float d = (float)((gscreen[tile * 4 + 3] & 0xf0) >> 4);

        // for oblique
        //write_tile_vertexes(x, y, gvertexes + 6 * 2 * tile, d);

        depth[tile] = !gscreen[tile*4] ? 0x7f : d; //TODO: no need for this in fort mode

        if (fogdensity > 0)
        {
            if (d > 0)
                d = d*fogstep + fogstart;

            for (int j = 0; j < 6*6; j++)
                fogcoord[tile * 6 * 6 + j] = d;
        }
    }
}

void renderer_cool::reshape_graphics()
{
    float tsx = (float)size_x / gps->dimx, tsy = (float)size_y / gps->dimy;

    int32_t w = gps->dimx, h = gps->dimy;

    int cx = *df::global::window_x + gdimx / 2;
    int cy = *df::global::window_y + gdimy / 2;

    df::viewscreen *ws = df::global::gview->view.child;
    if (df::viewscreen_dwarfmodest::_identity.is_direct_instance(ws))
    {
        gsize_x = (size_x - tsx * (gmenu_w + 1 + 1));
        gsize_y = (size_y - tsy * 2);
        goff_x = off_x + roundf(tsx);
        goff_y = off_y + roundf(tsy);        
    }
    else //Adv. mode
    {
        // *out2 << "reshape_graphics" << std::endl;
        gsize_x = size_x;
        gsize_y = size_y;
        goff_x = off_x;
        goff_y = goff_y_gl = off_y;
    }

    float _dimx = std::min(gsize_x / gdispx, 256.0f);
    float _dimy = std::min(gsize_y / gdispy, 256.0f); //*3 for oblique
    gdimx = ceilf(_dimx);
    gdimy = ceilf(_dimy);
    gdimxfull = floorf(_dimx);
    gdimyfull = floorf(_dimy);

    if (df::viewscreen_dwarfmodest::_identity.is_direct_instance(ws))
        goff_y_gl = goff_y - (gdimy == gdimyfull ? 0 : roundf(gdispy - (gsize_y - gdispy * gdimyfull)));                

    *df::global::window_x = std::max(0, cx - gdimx / 2);
    *df::global::window_y = std::max(0, cy - gdimy / 2);

    int tiles = gdimx * gdimy;

    // Recreate tile buffers
    allocate_buffers(tiles, gdimy + 1);

    // Recreate OpenGL buffers
    gvertexes = (GLfloat*)realloc(gvertexes, sizeof(GLfloat) * tiles * 2 * 6 * 6);
    gfg = (GLfloat*)realloc(gfg, sizeof(GLfloat) * tiles * 4 * 6 * 6);
    gtex = (GLfloat*)realloc(gtex, sizeof(GLfloat) * tiles * 2 * 6 * 6);

    // Initialise vertex coords
    int tile = 0;   
    for (GLfloat x = 0; x < gdimx; x++)
    {
        for (GLfloat y = 0; y < gdimy; y++, tile++)
        {
            write_tile_vertexes(x, y, gvertexes + 6 * 2 * (tile*6+0), 0);
            write_tile_vertexes(x, y, gvertexes + 6 * 2 * (tile*6+1), 0);
            write_tile_vertexes(x, y, gvertexes + 6 * 2 * (tile*6+2), 0);
            write_tile_vertexes(x, y, gvertexes + 6 * 2 * (tile*6+3), 0);
            write_tile_vertexes(x, y, gvertexes + 6 * 2 * (tile*6+4), 0);
            write_tile_vertexes(x, y, gvertexes + 6 * 2 * (tile*6+5), 0);
        }
    }

    needs_full_update = true;
}

void renderer_cool::reshape_gl()
{
    reshape_gl_old();

    tdimx = gps->dimx;
    tdimy = gps->dimy;

    static int last_fullscreen = -1;
    if (last_fullscreen != enabler->fullscreen)
    {
        last_fullscreen = enabler->fullscreen;
        map_texpos = tilesets[0].small_texpos;
        text_texpos = tilesets[1].small_texpos;

        if (!gdispx)
            gdispx = small_map_dispx, gdispy = small_map_dispy;
    }

    reshape_graphics();

    glShadeModel(GL_FLAT);    
}

void renderer_cool::draw(int vertex_count)
{
    static bool initial_resize = false;
    if (!initial_resize)
    {
        if (enabler->fullscreen)
            resize(size_x, size_y);
        else
            resize((size_x/init->font.small_font_dispx)*init->font.small_font_dispx, (size_y/init->font.small_font_dispy)*init->font.small_font_dispy);
            //resize(gps->dimx*init->font.small_font_dispx, gps->dimy*init->font.small_font_dispy);

        reshape_gl();
        initial_resize = true;
    }

    // Only for fortress mode because adventure mode map is updated in render()
    display_new(screen_map_type == 1);

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

            grid_resize(world->map.x_count + 36, world->map.y_count + 2);
            *df::global::window_x = 0;
            *df::global::window_y = 0;
            gps->force_full_display_count = 1;
        }
        domapshot--;
    }

    GLuint framebuffer, renderbuffer;
    GLenum status;
    if (domapshot == 5)
    {
        // Set the width and height appropriately for your image
        GLuint imageWidth = gps->dimx * dispx,
               imageHeight = gps->dimy * dispy;
        //Set up a FBO with one renderbuffer attachment
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glGenRenderbuffers(1, &renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, imageWidth, imageHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, renderbuffer);
        glViewport(0, 0, gps->dimx * dispx, gps->dimy * dispy);

        /*glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glOrtho(0,gps->dimx,gps->dimy,0,-1,1);*/
    }

    if (screen_map_type)
    {
        bool skip = false;
        if (df::viewscreen_dungeonmodest::_identity.is_direct_instance(Gui::getCurViewscreen()))
        {
            int m = df::global::ui_advmode->menu;
            bool tmode = advmode_needs_map(m);
            skip = !tmode;
        }

        if (!skip)
        {
            /////
            glViewport(goff_x, goff_y_gl, gdimx * gdispx, gdimy * gdispy);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, gdimx, gdimy, 0, -1, 1);
            //glTranslatef(1,-1,0);

            //glScissor(off_x+(float)size_x/gps->dimx, off_y+(float)size_y/gps->dimy, gsize_x, gsize_y);
            //glEnable(GL_SCISSOR_TEST);
            //glClearColor(1,0,0,1);
            //glClear(GL_COLOR_BUFFER_BIT);

            glClearColor(enabler->ccolor[0][0], enabler->ccolor[0][1], enabler->ccolor[0][2], 1);
            glClear(GL_COLOR_BUFFER_BIT);            

            if (multi_rendered && fogdensity > 0)
            {
                glEnable(GL_FOG);
                glFogfv(GL_FOG_COLOR, fogcolor);
                glFogf(GL_FOG_DENSITY, fogdensity);
                glFogi(GL_FOG_COORD_SRC, GL_FOG_COORD);
                glEnableClientState(GL_FOG_COORD_ARRAY);
                glFogCoordPointer(GL_FLOAT, 0, fogcoord);
            }

            glVertexPointer(2, GL_FLOAT, 0, gvertexes);

            // Render map tiles
            glEnable(GL_TEXTURE_2D);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glTexCoordPointer(2, GL_FLOAT, 0, gtex);
            glColorPointer(4, GL_FLOAT, 0, gfg);
            glDrawArrays(GL_TRIANGLES, 0, gdimx * gdimy * 6 * 6);

            if (multi_rendered)
            {
                glDisableClientState(GL_FOG_COORD_ARRAY);
                glDisable(GL_FOG);
            }

            // Prepare and render shadows
            if (multi_rendered && shadowsloaded)
            {
                int elemcnt = 0;
                //TODO: don't do this if view not moved and tiles with shadows not changed
                {
                    gl_texpos *txt = (gl_texpos *) enabler->textures.gl_texpos;
                    int x1 = std::min(gdimx, world->map.x_count-gwindow_x);                
                    int y1 = std::min(gdimy, world->map.y_count-gwindow_y);

                    for (int tile = 0; tile < gdimx * gdimy; tile++)
                    {
                        int xx = tile / gdimy;
                        int yy = tile % gdimy;

                        int d = depth[tile];
                        if (d && d != 0x7f) //TODO: no need for the second check in fort mode
                        {
                            GLfloat *tex = shadowtex + elemcnt * 2;

                            bool top = false, left = false, btm = false, right = false;
                            if (xx > 0 && (depth[((xx - 1)*gdimy + yy)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile*6 * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[0]);
                                elemcnt += 6;
                                left = true;
                            }
                            if (yy < y1 - 1 && (depth[((xx)*gdimy + yy + 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile*6 * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[1]);
                                elemcnt += 6;
                                btm = true;
                            }
                            if (yy > 0 && (depth[((xx)*gdimy + yy - 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile*6 * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[2]);
                                elemcnt += 6;
                                top = true;
                            }
                            if (xx < x1-1 && (depth[((xx + 1)*gdimy + yy)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile*6 * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[3]);
                                elemcnt += 6;
                                right = true;
                            }

                            if (!right && !btm && xx < x1 - 1 && yy < y1 - 1 && (depth[((xx + 1)*gdimy + yy + 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile*6 * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[4]);
                                elemcnt += 6;
                            }
                            if (!left && !btm && xx > 0 && yy < y1 - 1 && (depth[((xx - 1)*gdimy + yy + 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile*6 * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[5]);
                                elemcnt += 6;
                            }
                            if (!left && !top && xx > 0 && yy > 0 && (depth[((xx - 1)*gdimy + yy - 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile*6 * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[6]);
                                elemcnt += 6;
                            }
                            if (!top && !right && xx < x1 - 1 && yy > 0 && (depth[((xx + 1)*gdimy + yy - 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile*6 * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[7]);
                                elemcnt += 6;
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
                    glTexCoordPointer(2, GL_FLOAT, 0, shadowtex);
                    glVertexPointer(2, GL_FLOAT, 0, shadowvert);
                    glDrawArrays(GL_TRIANGLES, 0, elemcnt);
                    glEnableClientState(GL_COLOR_ARRAY);
                }
            }

            glDisable(GL_SCISSOR_TEST);

            glViewport(off_x, off_y, size_x, size_y);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, tdimx, tdimy, 0, -1, 1);            
        }
    }
    {
        glVertexPointer(2, GL_FLOAT, 0, vertexes);

        // Render background colors
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnable(GL_BLEND);
        glColorPointer(4, GL_FLOAT, 0, bg);
        glDrawArrays(GL_TRIANGLES, 0, tdimx*tdimy*6);

        // Render foreground
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexCoordPointer(2, GL_FLOAT, 0, tex);
        glColorPointer(4, GL_FLOAT, 0, fg);
        glDrawArrays(GL_TRIANGLES, 0, tdimx*tdimy*6);
    }


    if (domapshot == 1)
    {
        int w = world->map.x_count * dispx;
        int h = world->map.y_count * dispy;

        unsigned char *data = (unsigned char *) malloc(w * h * 3);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glReadPixels(dispx, dispy, w, h, GL_BGR, GL_UNSIGNED_BYTE, data);


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

        *out2 << w << " " << h << std::endl;
        std::ofstream img("mapshot.tga", std::ofstream::binary);
        img.write((const char *)&hdr, sizeof(hdr));
        /*        for (int j = 0; j<w*h*3; j++)
                {
                    unsigned char c = data[j+0];
                    data[0] = data[j+2];
                    data[j+2] = c;
                }*/
        img.write((const char *)data, w * h * 3);

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

void renderer_cool::display_new(bool update_graphics)
{
#ifndef NO_DISPLAY_PATCH
        // In this case this function replaces original (non-virtual) renderer::display()
        // So update text tiles here

    if (gps->force_full_display_count)
    {
        update_all();
    }
    else
    {
        uint32_t *screenp = (uint32_t*)screen, *oldp = (uint32_t*)screen_old;
        for (int x2=0; x2 < tdimx; ++x2)
        {
            for (int y2=0; y2 < tdimy; ++y2, ++screenp, ++oldp)
            {
                if (*screenp != *oldp)
                    update_tile(x2, y2);
            }
        }
    }

    if (gps->force_full_display_count > 0) gps->force_full_display_count--;
#endif

    // Update map tiles if current screen has a map
    if (update_graphics)
        display_map();
} 

void renderer_cool::display_map()
{
    //trash the previous map cache, where materials, etc, are stored.
    if (map_cache)
    {
        map_cache->trash();

        //If the map size has changed, get rid of it entirely.
        unsigned int x_bmax, y_bmax, z_max;
        Maps::getSize(x_bmax, y_bmax, z_max);
        if (map_cache->maxBlockX() != x_bmax || map_cache->maxBlockY() != y_bmax || map_cache->maxZ() != z_max)
            delete(map_cache);
    }

    if (needs_full_update || always_full_update)
    {
        needs_full_update = false;

        //clock_t c1 = clock();
        for (int x2 = 0; x2 < gdimx; x2++)
            for (int y2 = 0; y2 < gdimy; y2++)
                update_map_tile(x2, y2);
        //clock_t c2 = clock();
        //*out2 << (c2-c1) << std::endl;
    }
    else
    {
        uint32_t *gscreenp = (uint32_t*)gscreen, *goldp = (uint32_t*)gscreen_old;
        int off = 0;
        for (int x2=0; x2 < gdimx; x2++) {
            for (int y2=0; y2 < gdimy; y2++, ++off, ++gscreenp, ++goldp) {
                // We don't use pointers for the non-screen arrays because we mostly fail at the
                // *first* comparison, and having pointers for the others would exceed register
                // count.
                if (*gscreenp == *goldp &&
                gscreentexpos[off] == gscreentexpos_old[off] &&
                gscreentexpos_addcolor[off] == gscreentexpos_addcolor_old[off] &&
                gscreentexpos_grayscale[off] == gscreentexpos_grayscale_old[off] &&
                gscreentexpos_cf[off] == gscreentexpos_cf_old[off] &&
                gscreentexpos_cbr[off] == gscreentexpos_cbr_old[off])
                    ;
                else
                    update_map_tile(x2, y2);
            }
        }
    }
}

void renderer_cool::gswap_arrays()
{
    static int j = 0;

    this->gscreen = ::gscreen = _gscreen[j];
    this->gscreentexpos = ::gscreentexpos = _gscreentexpos[j];
    gscreentexpos_addcolor = _gscreentexpos_addcolor[j];
    gscreentexpos_grayscale = _gscreentexpos_grayscale[j];
    gscreentexpos_cf = _gscreentexpos_cf[j];
    gscreentexpos_cbr = _gscreentexpos_cbr[j];

    j ^= 1;

    gscreen_old = _gscreen[j];
    gscreentexpos_old = _gscreentexpos[j];
    gscreentexpos_addcolor_old = _gscreentexpos_addcolor[j];
    gscreentexpos_grayscale_old = _gscreentexpos_grayscale[j];
    gscreentexpos_cf_old = _gscreentexpos_cf[j];
    gscreentexpos_cbr_old = _gscreentexpos_cbr[j];
}

void renderer_cool::allocate_buffers(int tiles, int extra_tiles)
{
#define REALLOC(var,type,count) var = (type*)realloc(var, (count) * sizeof(type));

    REALLOC(gscreen_origin,                 uint8_t, (tiles+extra_tiles) * 4 * 2)
    REALLOC(gscreentexpos_origin,           long,    (tiles+extra_tiles) * 2);
    REALLOC(gscreentexpos_addcolor_origin,  int8_t,  (tiles+extra_tiles) * 2);
    REALLOC(gscreentexpos_grayscale_origin, uint8_t, (tiles+extra_tiles) * 2);
    REALLOC(gscreentexpos_cf_origin,        uint8_t, (tiles+extra_tiles) * 2);
    REALLOC(gscreentexpos_cbr_origin,       uint8_t, (tiles+extra_tiles) * 2);

    REALLOC(gscreen_under,                  uint8_t, tiles * 4);
    REALLOC(mscreen_under,                  uint8_t, tiles * 4);

    _gscreen[0]                 = gscreen_origin                 + extra_tiles * 4;
    _gscreentexpos[0]           = gscreentexpos_origin           + extra_tiles;
    _gscreentexpos_addcolor[0]  = gscreentexpos_addcolor_origin  + extra_tiles;
    _gscreentexpos_grayscale[0] = gscreentexpos_grayscale_origin + extra_tiles;
    _gscreentexpos_cf[0]        = gscreentexpos_cf_origin        + extra_tiles;
    _gscreentexpos_cbr[0]       = gscreentexpos_cbr_origin       + extra_tiles;

    _gscreen[1]                 = gscreen_origin                 + (extra_tiles * 2 + tiles) * 4;
    _gscreentexpos[1]           = gscreentexpos_origin           + extra_tiles * 2 + tiles;
    _gscreentexpos_addcolor[1]  = gscreentexpos_addcolor_origin  + extra_tiles * 2 + tiles;
    _gscreentexpos_grayscale[1] = gscreentexpos_grayscale_origin + extra_tiles * 2 + tiles;
    _gscreentexpos_cf[1]        = gscreentexpos_cf_origin        + extra_tiles * 2 + tiles;
    _gscreentexpos_cbr[1]       = gscreentexpos_cbr_origin       + extra_tiles * 2 + tiles;

    gswap_arrays();

    //TODO: don't allocate arrays below if multilevel rendering is not enabled
    //TODO: calculate maximum possible number of shadows
    REALLOC(depth,      int8_t,  tiles)
    REALLOC(shadowtex,  GLfloat, tiles * 4 * 2 * 6)
    REALLOC(shadowvert, GLfloat, tiles * 4 * 2 * 6)
    REALLOC(fogcoord,   GLfloat, tiles * 6 * 6)

    REALLOC(mscreen_origin,                 uint8_t, (tiles+extra_tiles) * 4)
    REALLOC(mscreentexpos_origin,           long,    tiles+extra_tiles);
    REALLOC(mscreentexpos_addcolor_origin,  int8_t,  tiles+extra_tiles);
    REALLOC(mscreentexpos_grayscale_origin, uint8_t, tiles+extra_tiles);
    REALLOC(mscreentexpos_cf_origin,        uint8_t, tiles+extra_tiles);
    REALLOC(mscreentexpos_cbr_origin,       uint8_t, tiles+extra_tiles);

    mscreen                 = mscreen_origin                     + extra_tiles * 4;
    mscreentexpos           = mscreentexpos_origin               + extra_tiles;
    mscreentexpos_addcolor  = mscreentexpos_addcolor_origin      + extra_tiles;
    mscreentexpos_grayscale = mscreentexpos_grayscale_origin     + extra_tiles;
    mscreentexpos_cf        = mscreentexpos_cf_origin            + extra_tiles;
    mscreentexpos_cbr       = mscreentexpos_cbr_origin           + extra_tiles;

    // We need to zero out these buffers because game doesn't change them for tiles without creatures,
    // so there will be garbage that will cause every tile to be updated each frame and other bad things
    memset(_gscreen[0],                 0, (tiles * 2 + extra_tiles) * 4);
    memset(_gscreentexpos[0],           0, (tiles * 2 + extra_tiles) * sizeof(long));
    memset(_gscreentexpos_addcolor[0],  0, tiles * 2 + extra_tiles);
    memset(_gscreentexpos_grayscale[0], 0, tiles * 2 + extra_tiles);
    memset(_gscreentexpos_cf[0],        0, tiles * 2 + extra_tiles);
    memset(_gscreentexpos_cbr[0],       0, tiles * 2 + extra_tiles);
    memset(mscreentexpos,               0, tiles * sizeof(long));
}

void renderer_cool::reshape_zoom_swap()
{
    static int game_mode = 3;
    if (game_mode != *df::global::gamemode)
    {
        needs_reshape = true;
        game_mode = *df::global::gamemode;
    }

    if (needs_reshape)
    {
        if (needs_zoom)
        {
            if (needs_zoom > 0)
            {
                gdispx += needs_zoom;
                gdispy += needs_zoom;
                //gdispy = gdispx*2.5; // for oblique
            }
            else if (gdispx > 1 && gdispy > 1 && (gdimxfull < world->map.x_count || gdimyfull < world->map.y_count))
            {
                gdispx += needs_zoom;
                gdispy += needs_zoom;
                //gdispy = gdispx*2.5; // for oblique
            }
        
            needs_zoom = 0;
        }
        
        needs_reshape = false;
        reshape_graphics();
        gps->force_full_display_count = 1;
        DFHack::Gui::getCurViewscreen()->resize(gps->dimx, gps->dimy);
    }
    else
        gswap_arrays();
}

void renderer_cool::zoom(df::zoom_commands cmd)
{
    if (!screen_map_type)
    {
        zoom_old(cmd);
        return;
    }

    if (cmd == df::zoom_commands::zoom_in)
    {
        gdispx++;
        gdispy++;
        //gdispy = gdispx*2.5; // for oblique
    }
    else if (cmd == df::zoom_commands::zoom_out)
    {
        if (gdispx > 1 && gdispy > 1 && (gdimxfull < world->map.x_count || gdimyfull < world->map.y_count))
        {
            gdispx--;
            gdispy--;
            //gdispy = gdispx*2.5; // for oblique
        }
    }
    else if (cmd == df::zoom_commands::zoom_reset)
    {
        gdispx = small_map_dispx;
        gdispy = small_map_dispy;
    }
    else
    {
        zoom_old(cmd);
        return;
    }

    needs_reshape = true;
}

extern "C" {
    uint8_t SDL_GetMouseState(int *x, int *y);
}

bool renderer_cool::get_mouse_coords(int32_t *x, int32_t *y)
{
    if (!screen_map_type)
        return get_mouse_coords_old(x, y);

    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);    
    
    mouse_x -= goff_x;
    mouse_y -= goff_y;

    int _x = (float) mouse_x / gdispx + 1;
    int _y = (float) mouse_y / gdispy + 1;

    if (_x < 1 || _y < 1 || _x > gdimx || _y > gdimy)
        return false;

    *x = _x;
    *y = _y;
    
    return true;
}