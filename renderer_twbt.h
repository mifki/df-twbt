#ifndef _RENDERER_TWBT_H
#define _RENDERER_TWBT_H

#include "df/zoom_commands.h"

// This is from g_src/renderer_opengl.hpp
struct _renderer_opengl : public df::renderer
{
    void *sdlscreen;
    int dispx, dispy;
    float *vertexes, *fg, *bg, *tex;
    int zoom_steps, forced_steps;
    int natural_w, natural_h;
    int off_x, off_y, size_x, size_y;

    virtual void allocate(int tiles) {};
    virtual void init_opengl() {};
    virtual void uninit_opengl() {};
    virtual void draw(int vertex_count) {};
    virtual ~_renderer_opengl() {};
    virtual void reshape_gl() {};
};
typedef _renderer_opengl renderer_opengl; // This is to make Linux happy

struct renderer_cool : renderer_opengl
{
    uint32_t dummy;

    float *gvertexes, *gfg, *gbg, *gtex;
    int gdimx, gdimy, gdimxfull, gdimyfull;
    int gdispx, gdispy;
    float goff_x, goff_y, gsize_x, gsize_y;
	bool needs_reshape;
    int needs_zoom;
    bool needs_full_update;
    unsigned char *gscreen;
    int32_t *gscreentexpos;
    float goff_y_gl;

    renderer_cool();

    void reshape_graphics();
    void display_new(bool update_graphics);
    void gswap_arrays();
    void allocate_buffers(int tiles);
    void update_map_tile(int x, int y);
    void reshape_zoom_swap();

    virtual void update_tile(int x, int y);
    virtual void draw(int vertex_count);
    virtual void reshape_gl();

    virtual void zoom(df::zoom_commands cmd); 
    virtual bool get_mouse_coords(int32_t *x, int32_t *y);

    virtual void update_tile_old(int x, int y) {}; //17
    virtual void reshape_gl_old() {}; //18
    virtual bool get_mouse_coords_old(int32_t *x, int32_t *y) { return false; };
    virtual void zoom_old(df::zoom_commands cmd) {};

    virtual void _last_vmethod() {};

    bool is_twbt() {
        return (this->dummy == 'TWBT');
    };

    void output_string(int8_t color, int x, int y, std::string str)
    {

    };

    void output_char(int8_t color, int x, int y, unsigned char ch)
    {
        const int tile = (x-1) * gdimy + (y-1);
        if (x < 1 || x > gdimx || y < 1 || y > gdimy)
            return;

        unsigned char *s = gscreen + tile*4;
        s[0] = ch;
        s[1] = color % 8;
        s[2] = 0;
        s[3] = (color / 8) | (s[3]&0xf0);

        gscreentexpos[tile] = 0;
    };    

    DFHack::Gui::DwarfmodeDims map_dims()
    {
        DFHack::Gui::DwarfmodeDims dims = { 1, gdimx, 0, 0, 0, 0, 1, gdimy };
        return dims;
    };
};

#endif