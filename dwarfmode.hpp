struct dwarfmode_hook : public df::viewscreen_dwarfmodest
{
    typedef df::viewscreen_dwarfmodest interpose_base;

    int get_menu_width()
    {
        uint8_t menu_width, area_map_width;
        Gui::getMenuWidth(menu_width, area_map_width);
        int32_t menu_w = 0;

        bool menuforced = (ui->main.mode != df::ui_sidebar_mode::Default || df::global::cursor->x != -30000);

        if ((menuforced || menu_width == 1) && area_map_width == 2) // Menu + area map
            menu_w = 55;
        else if (menu_width == 2 && area_map_width == 2) // Area map only
        {
            menu_w = 24;
        }
        else if (menu_width == 1) // Wide menu
            menu_w = 55;
        else if (menuforced || (menu_width == 2 && area_map_width == 3)) // Menu only
            menu_w = 31; 

        return menu_w;
    }

    DEFINE_VMETHOD_INTERPOSE(void, feed, (std::set<df::interface_key> *input))
    {
        renderer_cool *r = (renderer_cool*)enabler->renderer;

        init->display.grid_x = r->gdimxfull + gmenu_w + 2;
        init->display.grid_y = r->gdimyfull + 2;

        INTERPOSE_NEXT(feed)(input);

        init->display.grid_x = tdimx;
        init->display.grid_y = tdimy;

        int menu_w_new = get_menu_width();
        if (gmenu_w != menu_w_new)
        {
            gmenu_w = menu_w_new;
            r->needs_reshape = true;
        }
    }

    DEFINE_VMETHOD_INTERPOSE(void, render, ())
    {
        //clock_t c1 = clock();
        INTERPOSE_NEXT(render)();

#ifdef WIN32
        static bool patched = false;
        if (!patched)
        {
            unsigned char t1[] = {  0x90,0x90, 0x90, 0x90,0x90,0x90,0x90,0x90 };
            Core::getInstance().p->patchMemory((void*)(0x0058eabd+(Core::getInstance().vinfo->getRebaseDelta())), t1, sizeof(t1));

            patched = true;
        }
#endif

        renderer_cool *r = (renderer_cool*)enabler->renderer;

        if (gmenu_w < 0)
        {
            gmenu_w = get_menu_width();
            r->needs_reshape = true;
        }

        r->handle_reshape_zoom_requests();       
        r->gswap_arrays();

#ifdef WIN32
        void (_stdcall *_render_map)(int) = (void (_stdcall *)(int))(0x008f65c0+(Core::getInstance().vinfo->getRebaseDelta()));
        #define render_map() _render_map(0)
#elif defined(__APPLE__)
        void (*_render_map)(void *, int) = (void (*)(void *, int))0x0084b4c0;
    #ifdef DFHACK_r5
        #define render_map() _render_map(df::global::map_renderer, 0)
    #else
        #define render_map() _render_map(df::global::cursor_unit_list, 0)
    #endif
#else
#endif

        // These values may change from the main thread while being accessed from the rendering thread,
        // and that will cause flickering of overridden tiles at least, so save them here
        gwindow_x = *df::global::window_x;
        gwindow_y = *df::global::window_y;
        gwindow_z = *df::global::window_z;

        long *z = (long*)gscreen;
        for (int y = 0; y < r->gdimy; y++)
        {
            for (int x = world->map.x_count-*df::global::window_x; x < r->gdimx; x++)
                z[x*r->gdimy+y] = 0;
        }
        for (int x = 0; x < r->gdimx; x++)
        {
            for (int y = world->map.y_count-*df::global::window_y; y < r->gdimy; y++)
                z[x*r->gdimy+y] = 0;
        }        

        uint8_t *sctop                     = enabler->renderer->screen;
        int32_t *screentexpostop           = enabler->renderer->screentexpos;
        int8_t *screentexpos_addcolortop   = enabler->renderer->screentexpos_addcolor;
        uint8_t *screentexpos_grayscaletop = enabler->renderer->screentexpos_grayscale;
        uint8_t *screentexpos_cftop        = enabler->renderer->screentexpos_cf;
        uint8_t *screentexpos_cbrtop       = enabler->renderer->screentexpos_cbr;

        // In fort mode render_map() will render starting at (1,1)
        // and will use dimensions from init->display.grid to calculate map region to render
        // but dimensions from gps to calculate offsets into screen buffer.
        // So we adjust all this so that it renders to our gdimx x gdimy buffer starting at (0,0).
        gps->screen                 = enabler->renderer->screen                 = gscreen - 4*r->gdimy - 4;
        gps->screen_limit           = gscreen + r->gdimx * r->gdimy * 4;
        gps->screentexpos           = enabler->renderer->screentexpos           = gscreentexpos           - r->gdimy - 1;
        gps->screentexpos_addcolor  = enabler->renderer->screentexpos_addcolor  = gscreentexpos_addcolor  - r->gdimy - 1;
        gps->screentexpos_grayscale = enabler->renderer->screentexpos_grayscale = gscreentexpos_grayscale - r->gdimy - 1;
        gps->screentexpos_cf        = enabler->renderer->screentexpos_cf        = gscreentexpos_cf        - r->gdimy - 1;
        gps->screentexpos_cbr       = enabler->renderer->screentexpos_cbr       = gscreentexpos_cbr       - r->gdimy - 1;

        init->display.grid_x = r->gdimx + gmenu_w + 2;
        init->display.grid_y = r->gdimy + 2;
        gps->dimx = r->gdimx;
        gps->dimy = r->gdimy;
        gps->clipx[1] = r->gdimx;
        gps->clipy[1] = r->gdimy;

        if (maxlevels && shadowsloaded)
            patch_rendering(false);
        
        render_map();
        
        if (maxlevels && shadowsloaded)
        {
            multi_rendered = false;

            gps->screen                 = mscreen - 4*r->gdimy - 4;
            gps->screen_limit           = mscreen + r->gdimx * r->gdimy * 4;
            gps->screentexpos           = mscreentexpos           - r->gdimy - 1;
            gps->screentexpos_addcolor  = mscreentexpos_addcolor  - r->gdimy - 1;
            gps->screentexpos_grayscale = mscreentexpos_grayscale - r->gdimy - 1;
            gps->screentexpos_cf        = mscreentexpos_cf        - r->gdimy - 1;
            gps->screentexpos_cbr       = mscreentexpos_cbr       - r->gdimy - 1;

            bool empty_tiles_left, rendered1st = false;
            int p = 1;
            int x0 = 0;
            int zz0 = *df::global::window_z; 
            int maxp = std::min(maxlevels, zz0);   

            do
            {
                if (p == maxlevels)
                    patch_rendering(true);

                (*df::global::window_z)--;

                if (p > 1)
                {
                    (*df::global::window_x) += x0;
                    init->display.grid_x -= x0-1;

                    render_map();

                    (*df::global::window_x) -= x0;
                    init->display.grid_x += x0-1;
                }

                empty_tiles_left = false;
                int x00 = x0;
                int zz = zz0 - p + 1;

                int x1 = std::min(r->gdimx, world->map.x_count-*df::global::window_x);
                int y1 = std::min(r->gdimy, world->map.y_count-*df::global::window_y);
                for (int x = x0; x < x1; x++)
                {
                    for (int y = 0; y < y1; y++)
                    {
                        const int tile = x * r->gdimy + y, stile = tile * 4;

#ifndef NO_RENDERING_PATCH
                        // Fast path. When rendering patch is available, tiles that are empty on the current level
                        // (and only them) will have symbol 00. We only also check for 31 which is a down ramp.

                        //if ((gscreen[stile+3]&0xf0))
                        //    continue;

                        unsigned char ch = gscreen[stile+0];
                        if (ch != 0 && ch != 31)
                            continue;

                        int xx = *df::global::window_x + x;
                        int yy = *df::global::window_y + y;
                        if (xx < 0 || yy < 0)
                            continue;

                        int xxquot = xx >> 4, xxrem = xx & 15;
                        int yyquot = yy >> 4, yyrem = yy & 15;                    

                        if (ch == 31)
                        {
                            //TODO: zz0 or zz ??
                            df::map_block *block0 = world->map.block_index[xxquot][yyquot][zz0];
                            if (block0->tiletype[xxrem][yyrem] != df::tiletype::RampTop || block0->designation[xxrem][yyrem].bits.flow_size)
                                continue;
                        }

                        if (p == 1 && !rendered1st)
                        {
                            multi_rendered = true;

                            (*df::global::window_x) += x0;
                            init->display.grid_x -= x0-1;

                            render_map();

                            (*df::global::window_x) -= x0;
                            init->display.grid_x += x0-1;

                            x00 = x0;

                            rendered1st = true;                        
                        }                    

                        const int tile2 = (x-(x00)) * r->gdimy + y, stile2 = tile2 * 4;                    

                        int d = p;
                        ch = mscreen[stile2+0];
                        if (p < maxp)
                        {
                            if (ch == 0)
                            {
                                empty_tiles_left = true;
                                continue;
                            }
                            else if (ch == 31)
                            {
                                df::map_block *block1 = world->map.block_index[xxquot][yyquot][zz-1];
                                df::tiletype t1 = block1->tiletype[xxrem][yyrem];
                                if (t1 == df::tiletype::RampTop && !block1->designation[xxrem][yyrem].bits.flow_size)
                                {
                                    empty_tiles_left = true;
                                    continue;
                                }
                            }
                        }

#else
                        // Slow path. Without rendering patch we have to check all symbols that the game
                        // may render for lower levels if a tile is empty on the current level.

                        if ((gscreen[stile+3]&0xf0))
                            continue;

                        unsigned char ch = gscreen[stile+0];
                        if (ch != 31 && ch != 249 && ch != 250 && ch != 254 && ch != skytile && ch != chasmtile && !(ch >= '1' && ch <= '7'))
                            continue;

                        int xx = *df::global::window_x + x;
                        int yy = *df::global::window_y + y;
                        if (xx < 0 || yy < 0)
                            continue;

                        int xxquot = xx >> 4, xxrem = xx & 15;
                        int yyquot = yy >> 4, yyrem = yy & 15;                    

                        //TODO: check for z=0 (?)
                        bool e0,h,h0;
                        //*out2 << xx << " " << world->map.x_count << " " << yy << " " << world->map.y_count << " " << *df::global::window_x << " " << *df::global::window_y << std::endl;
                        df::map_block *block0 = world->map.block_index[xxquot][yyquot][zz0];
                        h0 = block0 && block0->designation[xxrem][yyrem].bits.hidden;
                        if (h0)
                            continue;
                        e0 = !block0 || ((block0->tiletype[xxrem][yyrem] == df::tiletype::OpenSpace || block0->tiletype[xxrem][yyrem] == df::tiletype::RampTop) && !block0->designation[xxrem][yyrem].bits.flow_size);
                        if (!(e0))
                            continue;

                        if (p == 1 && !rendered1st)
                        {
                            (*df::global::window_x) += x0;
                            init->display.grid_x -= x0-1;

                            render_map();

                            (*df::global::window_x) -= x0;
                            init->display.grid_x += x0-1;

                            x00 = x0;

                            rendered1st = true;                        
                        }                    

                        const int tile2 = (x-(x00)) * r->gdimy + y, stile2 = tile2 * 4;                    

                        int d = p;
                        ch = mscreen[stile2+0];
                        if (!(ch!=31&&ch != 249 && ch != 250 && ch != 254 && ch != skytile && ch != chasmtile && !(ch >= '1' && ch <= '7')))
                        {
                            df::map_block *block1 = world->map.block_index[xxquot][yyquot][zz-1];
                            if (!block1)
                            {
                                //TODO: skip all other y's in this block
                                if (p < maxp)
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
                                    if (p < maxp)
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
    #endif

                        *((int*)gscreen + tile) = *((int*)mscreen + tile2);
                        if (*(mscreentexpos+tile2))
                        {
                            *(gscreentexpos + tile) = *(mscreentexpos + tile2);
                            *(gscreentexpos_addcolor + tile) = *(mscreentexpos_addcolor + tile2);
                            *(gscreentexpos_grayscale + tile) = *(mscreentexpos_grayscale + tile2);
                            *(gscreentexpos_cf + tile) = *(mscreentexpos_cf + tile2);
                            *(gscreentexpos_cbr + tile) = *(mscreentexpos_cbr + tile2);
                        }
                        gscreen[stile+3] = (0x10*d) | (gscreen[stile+3]&0x0f);
                    }

                    if (!empty_tiles_left)
                        x0 = x + 1;
                }

                if (!empty_tiles_left)
                    break;
                if (p++ >= maxp)
                    break;
            } while(empty_tiles_left);

            (*df::global::window_z) = zz0;
        }

        init->display.grid_x = gps->dimx = tdimx;
        init->display.grid_y = gps->dimy = tdimy;
        gps->clipx[1] = gps->dimx - 1;
        gps->clipy[1] = gps->dimy - 1;

        gps->screen = enabler->renderer->screen = sctop;
        gps->screen_limit = gps->screen + gps->dimx * gps->dimy * 4;
        gps->screentexpos = enabler->renderer->screentexpos = screentexpostop;
        gps->screentexpos_addcolor = enabler->renderer->screentexpos_addcolor = screentexpos_addcolortop;
        gps->screentexpos_grayscale = enabler->renderer->screentexpos_grayscale = screentexpos_grayscaletop;
        gps->screentexpos_cf = enabler->renderer->screentexpos_cf = screentexpos_cftop;
        gps->screentexpos_cbr = enabler->renderer->screentexpos_cbr = screentexpos_cbrtop;

        //clock_t c2 = clock();
        //*out2 << (c2-c1) << std::endl;
    }
};

IMPLEMENT_VMETHOD_INTERPOSE_PRIO(dwarfmode_hook, render, 200);
IMPLEMENT_VMETHOD_INTERPOSE(dwarfmode_hook, feed);
