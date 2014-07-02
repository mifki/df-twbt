static volatile int domapshot = 0;



void renderer_cool::update_tile(int x, int y)
{
    if (!enabled || !texloaded)
    {
        this->update_tile_old(x, y);
        return;
    }

    if (gupdate)
    {
    const int tile = x*gdimy + y;

    GLfloat *_fg  = gfg + tile * 4 * 6;
    GLfloat *_bg  = gbg + tile * 4 * 6;
    GLfloat *_tex = gtex + tile * 2 * 6;

    write_tile_arrays(this, x, y, _fg, _bg, _tex);

        //const int tile = x * gps->dimy + y;
        float d = (float)((screen2[tile*4+3]&0xf0)>>4);
        fogcoord[tile*6+0] = d;
        fogcoord[tile*6+1] = d;
        fogcoord[tile*6+2] = d;
        fogcoord[tile*6+3] = d;
        fogcoord[tile*6+4] = d;
        fogcoord[tile*6+5] = d;
    }
else
{
    const int tile = x*gps->dimy + y;

    GLfloat *_fg  = fg + tile * 4 * 6;
    GLfloat *_bg  = bg + tile * 4 * 6;
    GLfloat *_tex = tex + tile * 2 * 6;

    write_tile_arrays(this, x, y, _fg, _bg, _tex);
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

  static void write_tile_vertexes(GLfloat x, GLfloat y, GLfloat *vertex) {
    vertex[0]  = x;   // Upper left
    vertex[1]  = y;
    vertex[2]  = x+1; // Upper right
    vertex[3]  = y;
    vertex[4]  = x;   // Lower left
    vertex[5]  = y+1;
    vertex[6]  = x;   // Lower left again (triangle 2)
    vertex[7]  = y+1;
    vertex[8]  = x+1; // Upper right
    vertex[9]  = y;
    vertex[10] = x+1; // Lower right
    vertex[11] = y+1;
  }    

    void renderer_cool::reshape_graphics()
  {
    {
        float tsx = (float)size_x/gps->dimx, tsy = (float)size_y/gps->dimy;



        int32_t w = gps->dimx, h = gps->dimy;
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


        int cx = *df::global::window_x + gdimx / 2;
        int cy = *df::global::window_y + gdimy / 2;


        gsize_x = (size_x-tsx*(menu_w+1+1));
        gsize_y = (size_y-tsy*2);
*out2 << tsx << " " << gsize_x << std::endl;
        float dimx = gsize_x / gdispx;
        float dimy = gsize_y / gdispy;
        gdimx = ceilf(dimx);
        gdimy = ceilf(dimy);
        gdimxfull = floorf(dimx);
        gdimyfull = floorf(dimy);
        *out2 << gdispx << " " << gdispy << "   " << gdimx << " " << gdimy << "   " << gdimxfull << " " << gdimyfull << " " << (gsize_x-gdispx*gdimxfull) << " " << (gsize_y-gdispy*gdimyfull) << std::endl;

        *df::global::window_x = cx - gdimx/2;
        *df::global::window_y = cy - gdimy/2;

        int tiles = gdimx * gdimy;
        gvertexes = static_cast<GLfloat*>(realloc(gvertexes, sizeof(GLfloat) * tiles * 2 * 6));
        gfg = static_cast<GLfloat*>(realloc(gfg, sizeof(GLfloat) * tiles * 4 * 6));
        gbg = static_cast<GLfloat*>(realloc(gbg, sizeof(GLfloat) * tiles * 4 * 6));
        gtex = static_cast<GLfloat*>(realloc(gtex, sizeof(GLfloat) * tiles * 2 * 6));
*out2 << gvertexes << " " << gfg << " " << gbg << " " << gtex << std::endl;
        int tile = 0;
        for (GLfloat x = 0; x < gdimx; x++)
          for (GLfloat y = 0; y < gdimy; y++, tile++)
            write_tile_vertexes(x, y, gvertexes + 6*2*tile);

        *out2 << gvertexes << std::endl;
    }        
  }

  void renderer_cool::reshape_gl()
  {
    *out2 << "RESHAPE" << std::endl;
    reshape_gl_old();
    //if (!gvertexes)
    reshape_graphics();
  }



void renderer_cool::draw(int vertex_count)
{

    if (gvertexes)
    {

        uint8_t *sctop = enabler->renderer->screen;
        int32_t *screentexpostop = enabler->renderer->screentexpos;
        int8_t *screentexpos_addcolortop = enabler->renderer->screentexpos_addcolor;
        uint8_t *screentexpos_grayscaletop = enabler->renderer->screentexpos_grayscale;
        uint8_t *screentexpos_cftop = enabler->renderer->screentexpos_cf;
        uint8_t *screentexpos_cbrtop = enabler->renderer->screentexpos_cbr;
        gps->screen = enabler->renderer->screen = screen2;
        gps->screen_limit = gps->screen + gdimx * gdimy * 4;
        gps->screentexpos = enabler->renderer->screentexpos = screentexpos2;
        gps->screentexpos_addcolor = enabler->renderer->screentexpos_addcolor = screentexpos_addcolor2;
        gps->screentexpos_grayscale = enabler->renderer->screentexpos_grayscale = screentexpos_grayscale2;
        gps->screentexpos_cf = enabler->renderer->screentexpos_cf = screentexpos_cf2;
        gps->screentexpos_cbr = enabler->renderer->screentexpos_cbr = screentexpos_cbr2;
    gupdate = true;
      for (int x2=0; x2 < gdimx; x2++)
        for (int y2=0; y2 < gdimy; y2++)
            update_tile(x2,y2);

    gupdate = false;
        gps->screen = enabler->renderer->screen = sctop;
        gps->screen_limit = gps->screen + gps->dimx * gps->dimy * 4;
        gps->screentexpos = enabler->renderer->screentexpos = screentexpostop;
        gps->screentexpos_addcolor = enabler->renderer->screentexpos_addcolor = screentexpos_addcolortop;
        gps->screentexpos_grayscale = enabler->renderer->screentexpos_grayscale = screentexpos_grayscaletop;
        gps->screentexpos_cf = enabler->renderer->screentexpos_cf = screentexpos_cftop;
        gps->screentexpos_cbr = enabler->renderer->screentexpos_cbr = screentexpos_cbrtop;
}

#ifdef WIN32
    static bool glew_init = false;
    if (!glew_init)
    {
        glew_init = true;
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
          /* Problem: glewInit failed, something is seriously wrong. */
          *out2 << glewGetErrorString(err);
        }
    }
#endif

    if (!texloaded)
    {
        long dx, dy;
        void *t = &enabler->textures;

        for (int j = 0; j < tilesets.size(); j++)
        {
            struct tileset &ts = tilesets[j];
            if (!ts.small_font_path.length())
                continue;

            long buf[256];
            if (j > 1)
                load_multi_pdim_x(t, ts.small_font_path, tilesets[j].small_texpos, 16, 16, true, &dx, &dy);
            else
            {
                gdispx = init->font.small_font_dispx;
                gdispy = init->font.small_font_dispy;
                memcpy(tilesets[j].small_texpos, init->font.small_font_texpos, sizeof(long)*256);
                load_multi_pdim_x(t, ts.small_font_path, (long*)init->font.small_font_texpos, 16, 16, true, (long*)&init->font.small_font_dispx, (long*)&init->font.small_font_dispy);
                dispx = init->font.small_font_dispx;
                dispy = init->font.small_font_dispy;
            }

            if (ts.large_font_path != ts.small_font_path)
                load_multi_pdim_x(t, ts.large_font_path, tilesets[j].large_texpos, 16, 16, true, &dx, &dy);
            else
                memcpy(ts.large_texpos, ts.small_texpos, sizeof(ts.large_texpos));

            resize((size_x/dispx)*dispx, (size_y/dispy)*dispy);
        }

        /*struct tileset ts;
        load_multi_pdim_x(t, "data/art/Spacefox_16x16.png", ts.small_texpos, 16, 16, true, &dx, &dy);
        tilesets.push_back(ts);
        *out2 << "=="<<tilesets.size() << std::endl;*/

        // Load shadows
        struct stat buf;
        if (stat("data/art/shadows.png", &buf) == 0)
        {
            load_multi_pdim_x(t, "data/art/shadows.png", shadow_texpos, 8, 1, false, &dx, &dy);        
            shadowsloaded = true;
        }
        else
        {
            out2->color(COLOR_RED);
            *out2 << "TWBT: shadows.png not found in data/art folder" << std::endl;
            out2->color(COLOR_RESET);
        }

        texloaded = true;
        gps->force_full_display_count = true;
    }

    static int old_dimx, old_dimy, old_winx, old_winy;
    if (domapshot)
    {
        if (domapshot == 10)
        {
            old_dimx = gps->dimx;
            old_dimy = gps->dimy;
            old_winx = *df::global::window_x;
            old_winy = *df::global::window_y;

            grid_resize(world->map.x_count+36, world->map.y_count+2);
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


    static df::viewscreen *prevws = NULL;
    df::viewscreen *ws = Gui::getCurViewscreen();
    if (ws != prevws)
    {
        gps->force_full_display_count = true;
        prevws = ws;
    }
    if (df::viewscreen_dwarfmodest::_identity.is_direct_instance(ws))
    {
    if (!gvertexes)
    {
        reshape_graphics();
    }
{
    /////
    glViewport(off_x+roundf((float)size_x/gps->dimx), off_y+roundf((float)size_y/gps->dimy)-(gdimy==gdimyfull?0:roundf(gdispy-(gsize_y-gdispy*gdimyfull))), gdimx*gdispx, gdimy*gdispy);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, gdimx, gdimy, 0,-1,1);
    //glTranslatef(1,-1,0);

    //glScissor(off_x+(float)size_x/gps->dimx, off_y+(float)size_y/gps->dimy, gsize_x, gsize_y);
    //glEnable(GL_SCISSOR_TEST);
    //glClearColor(1,0,0,1);
    //glClear(GL_COLOR_BUFFER_BIT);

    float FogCol[3]={0.1f,0.1f,0.3f};
    //float FogCol[3]={0.8f,0.8f,0.8f};
    //glEnable(GL_FOG);
    glFogfv(GL_FOG_COLOR,FogCol);
    glFogf(GL_FOG_DENSITY,0.15f);
    //glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_END, 20);
    glFogi(GL_FOG_COORD_SRC, GL_FOG_COORD);

    glFogCoordPointer(GL_FLOAT, 0, fogcoord);
    glEnableClientState(GL_FOG_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, gvertexes);

    // Render background colors
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_BLEND);
    glColorPointer(4, GL_FLOAT, 0, gbg);
    glDrawArrays(GL_TRIANGLES, 0, gdimx*gdimy*6);

    // Render foreground
    //glAlphaFunc(GL_NOTEQUAL, 0);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexCoordPointer(2, GL_FLOAT, 0, gtex);
    glColorPointer(4, GL_FLOAT, 0, gfg);
    glDrawArrays(GL_TRIANGLES, 0, gdimx*gdimy*6);

    glDisable(GL_FOG);    
    glDisable(GL_SCISSOR_TEST);
}
}
    {
glViewport(off_x, off_y, size_x, size_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, gps->dimx, gps->dimy, 0,-1,1);

    glVertexPointer(2, GL_FLOAT, 0, vertexes);

    // Render background colors
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_BLEND);
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
}
    return;
    // Prepare and render shadows
    short elemcnt = 0;
    //TODO: don't do this if view not moved and tiles with shadows not changed
    {
        gl_texpos *txt = (gl_texpos*) enabler->textures.gl_texpos;

        for (int tile = 0; tile < gps->dimx*gps->dimy; tile++)
        {
            if ((screen[tile*4+3]&0xf0))
            {
                GLfloat *tex = shadowtex+elemcnt*2;
                unsigned char kk = shadows[tile];

                if (kk & (1 << 0))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                    SETTEX(shadow_texpos[0]);
                    elemcnt+=6;
                }
                if (kk & (1 << 1))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(shadow_texpos[1]);
                    elemcnt+=6;
                }
                if (kk & (1 << 2))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(shadow_texpos[2]);
                    elemcnt+=6;
                }
                if (kk & (1 << 3))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(shadow_texpos[3]);
                    elemcnt+=6;
                }        
                if (kk & (1 << 4))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(shadow_texpos[4]);
                    elemcnt+=6;
                }               
                if (kk & (1 << 5))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(shadow_texpos[5]);
                    elemcnt+=6;
                }            
                if (kk & (1 << 6))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(shadow_texpos[6]);
                    elemcnt+=6;
                }        
                if (kk & (1 << 7))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(shadow_texpos[7]);
                    elemcnt+=6;
                }
            } 
        }
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glColor4f(0, 0, 0, 0.4f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
    glTexCoordPointer(2, GL_FLOAT, 0, shadowtex);
    glVertexPointer(2, GL_FLOAT, 0, shadowvert);
    glDrawArrays(GL_TRIANGLES, 0, elemcnt);
    glEnableClientState(GL_COLOR_ARRAY);    


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
        std::ofstream img("mapshot.tga");
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

/*void display()
{
  const int dimx = init.display.grid_x;
  const int dimy = init.display.grid_y;
  static bool use_graphics = init.display.flag.has_flag(INIT_DISPLAY_FLAG_USE_GRAPHICS);
  if (gps.force_full_display_count) {
    // Update the entire screen
    update_all();
  } else {
    Uint32 *screenp = (Uint32*)screen, *oldp = (Uint32*)screen_old;
    if (use_graphics) {
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
    } else {
      for (int x2=0; x2 < dimx; ++x2) {
        for (int y2=0; y2 < dimy; ++y2, ++screenp, ++oldp) {
          if (*screenp != *oldp) {
            update_tile(x2, y2);
          }
        }
      }
    }
  }
  if (gps.force_full_display_count > 0) gps.force_full_display_count--;
}    */
}