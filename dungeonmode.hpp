struct dungeonmode_hook : public df::viewscreen_dungeonmodest
{
    typedef df::viewscreen_dungeonmodest interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, feed, (std::set<df::interface_key> *input))
    {
        renderer_cool *r = (renderer_cool*)enabler->renderer;

        int oldgridx = init->display.grid_x;
        int oldgridy = init->display.grid_y;

        init->display.grid_x = r->gdimxfull;
        init->display.grid_y = r->gdimyfull;

		gmenu_w = 0;

        INTERPOSE_NEXT(feed)(input);

        init->display.grid_x = oldgridx;
        init->display.grid_y = oldgridy;
    }

    DEFINE_VMETHOD_INTERPOSE(void, logic, ())
    {
        renderer_cool *r = (renderer_cool*)enabler->renderer;

        int oldgridx = init->display.grid_x;
        int oldgridy = init->display.grid_y;

        init->display.grid_x = r->gdimxfull;
        init->display.grid_y = r->gdimyfull;

        INTERPOSE_NEXT(logic)();

        init->display.grid_x = tdimx;
        init->display.grid_y = tdimy;
	}    

    DEFINE_VMETHOD_INTERPOSE(void, render, ())
    {   
        screen_map_type = 2;

        renderer_cool *r = (renderer_cool*)enabler->renderer;
        r->reshape_zoom_swap();

        memset(gscreen_under, 0, r->gdimx*r->gdimy*sizeof(uint32_t));
        screen_under_ptr = gscreen_under;
        screen_ptr = gscreen;
        mwindow_x = gwindow_x;

        float wx = *df::global::window_x;
        float wy = *df::global::window_y;

        INTERPOSE_NEXT(render)();

        // It's a way around something
        *df::global::window_x = wx;
        *df::global::window_y = wy;

#ifdef WIN32
        static bool patched = false;
        if (!patched)
        {
            MemoryPatcher p(Core::getInstance().p.get());

            for (int j = 0; j < sizeof(p_advmode_render)/sizeof(patchdef); j++)
                apply_patch(&p, p_advmode_render[j]);

            patched = true;
        }
#endif    

        static bool tmode_old;
        int m = df::global::ui_advmode->menu;
        bool tmode = advmode_needs_map(m);
        if (tmode != tmode_old)
        {
        	tmode_old = tmode;
        	gps->force_full_display_count = 1;
        }
        if (!tmode)
        	return;

        // These values may change from the main thread while being accessed from the rendering thread,
        // and that will cause flickering of overridden tiles at least, so save them here
        gwindow_x = *df::global::window_x;
        gwindow_y = *df::global::window_y;
        gwindow_z = *df::global::window_z;                

        uint8_t *sctop = gps->screen;
        long *screentexpostop = gps->screentexpos;
        int8_t *screentexpos_addcolortop = gps->screentexpos_addcolor;
        uint8_t *screentexpos_grayscaletop = gps->screentexpos_grayscale;
        uint8_t *screentexpos_cftop = gps->screentexpos_cf;
        uint8_t *screentexpos_cbrtop = gps->screentexpos_cbr;

        gps->screen = gscreen;
        gps->screen_limit = gscreen + r->gdimx * r->gdimy * 4;
        gps->screentexpos = gscreentexpos;
        gps->screentexpos_addcolor = gscreentexpos_addcolor;
        gps->screentexpos_grayscale = gscreentexpos_grayscale;
        gps->screentexpos_cf = gscreentexpos_cf;
        gps->screentexpos_cbr = gscreentexpos_cbr;

        int oldgridx = init->display.grid_x;
        int oldgridy = init->display.grid_y;
        init->display.grid_x = r->gdimx;
        init->display.grid_y = r->gdimy+1;
        gps->dimx = r->gdimx;
        gps->dimy = r->gdimy;
        gps->clipx[1] = r->gdimx-1;
        gps->clipy[1] = r->gdimy;

        if (maxlevels)
            patch_rendering(false);
         
       	render_map();

        if (maxlevels)
        {
            multi_rendered = false;

            gps->screen = mscreen;
            gps->screen_limit = mscreen + r->gdimx * r->gdimy * 4;
            gps->screentexpos = mscreentexpos;
            gps->screentexpos_addcolor = mscreentexpos_addcolor;
            gps->screentexpos_grayscale = mscreentexpos_grayscale;
            gps->screentexpos_cf = mscreentexpos_cf;
            gps->screentexpos_cbr = mscreentexpos_cbr;

            memset(mscreen_under, 0, r->gdimx*r->gdimy*sizeof(uint32_t));
            screen_under_ptr = mscreen_under;
            screen_ptr = mscreen;

            bool lower_level_rendered = false;
            int p = 1;
            int x0 = 0;
            int zz0 = *df::global::window_z;  // Current "top" zlevel
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

                lower_level_rendered = false;
                int x00 = x0;
                int zz = zz0 - p + 1; // Last rendered zlevel in gscreen, the tiles of which we're checking below

                int x1 = std::min(r->gdimx, world->map.x_count-*df::global::window_x);
                int y1 = std::min(r->gdimy, world->map.y_count-*df::global::window_y);
                for (int x = x0; x < x1; x++)
                {
                    for (int y = 0; y < y1; y++)
                    {
                        const int tile = x * r->gdimy + y, stile = tile * 4;
                        unsigned char ch = gscreen[stile+0];

                        // Continue if the tile is not empty and doesn't look like a ramp
                        if (ch != 0 && ch != 31)
                            continue;

                        int xx = *df::global::window_x + x;
                        int yy = *df::global::window_y + y;
                        if (xx < 0 || yy < 0)
                            continue;

                        int xxquot = xx >> 4, xxrem = xx & 15;
                        int yyquot = yy >> 4, yyrem = yy & 15;

                        // If the tile looks like a ramp, check that it's really a ramp
                        // Also, no need to go deeper if the ramp is covered with water
                        if (ch == 31)
                        {
                            df::map_block *block0 = world->map.block_index[xxquot][yyquot][zz];
                            if (block0->tiletype[xxrem][yyrem] != df::tiletype::RampTop || block0->designation[xxrem][yyrem].bits.flow_size)
                                continue;
                        }

                        // If the tile is empty, render the next zlevel (if not rendered already)
                        if (!lower_level_rendered)
                        {
                            multi_rendered = true;

                            // All tiles to the left were not empty, so skip them
                            x0 = x;

                            (*df::global::window_x) += x0;
                            init->display.grid_x -= x0;
                            mwindow_x = gwindow_x + x0;

                            memset(mscreen_under, 0, (r->gdimx-x0)*r->gdimy*sizeof(uint32_t));
                            render_map();

                            (*df::global::window_x) -= x0;
                            init->display.grid_x += x0;

                            x00 = x0;

                            lower_level_rendered = true;
                        }

                        const int tile2 = (x-(x00)) * r->gdimy + y, stile2 = tile2 * 4;

                        *((uint32_t*)gscreen + tile) = *((uint32_t*)mscreen + tile2);
                        *((uint32_t*)gscreen_under + tile) = *((uint32_t*)mscreen_under + tile2);
                        if (*(mscreentexpos+tile2))
                        {
                            *(gscreentexpos + tile) = *(mscreentexpos + tile2);
                            *(gscreentexpos_addcolor + tile) = *(mscreentexpos_addcolor + tile2);
                            *(gscreentexpos_grayscale + tile) = *(mscreentexpos_grayscale + tile2);
                            *(gscreentexpos_cf + tile) = *(mscreentexpos_cf + tile2);
                            *(gscreentexpos_cbr + tile) = *(mscreentexpos_cbr + tile2);
                        }
                        gscreen[stile+3] = (0x10*p) | (gscreen[stile+3]&0x0f);
                    }
                }

                if (p++ >= maxp)
                    break;
            } while(lower_level_rendered);

            (*df::global::window_z) = zz0;
        }

        {
            gps->screen = gscreen;
            gps->screen_limit = gscreen + r->gdimx * r->gdimy * 4;
            gps->screentexpos = gscreentexpos;
            gps->screentexpos_addcolor = gscreentexpos_addcolor;
            gps->screentexpos_grayscale = gscreentexpos_grayscale;
            gps->screentexpos_cf = gscreentexpos_cf;
            gps->screentexpos_cbr = gscreentexpos_cbr;

            render_updown();
        }

        init->display.grid_x = gps->dimx = tdimx;
        init->display.grid_y = gps->dimy = tdimy;
        gps->clipx[1] = tdimx - 1;
        gps->clipy[1] = tdimy - 1;

        gps->screen = sctop;
        gps->screen_limit = gps->screen + tdimx*tdimy * 4;
        gps->screentexpos = screentexpostop;
        gps->screentexpos_addcolor = screentexpos_addcolortop;
        gps->screentexpos_grayscale = screentexpos_grayscaletop;
        gps->screentexpos_cf = screentexpos_cftop;
        gps->screentexpos_cbr = screentexpos_cbrtop;

        //clock_t c2 = clock();
        //*out2 << (c2-c1) << std::endl;
        //*out2<<"render_end"<<std::endl;

        if (block_index_size != world->map.x_count_block*world->map.y_count_block*world->map.z_count_block || (my_block_index && my_block_index[0] != world->map.block_index[0][0][0]))
        {
            free(my_block_index);
            block_index_size = world->map.x_count_block*world->map.y_count_block*world->map.z_count_block;
            my_block_index = (df::map_block**)malloc(block_index_size*sizeof(void*));

            for (int x = 0; x < world->map.x_count_block; x++)
            {
                for (int y = 0; y < world->map.y_count_block; y++)
                {
                    for (int z = 0; z < world->map.z_count_block; z++)
                    {
                        my_block_index[x*world->map.y_count_block*world->map.z_count_block + y*world->map.z_count_block + z] = world->map.block_index[x][y][z];
                    }                    
                }
            }
        }

        r->display_map();
    }
};

IMPLEMENT_VMETHOD_INTERPOSE(dungeonmode_hook, render);
IMPLEMENT_VMETHOD_INTERPOSE(dungeonmode_hook, logic);
IMPLEMENT_VMETHOD_INTERPOSE(dungeonmode_hook, feed);
