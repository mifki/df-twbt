static renderer_opengl *oldr_legacy;
static bool legacy_mode;

static void hook_legacy()
{
    if (enabled)
        return;

    MemoryPatcher p(Core::getInstance().p);    

    //XXX: This is a crazy work-around for vtable address for df::renderer not being available yet
    //in dfhack for 0.40.xx, which prevents its subclasses form being instantiated. We're overwriting
    //original vtable anyway, so any value will go.
    unsigned char zz[] = { 0xff, 0xff, 0xff, 0xff };
#ifdef WIN32
    //p.write((char*)&df::renderer::_identity + 72, zz, 4);
#else
    p.write((char*)&df::renderer::_identity + 128, zz, 4);
#endif

    oldr_legacy = (renderer_opengl*)enabler->renderer;
    renderer_legacy *newr = new renderer_legacy;

    void **vtable_old = ((void ***)oldr_legacy)[0];
    void ** volatile vtable_new = ((void ***)newr)[0];

#undef DEFIDX
#define DEFIDX(n) int IDX_##n = vmethod_pointer_to_idx(&renderer_legacy::n);

    DEFIDX(draw)
    DEFIDX(update_tile)
    DEFIDX(update_tile_old)
    DEFIDX(reshape_gl)
    DEFIDX(reshape_gl_old)

    void *draw_new             = vtable_new[IDX_draw];
    void *reshape_gl_new       = vtable_new[IDX_reshape_gl];
    void *update_tile_new      = vtable_new[IDX_update_tile];    

    p.verifyAccess(vtable_new, sizeof(void*)*32, true);
    memcpy(vtable_new, vtable_old, sizeof(void*)*32);

    vtable_new[IDX_draw] = draw_new;

    vtable_new[IDX_update_tile] = update_tile_new;
    vtable_new[IDX_update_tile_old] = vtable_old[IDX_update_tile];

    vtable_new[IDX_reshape_gl] = reshape_gl_new;
    vtable_new[IDX_reshape_gl_old] = vtable_old[IDX_reshape_gl];
    
    memcpy(&newr->screen, &oldr_legacy->screen, (char*)&newr->dummy-(char*)&newr->screen);
    enabler->renderer = (df::renderer*)newr;

    enabled = true;   
}

static void unhook_legacy()
{
    if (!enabled)
        return;

    enabled = false;

    //TODO: !!!
    enabler->renderer = (df::renderer*)oldr_legacy;

    gps->force_full_display_count = true;
}

static unsigned char screen2[256*256*4];
static long screentexpos2[256*256];
static int8_t screentexpos_addcolor2[256*256];
static uint8_t screentexpos_grayscale2[256*256];
static uint8_t screentexpos_cf2[256*256];
static uint8_t screentexpos_cbr2[256*256];

struct dwarfmode_hook_legacy : public df::viewscreen_dwarfmodest
{
    typedef df::viewscreen_dwarfmodest interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, render, ())
    {
        // These values may change from the main thread while being accessed from the rendering thread,
        // and that will cause flickering of overridden tiles at least, so save them here
        gwindow_x = *df::global::window_x;
        gwindow_y = *df::global::window_y;
        gwindow_z = *df::global::window_z;

        //clock_t c1 = clock();
        INTERPOSE_NEXT(render)();

        if (maxlevels)
            render_more_layers();

        //clock_t c2 = clock();
        //*out2 << (c2-c1) << std::endl;
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


        uint8_t *sctop = gps->screen;
        long *screentexpostop = gps->screentexpos;
        int8_t *screentexpos_addcolortop = gps->screentexpos_addcolor;
        uint8_t *screentexpos_grayscaletop = gps->screentexpos_grayscale;
        uint8_t *screentexpos_cftop = gps->screentexpos_cf;
        uint8_t *screentexpos_cbrtop = gps->screentexpos_cbr;

        gps->screen = screen2;
        gps->screen_limit = gps->screen + gps->dimx * gps->dimy * 4;
        gps->screentexpos = screentexpos2;
        gps->screentexpos_addcolor = screentexpos_addcolor2;
        gps->screentexpos_grayscale = screentexpos_grayscale2;
        gps->screentexpos_cf = screentexpos_cf2;
        gps->screentexpos_cbr = screentexpos_cbr2;

        bool empty_tiles_left, rendered1st = false;
        int p = 1;
        int x0 = 1;
        int zz0 = *df::global::window_z;        

        do
        {
            //TODO: if z=0 should just render and use for all tiles always
            if (*df::global::window_z == 0)
                break;

            (*df::global::window_z)--;

            if (p > 1)
            {
                (*df::global::window_x) += x0 - 1;
                init->display.grid_x -= x0 - 1;

                INTERPOSE_NEXT(render)();

                (*df::global::window_x) -= x0 - 1;
                init->display.grid_x += x0 - 1;
            }

            empty_tiles_left = false;
            int x00 = x0;
            int zz = zz0 - p + 1;

            //*out2 << p << " " << x0 << std::endl;
            
            GLfloat *vertices = ((renderer_opengl*)enabler->renderer)->vertexes;
            //TODO: test this
            int x1 = std::min(menu_left, world->map.x_count-*df::global::window_x+1);
            int y1 = std::min(h-1, world->map.y_count-*df::global::window_y+1);
            for (int x = x0; x < x1; x++)
            {
                for (int y = 1; y < y1; y++)
                {
                    const int tile = x * gps->dimy + y, stile = tile * 4;

                    if ((sctop[stile+3]&0xf0))
                        continue;

                    unsigned char ch = sctop[stile+0];
                    if (ch != 31 && ch != 249 && ch != 250 && ch != 254 && ch != skytile && ch != chasmtile && !(ch >= '1' && ch <= '7'))
                        continue;

                    int xx = *df::global::window_x + x - 1;
                    int yy = *df::global::window_y + y - 1;
                    if (xx < 0 || yy < 0)
                        continue;

                    //TODO: check for z=0 (?)
                    bool e0, h, h0;

                    int xxquot = xx >> 4, xxrem = xx & 15;
                    int yyquot = yy >> 4, yyrem = yy & 15;

                    df::map_block *block0 = world->map.block_index[xxquot][yyquot][zz0];
                    h0 = block0 && block0->designation[xxrem][yyrem].bits.hidden;
                    if (h0)
                        continue;
                    e0 = !block0 || ((block0->tiletype[xxrem][yyrem] == df::tiletype::OpenSpace || block0->tiletype[xxrem][yyrem] == df::tiletype::RampTop) && !block0->designation[xxrem][yyrem].bits.flow_size);
                    if (!(e0))
                        continue;

                    if (p == 1 && !rendered1st)
                    {
                        (*df::global::window_x) += x0 - 1;
                        init->display.grid_x -= x0 - 1;

                        INTERPOSE_NEXT(render)();

                        (*df::global::window_x) -= x0 - 1;
                        init->display.grid_x += x0 - 1;

                        x00 = x0;

                        rendered1st = true;
                    }

                    const int tile2 = (x-(x00-1)) * gps->dimy + y, stile2 = tile2 * 4;                    

                    int d = p;
                    ch = screen2[stile2+0];
                    if (!(ch!=31&&ch != 249 && ch != 250 && ch != 254 && ch != skytile && ch != chasmtile && !(ch >= '1' && ch <= '7')))
                    {
                        df::map_block *block1 = world->map.block_index[xxquot][yyquot][zz-1];
                        if (!block1)
                        {
                            //TODO: skip all other y's in this block
                            if (p < maxlevels)
                            {
                                empty_tiles_left = true;
                                continue;
                            }
                            else
                                d = p+1;
                        }
                        else
                        {
                            //TODO: check for hidden also
                            df::tiletype t1 = block1->tiletype[xxrem][yyrem];
                            if ((t1 == df::tiletype::OpenSpace || t1 == df::tiletype::RampTop) && !block1->designation[xxrem][yyrem].bits.flow_size)
                            {
                                if (p < maxlevels)
                                {
                                    empty_tiles_left = true;
                                    continue;
                                }
                                else
                                {
                                    if (t1 != df::tiletype::RampTop)
                                        d = p+1;
                                }
                            }
                        }
                    }

                    *((int*)sctop+tile) = *((int*)screen2+tile2);
                    if (*(screentexpos2+tile2))
                    {
                        *(screentexpostop+tile) = *(screentexpos2+tile2);
                        *(screentexpos_addcolortop+tile) = *(screentexpos_addcolor2+tile2);
                        *(screentexpos_grayscaletop+tile) = *(screentexpos_grayscale2+tile2);
                        *(screentexpos_cftop+tile) = *(screentexpos_cf2+tile2);
                        *(screentexpos_cbrtop+tile) = *(screentexpos_cbr2+tile2);
                    }
                    sctop[stile+3] = (0x10*d) | (sctop[stile+3]&0x0f);                                        
                }

                if (!empty_tiles_left)
                    x0 = x + 1;
            }

            if (p++ >= maxlevels)
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

IMPLEMENT_VMETHOD_INTERPOSE_PRIO(dwarfmode_hook_legacy, render, 1000);
