static volatile int domapshot = 0;

renderer_cool::renderer_cool()
{
    dummy = 'TWBT';
    gvertexes = 0, gfg = 0, gbg = 0, gtex = 0;
    gdimx = 0, gdimy = 0, gdimxfull = 0, gdimyfull = 0;
    gdispx = 0, gdispy = 0;
    goff_x = 0, goff_y = 0, gsize_x = 0, gsize_y = 0;
    needs_reshape = needs_zoom = 0;
}

void renderer_cool::update_tile(int x, int y)
{
    if (!enabled)
    {
        this->update_tile_old(x, y);
        return;
    }

    const int tile = x * tdimy + y;

    GLfloat *_fg  = fg + tile * 4 * 6;
    GLfloat *_bg  = bg + tile * 4 * 6;
    GLfloat *_tex = tex + tile * 2 * 6;

    write_tile_arrays_text(this, x, y, _fg, _bg, _tex);
}

void renderer_cool::update_map_tile(int x, int y)
{
    const int tile = x * gdimy + y;

    GLfloat *_fg  = gfg + tile * 4 * 6;
    GLfloat *_bg  = gbg + tile * 4 * 6;
    GLfloat *_tex = gtex + tile * 2 * 6;

    write_tile_arrays_map(this, x, y, _fg, _bg, _tex);

    if (maxlevels)
    {
        float d = (float)((gscreen[tile * 4 + 3] & 0xf0) >> 4);

        depth[tile] = !gscreen[tile*4] ? 0x7f : d; //TODO: no need for this in fort mode

        if (fogdensity > 0)
        {
            fogcoord[tile * 6 + 0] = d;
            fogcoord[tile * 6 + 1] = d;
            fogcoord[tile * 6 + 2] = d;
            fogcoord[tile * 6 + 3] = d;
            fogcoord[tile * 6 + 4] = d;
            fogcoord[tile * 6 + 5] = d;
        }
    }
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

static void write_tile_vertexes(GLfloat x, GLfloat y, GLfloat *vertex)
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

void renderer_cool::reshape_graphics()
{
    float tsx = (float)size_x / gps->dimx, tsy = (float)size_y / gps->dimy;

    int32_t w = gps->dimx, h = gps->dimy;

    int cx = *df::global::window_x + gdimx / 2;
    int cy = *df::global::window_y + gdimy / 2;

    df::viewscreen *ws = Gui::getCurViewscreen();
    if (df::viewscreen_dwarfmodest::_identity.is_direct_instance(ws))
    {
        gsize_x = (size_x - tsx * (gmenu_w + 1 + 1));
        gsize_y = (size_y - tsy * 2);
        goff_x = off_x + roundf(tsx);
        goff_y = off_y + roundf(tsy);        
    }
    else //Adv. mode
    {
        *out2 << "reshape_graphics" << std::endl;
        gsize_x = size_x;
        gsize_y = size_y;
        goff_x = off_x;
        goff_y = goff_y_gl = off_y;
    }

    float dimx = std::min(gsize_x / gdispx, 256.0f);
    float dimy = std::min(gsize_y / gdispy, 256.0f);
    gdimx = ceilf(dimx);
    gdimy = ceilf(dimy);
    gdimxfull = floorf(dimx);
    gdimyfull = floorf(dimy);

    if (df::viewscreen_dwarfmodest::_identity.is_direct_instance(ws))
        goff_y_gl = goff_y - (gdimy == gdimyfull ? 0 : roundf(gdispy - (gsize_y - gdispy * gdimyfull)));                

    *df::global::window_x = std::max(0, cx - gdimx / 2);
    *df::global::window_y = std::max(0, cy - gdimy / 2);

    int tiles = gdimx * gdimy;

    // Recreate tile buffers
    allocate_buffers(tiles);

    // Recreate OpenGL buffers
    gvertexes = (GLfloat*)realloc(gvertexes, sizeof(GLfloat) * tiles * 2 * 6);
    gfg = (GLfloat*)realloc(gfg, sizeof(GLfloat) * tiles * 4 * 6);
    gbg = (GLfloat*)realloc(gbg, sizeof(GLfloat) * tiles * 4 * 6);
    gtex = (GLfloat*)realloc(gtex, sizeof(GLfloat) * tiles * 2 * 6);

    // Initialise vertex coords
    int tile = 0;   
    for (GLfloat x = 0; x < gdimx; x++)
        for (GLfloat y = 0; y < gdimy; y++, tile++)
            write_tile_vertexes(x, y, gvertexes + 6 * 2 * tile);

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
        if (last_fullscreen)
        {
            map_texpos = tilesets[0].large_texpos;
            text_texpos = tilesets[1].large_texpos;
            gdispx = large_map_dispx, gdispy = large_map_dispy;            
        }
        else
        {
            map_texpos = tilesets[0].small_texpos;
            text_texpos = tilesets[1].small_texpos;
            gdispx = small_map_dispx, gdispy = small_map_dispy;            
        }
    }

    reshape_graphics();
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

    static df::viewscreen *prevws = NULL;
    df::viewscreen *ws = Gui::getCurViewscreen();
    bool is_main_scr = df::viewscreen_dwarfmodest::_identity.is_direct_instance(ws) || df::viewscreen_dungeonmodest::_identity.is_direct_instance(ws);
    if (ws != prevws)
    {
        gps->force_full_display_count = true;
        prevws = ws;
        /*if (is_main_scr)
        {
            for (int x = 1; x < gps->dimx-gmenu_w-1; x++)
            {
                for (int y = 1; y < gps->dimy-1; y++)
                {
                    const int tile1 = x * gps->dimy + y;
                    for (int i = 0; i < 6; i++)
                        *(fg + tile * 4 * i + 3) = 0;
                }
            }
        }*/
    }    

    display_new(is_main_scr);

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

    if (is_main_scr)
    {
        bool skip = false;
        /*if (df::viewscreen_dungeonmodest::_identity.is_direct_instance(ws))
        {
            int m = df::global::ui_advmode->menu;
            if (m == df::ui_advmode_menu::Travel || m == df::ui_advmode_menu::Inventory || m == df::ui_advmode_menu::Eat)
                skip = true;
        }*/

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

            if (multi_rendered && fogdensity > 0)
            {
                glEnable(GL_FOG);
                glFogfv(GL_FOG_COLOR, fogcolor);
                glFogf(GL_FOG_DENSITY, fogdensity);
                glFogi(GL_FOG_COORD_SRC, GL_FOG_COORD);
                glFogCoordPointer(GL_FLOAT, 0, fogcoord);
                glEnableClientState(GL_FOG_COORD_ARRAY);
            }

            glVertexPointer(2, GL_FLOAT, 0, gvertexes);

            // Render background colors
            glDisable(GL_TEXTURE_2D);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisable(GL_BLEND);
            glColorPointer(4, GL_FLOAT, 0, gbg);
            glDrawArrays(GL_TRIANGLES, 0, gdimx * gdimy * 6);

            // Render foreground
            glEnable(GL_TEXTURE_2D);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glTexCoordPointer(2, GL_FLOAT, 0, gtex);
            glColorPointer(4, GL_FLOAT, 0, gfg);
            glDrawArrays(GL_TRIANGLES, 0, gdimx * gdimy * 6);

            if (multi_rendered)
                glDisable(GL_FOG);

            // Prepare and render shadows
            if (multi_rendered)
            {
                short elemcnt = 0;
                //TODO: don't do this if view not moved and tiles with shadows not changed
                {
                    gl_texpos *txt = (gl_texpos *) enabler->textures.gl_texpos;
                    int x1 = std::min(gdimx, world->map.x_count-*df::global::window_x);                
                    int y1 = std::min(gdimy, world->map.y_count-*df::global::window_y);

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
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[0]);
                                elemcnt += 6;
                                left = true;
                            }
                            if (yy < y1 - 1 && (depth[((xx)*gdimy + yy + 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[1]);
                                elemcnt += 6;
                                btm = true;
                            }
                            if (yy > 0 && (depth[((xx)*gdimy + yy - 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[2]);
                                elemcnt += 6;
                                top = true;
                            }
                            if (xx < x1-1 && (depth[((xx + 1)*gdimy + yy)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[3]);
                                elemcnt += 6;
                                right = true;
                            }

                            if (!right && !btm && xx < x1 - 1 && yy < y1 - 1 && (depth[((xx + 1)*gdimy + yy + 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[4]);
                                elemcnt += 6;
                            }
                            if (!left && !btm && xx > 0 && yy < y1 - 1 && (depth[((xx - 1)*gdimy + yy + 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[5]);
                                elemcnt += 6;
                            }
                            if (!left && !top && xx > 0 && yy > 0 && (depth[((xx - 1)*gdimy + yy - 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile * 6 * 2, 6 * 2 * sizeof(float));
                                SETTEX(shadow_texpos[6]);
                                elemcnt += 6;
                            }
                            if (!top && !right && xx < x1 - 1 && yy > 0 && (depth[((xx + 1)*gdimy + yy - 1)]) < d)
                            {
                                memcpy(shadowvert + elemcnt * 2, gvertexes + tile * 6 * 2, 6 * 2 * sizeof(float));
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
        }
    }
    {
        glViewport(off_x, off_y, size_x, size_y);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, tdimx, tdimy, 0, -1, 1);

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
#if defined(__APPLE__) || defined(WIN32)
    // In this case this function replaces original (non-virtual) renderer::display()
    // So update text tiles here

    const int dimx = tdimx;//init->display.grid_x;
    const int dimy = tdimy;//init->display.grid_y;

    if (gps->force_full_display_count) {
        update_all();
    } else {
        uint32_t *screenp = (uint32_t*)screen, *oldp = (uint32_t*)screen_old;
        /*if (use_graphics) {
        int off = 0;
        for (int x2=0; x2 < dimx; x2++) {
        for (int y2=0; y2 < dimy; y2++, ++off, ++screenp, ++oldp) {
        // We don't use pointers for the non-screen arrays because we mostly fail at the
        // *first* comparison, and having pointers for the others would exceed register
        // count.
        // Partial printing (and color-conversion): Big-ass if.
        if (*screenp == *oldp &&
        screentexpos[off] == screentexpos_old[off] &&
        screentexpos_addcolor[off] == screentexpos_addcolor_old[off] &&
        screentexpos_grayscale[off] == screentexpos_grayscale_old[off] &&
        screentexpos_cf[off] == screentexpos_cf_old[off] &&
        screentexpos_cbr[off] == screentexpos_cbr_old[off])
        {
        // Nothing's changed, this clause deliberately empty
        } else {
        update_tile(x2, y2);
        }
        }
        }
        } else {*/
        for (int x2=0; x2 < dimx; ++x2) {
            for (int y2=0; y2 < dimy; ++y2, ++screenp, ++oldp) {
                if (*screenp != *oldp) {
                    update_tile(x2, y2);
                }
            }
        }
        //}
    }

    if (gps->force_full_display_count > 0) gps->force_full_display_count--;
#endif

    // Update map tiles is current screen has a map
    if (update_graphics)
    {
        if (needs_full_update)
        {
            needs_full_update = false;
    
            for (int x2 = 0; x2 < gdimx; x2++)
                for (int y2 = 0; y2 < gdimy; y2++)
                    update_map_tile(x2, y2);
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

        gswap_arrays();
    }
} 

void renderer_cool::gswap_arrays()
{
    static int j = 0;

    this->gscreen = ::gscreen = _gscreen[j];
    gscreentexpos = _gscreentexpos[j];
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

void renderer_cool::allocate_buffers(int tiles)
{
#define REALLOC(var,type,count) var = (type*)realloc(var, count)

    this->gscreen = ::gscreen   = _gscreen[0]                 = (uint8_t*) realloc(_gscreen[0],                 tiles * 4 * 2);
    gscreentexpos               = _gscreentexpos[0]           = (int32_t*) realloc(_gscreentexpos[0],           sizeof(int32_t) * tiles * 2);
    gscreentexpos_addcolor      = _gscreentexpos_addcolor[0]  = (int8_t*)  realloc(_gscreentexpos_addcolor[0],  tiles * 2);
    gscreentexpos_grayscale     = _gscreentexpos_grayscale[0] = (uint8_t*) realloc(_gscreentexpos_grayscale[0], tiles * 2);
    gscreentexpos_cf            = _gscreentexpos_cf[0]        = (uint8_t*) realloc(_gscreentexpos_cf[0],        tiles * 2);
    gscreentexpos_cbr           = _gscreentexpos_cbr[0]       = (uint8_t*) realloc(_gscreentexpos_cbr[0],       tiles * 2);

    gscreen_old                 = _gscreen[1]                 = gscreen                 + tiles * 4;
    gscreentexpos_old           = _gscreentexpos[1]           = gscreentexpos           + tiles;
    gscreentexpos_addcolor_old  = _gscreentexpos_addcolor[1]  = gscreentexpos_addcolor  + tiles;
    gscreentexpos_grayscale_old = _gscreentexpos_grayscale[1] = gscreentexpos_grayscale + tiles;
    gscreentexpos_cf_old        = _gscreentexpos_cf[1]        = gscreentexpos_cf        + tiles;
    gscreentexpos_cbr_old       = _gscreentexpos_cbr[1]       = gscreentexpos_cbr       + tiles;

    //TODO: don't allocate arrays below if multilevel rendering is not enabled
    depth                   = (int8_t*)  realloc(depth,                   tiles);
    shadowtex               = (GLfloat*) realloc(shadowtex,               sizeof(GLfloat) * tiles * 2 * 6);
    shadowvert              = (GLfloat*) realloc(shadowvert,              sizeof(GLfloat) * tiles * 2 * 6);
    fogcoord                = (GLfloat*) realloc(fogcoord,                sizeof(GLfloat) * tiles * 6);

    mscreen                 = (uint8_t*) realloc(mscreen,                 tiles * 4);
    mscreentexpos           = (int32_t*) realloc(mscreentexpos,           sizeof(uint32_t) * tiles);
    mscreentexpos_addcolor  = (int8_t*)  realloc(mscreentexpos_addcolor,  tiles);
    mscreentexpos_grayscale = (uint8_t*) realloc(mscreentexpos_grayscale, tiles);
    mscreentexpos_cf        = (uint8_t*) realloc(mscreentexpos_cf,        tiles);
    mscreentexpos_cbr       = (uint8_t*) realloc(mscreentexpos_cbr,       tiles);

    memset(_gscreentexpos[0], 0, sizeof(uint32_t) * tiles * 2);
    memset(mscreentexpos, 0, sizeof(uint32_t) * tiles);
}

void renderer_cool::handle_reshape_zoom_requests()
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
                gdispx++;
                gdispy++;
            }
            else
            {
                gdispx--;
                gdispy--;

                if (gsize_x / gdispx > world->map.x_count)
                    gdispx = gdispy = gsize_x / world->map.x_count;
                else if (gsize_y / gdispy > world->map.y_count)
                    gdispx = gdispy = gsize_y / world->map.y_count;
            }
        
            needs_zoom = 0;
        }
        
        needs_reshape = false;
        reshape_graphics();
    }    
}

extern "C" {
    uint8_t SDL_GetMouseState(int *x, int *y);
}

bool renderer_cool::get_mouse_coords(int32_t *x, int32_t *y)
{
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);    
    
    mouse_x -= goff_x;
    mouse_y -= goff_y;

    int _x = (float) mouse_x / gdispx + 1;
    int _y = (float) mouse_y / gdispy + 1;

    if (_x < 0 || _y < 0 || _x > gdimxfull || _y > gdimyfull)
        return false;

    *x = _x;
    *y = _y;
    
    return true;
}
