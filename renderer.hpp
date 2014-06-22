static volatile int domapshot = 0;

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

    if (domapshot)
    {
        if (domapshot == 10)
        {
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
        GLuint imageWidth = gps->dimx*16,
               imageHeight = gps->dimy*16;
        //Set up a FBO with one renderbuffer attachment
        glGenFramebuffersEXT(1, &framebuffer);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
        glGenRenderbuffersEXT(1, &renderbuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, imageWidth, imageHeight);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                 GL_RENDERBUFFER_EXT, renderbuffer);        
        glViewport(0,0,gps->dimx*16,gps->dimy*16);

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

    // Render background color
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_BLEND);
    glColorPointer(4, GL_FLOAT, 0, bg);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    // Render foreground
    glAlphaFunc(GL_NOTEQUAL, 0);
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
                    SETTEX(0x70);
                    elemcnt+=6;
                }
                if (kk & (1 << 1))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(0x71);
                    elemcnt+=6;
                }
                if (kk & (1 << 2))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(0x72);
                    elemcnt+=6;
                }
                if (kk & (1 << 3))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(0x73);
                    elemcnt+=6;
                }        
                if (kk & (1 << 4))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(0x74);
                    elemcnt+=6;
                }               
                if (kk & (1 << 5))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(0x75);
                    elemcnt+=6;
                }            
                if (kk & (1 << 6))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(0x76);
                    elemcnt+=6;
                }        
                if (kk & (1 << 7))
                {
                    memcpy(shadowvert+elemcnt*2, vertexes+tile*6*2, 6*2*sizeof(float));

                    SETTEX(0x77);
                    elemcnt+=6;
                }
            } 
        }
    }

    glColor4f(0, 0, 0.0, 0.4);
    glDisableClientState(GL_COLOR_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, shadowtex);
    glVertexPointer(2, GL_FLOAT, 0, shadowvert);
    glDrawArrays(GL_TRIANGLES, 0, elemcnt);
    glEnableClientState(GL_COLOR_ARRAY);    


    if (domapshot==1)
    {
        int w = world->map.x_count*dispx;
        int h = world->map.y_count*dispy;

        unsigned char *data = (unsigned char*) malloc(w*h*3);
        
        //glPixelStorei(GL_PACK_ALIGNMENT, 1);
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
        std::ofstream img("img.tga");
        img.write((const char*)&hdr, sizeof(hdr));
/*        for (int j = 0; j<w*h*3; j++)
        {
            unsigned char c = data[j+0];
            data[0] = data[j+2];
            data[j+2] = c;
        }*/
        img.write((const char*)data, w*h*3);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
// Delete the renderbuffer attachment
glDeleteRenderbuffersEXT(1, &renderbuffer);


        domapshot = 0;        
    }
}