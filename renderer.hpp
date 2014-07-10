static volatile int domapshot = 0;

// This is from g_src/renderer_opengl.hpp
struct renderer_opengl_noconflict : df::renderer
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
typedef renderer_opengl_noconflict renderer_opengl; // This is to make Linux happy

struct renderer_cool : renderer_opengl
{
    // To know the size of renderer_opengl's fields
    void *dummy;

    virtual void update_tile(int x, int y);
    virtual void draw(int vertex_count);

    virtual void update_tile_old(int x, int y) {}; //17
};

void renderer_cool::update_tile(int x, int y)
{
    if (!enabled || !texloaded)
    {
        this->update_tile_old(x, y);
        return;
    }

    const int tile = x*gps->dimy + y;

    GLfloat *_fg  = fg + tile * 4 * 6;
    GLfloat *_bg  = bg + tile * 4 * 6;
    GLfloat *_tex = tex + tile * 2 * 6;

    write_tile_arrays(this, x, y, _fg, _bg, _tex);

    //const int tile = x * gps->dimy + y;
    float d = (float)((screen[tile*4+3]&0xf0)>>4);
    depth[tile] = d;
    fogcoord[tile*6+0] = d;
    fogcoord[tile*6+1] = d;
    fogcoord[tile*6+2] = d;
    fogcoord[tile*6+3] = d;
    fogcoord[tile*6+4] = d;
    fogcoord[tile*6+5] = d;
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

void renderer_cool::draw(int vertex_count)
{
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

            load_multi_pdim_x(t, ts.small_font_path, tilesets[j].small_texpos, 16, 16, true, &dx, &dy);
            if (ts.large_font_path != ts.small_font_path)
                load_multi_pdim_x(t, ts.large_font_path, tilesets[j].large_texpos, 16, 16, true, &dx, &dy);
            else
                memcpy(ts.large_texpos, ts.small_texpos, sizeof(ts.large_texpos));
        }

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

    float FogCol[3]={0.1f,0.1f,0.3f};
    //float FogCol[3]={0.8f,0.8f,0.8f};
    glEnable(GL_FOG);
    glFogfv(GL_FOG_COLOR,FogCol);
    glFogf(GL_FOG_DENSITY,0.15f);
    //glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_END, 20);
    glFogi(GL_FOG_COORD_SRC, GL_FOG_COORD);

    glFogCoordPointer(GL_FLOAT, 0, fogcoord);
    glEnableClientState(GL_FOG_COORD_ARRAY);

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

    glDisable(GL_FOG);    

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

            int d = depth[tile];
            if (d)
            {
                GLfloat *tex = shadowtex+elemcnt*2;

                bool top=false, left=false, btm=false, right=false;
                if (xx > 1 && (depth[((xx-1)*gps->dimy + yy)]) < d)
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                    SETTEX(shadow_texpos[0]);
                    elemcnt+=6;
                    left = true;
                }
                if (yy < h-2 && (depth[((xx)*gps->dimy + yy+1)]) < d)
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                    SETTEX(shadow_texpos[1]);
                    elemcnt+=6;
                    btm = true;
                }
                if (yy > 1 && (depth[((xx)*gps->dimy + yy-1)]) < d)
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                    SETTEX(shadow_texpos[2]);
                    elemcnt+=6;
                    top = true;
                }
                if (xx < menu_left-1 && (depth[((xx+1)*gps->dimy + yy)]) < d)
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                    SETTEX(shadow_texpos[3]);
                    elemcnt+=6;
                    right = true;
                }

                if (!right && !btm && xx < menu_left-1 && yy < h-2 && (depth[((xx+1)*gps->dimy + yy+1)]) < d)
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                    SETTEX(shadow_texpos[4]);
                    elemcnt+=6;
                }
                if (!left && !btm && xx > 1 && yy < h-2 && (depth[((xx-1)*gps->dimy + yy+1)]) < d)
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                    SETTEX(shadow_texpos[5]);
                    elemcnt+=6;
                }
                if (!left && !top && xx > 1 && yy > 1 && (depth[((xx-1)*gps->dimy + yy-1)]) < d)
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                    SETTEX(shadow_texpos[6]);
                    elemcnt+=6;
                }
                if (!top && !right && xx < menu_left-1 && yy > 1 && (depth[((xx+1)*gps->dimy + yy-1)]) < d)
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));
                    SETTEX(shadow_texpos[7]);
                    elemcnt+=6;
                }
            }            
        }
    }

    if (elemcnt)
    {
        glDisableClientState(GL_COLOR_ARRAY);
        glColor4f(0, 0, 0, 0.4f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
        glTexCoordPointer(2, GL_FLOAT, 0, shadowtex);
        glVertexPointer(2, GL_FLOAT, 0, shadowvert);
        glDrawArrays(GL_TRIANGLES, 0, elemcnt);
        glEnableClientState(GL_COLOR_ARRAY);    
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
