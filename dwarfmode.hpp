struct dwarfmode_hook : public df::viewscreen_dwarfmodest
{
    typedef df::viewscreen_dwarfmodest interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, feed, (std::set<df::interface_key> *input))
    {
        renderer_cool *r = (renderer_cool*)enabler->renderer;

        int oldgridx = init->display.grid_x;
        int oldgridy = init->display.grid_y;


        static uint8_t menu_width_last, area_map_width_last;
        static bool menuforced_last=0;

        int32_t w = gps->dimx, h = gps->dimy;
        uint8_t menu_width, area_map_width;
        Gui::getMenuWidth(menu_width, area_map_width);
        int32_t menu_w = 0;

        bool menuforced = (ui->main.mode != df::ui_sidebar_mode::Default || df::global::cursor->x != -30000);

        /*if (menu_width != menu_width_last || area_map_width != area_map_width_last || menuforced != menuforced_last)
        {
            needs_reshape = true;
            menu_width_last = menu_width;
            area_map_width_last = area_map_width;
            menuforced_last = menuforced;
        }*/

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


        init->display.grid_x = r->gdimxfull+menu_w+2;
        init->display.grid_y = r->gdimyfull+2;
        gps->dimx = r->gdimxfull+menu_w+2;
        gps->dimy = r->gdimyfull+2;


        INTERPOSE_NEXT(feed)(input);
        init->display.grid_x = gps->dimx = oldgridx;
        init->display.grid_y = gps->dimy = oldgridy;

        uint8_t menu_width_new, area_map_width_new;
        Gui::getMenuWidth(menu_width_new, area_map_width_new);
        bool menuforced_new = (ui->main.mode != df::ui_sidebar_mode::Default || df::global::cursor->x != -30000);
        if (menu_width != menu_width_new || area_map_width != area_map_width_new || menuforced != menuforced_new)
        {
            if ((menuforced_new || menu_width_new == 1) && area_map_width_new == 2) // Menu + area map
            {
                gmenu_w = 55;
            }
            else if (menu_width_new == 2 && area_map_width_new == 2) // Area map only
            {
                gmenu_w = 24;
            }
            else if (menu_width_new == 1) // Wide menu
                gmenu_w = 55;
            else if (menuforced_new || (menu_width_new == 2 && area_map_width_new == 3)) // Menu only
                gmenu_w = 31;
            else
                gmenu_w = 0;

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

        if (r->needs_reshape)
        {
            if (r->needs_zoom)
            {
                if (r->needs_zoom > 0)
                {
                    r->gdispx++;
                    r->gdispy++;
                }
                else
                {
                    r->gdispx--;
                    r->gdispy--;

                    if (r->gsize_x / r->gdispx > world->map.x_count)
                        r->gdispx = r->gdispy = r->gsize_x / world->map.x_count;
                    else if (r->gsize_y / r->gdispy > world->map.y_count)
                        r->gdispx = r->gdispy = r->gsize_y / world->map.y_count;
                }
                r->needs_zoom = 0;
            }
            r->needs_reshape = false;
            r->reshape_graphics();
        }

#ifdef WIN32
        void (*_render_map)(int) = (void (*)(int))(0x008f65c0+(Core::getInstance().vinfo->getRebaseDelta()));
        #define render_map() _render_map(1)
#elif defined(__APPLE__)
        void (*_render_map)(void *, int) = (void (*)(void *, int))0x0084b4c0;
    #ifdef DFHACK_r5
        #define render_map() _render_map(df::global::map_renderer, 1)
    #else
        #define render_map() _render_map(df::global::cursor_unit_list, 1)
    #endif
#else
#endif

        uint8_t *sctop = enabler->renderer->screen;
        int32_t *screentexpostop = enabler->renderer->screentexpos;
        int8_t *screentexpos_addcolortop = enabler->renderer->screentexpos_addcolor;
        uint8_t *screentexpos_grayscaletop = enabler->renderer->screentexpos_grayscale;
        uint8_t *screentexpos_cftop = enabler->renderer->screentexpos_cf;
        uint8_t *screentexpos_cbrtop = enabler->renderer->screentexpos_cbr;

        gps->screen = enabler->renderer->screen = gscreen;
        gps->screen_limit = gscreen + r->gdimx * r->gdimy * 4;
        gps->screentexpos = enabler->renderer->screentexpos = gscreentexpos;
        gps->screentexpos_addcolor = enabler->renderer->screentexpos_addcolor = gscreentexpos_addcolor;
        gps->screentexpos_grayscale = enabler->renderer->screentexpos_grayscale = gscreentexpos_grayscale;
        gps->screentexpos_cf = enabler->renderer->screentexpos_cf = gscreentexpos_cf;
        gps->screentexpos_cbr = enabler->renderer->screentexpos_cbr = gscreentexpos_cbr;

        long *z = (long*)r->screen;
        for (int y = 0; y < r->gdimy; y++)
        {
            for (int x = world->map.x_count-*df::global::window_x; x < r->gdimx; x++)
            {
                z[x*r->gdimy+y] = 0;
            }
        }
        for (int x = 0; x < r->gdimx; x++)
        {
            for (int y = world->map.y_count-*df::global::window_y; y < r->gdimy; y++)
            {
                z[x*r->gdimy+y] = 0;
            }
        }

        int oldgridx = init->display.grid_x;
        int oldgridy = init->display.grid_y;

        init->display.grid_x = r->gdimx;
        init->display.grid_y = r->gdimy;
        gps->dimx = r->gdimx;
        gps->dimy = r->gdimy;
        gps->clipx[1] = r->gdimx-1;
        gps->clipy[1] = r->gdimy-1;

        if (maxlevels && shadowsloaded)
            patch_rendering(false);

        render_map();

        if (maxlevels && shadowsloaded)
        {
            multi_rendered = false;

            gps->screen = mscreen;
            gps->screen_limit = mscreen + r->gdimx * r->gdimy * 4;
            gps->screentexpos = mscreentexpos;
            gps->screentexpos_addcolor = mscreentexpos_addcolor;
            gps->screentexpos_grayscale = mscreentexpos_grayscale;
            gps->screentexpos_cf = mscreentexpos_cf;
            gps->screentexpos_cbr = mscreentexpos_cbr;


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

                        bool e0,h,h0;
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

                        //*out2 << p << " !" << std::endl;
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

            //patch_rendering(false);




        }








        init->display.grid_x = gps->dimx = oldgridx;
        init->display.grid_y = gps->dimy = oldgridy;
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
