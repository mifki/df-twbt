#ifndef _RENDERER_TWBT_H
#define _RENDERER_TWBT_H

// This is from g_src/renderer_opengl.hpp
struct _renderer_opengl : df::renderer
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

    renderer_cool();

    void reshape_graphics();
    void display_new();
    void update_map_tile(int x, int y);

    virtual void update_tile(int x, int y);
    virtual void draw(int vertex_count);
    virtual void reshape_gl();

    virtual bool get_mouse_coords(int32_t *x, int32_t *y);

    virtual void update_tile_old(int x, int y) {}; //17
    virtual void reshape_gl_old() {}; //18

    virtual void _last_vmethod() {};
};

#endif