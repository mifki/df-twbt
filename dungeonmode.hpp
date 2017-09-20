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
            MemoryPatcher p(Core::getInstance().p);

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

        /*uint32_t *z = (uint32_t*)gscreen;
        for (int y = 0; y < r->gdimy; y++)
        {
            for (int x = world->map.x_count-*df::global::window_x; x < r->gdimx; x++)
            {
                z[x*r->gdimy+y] = 0;
                gscreentexpos[x*r->gdimy+y] = 0;
            }
        }
        for (int x = 0; x < r->gdimx; x++)
        {
            for (int y = world->map.y_count-*df::global::window_y; y < r->gdimy; y++)
            {
                z[x*r->gdimy+y] = 0;
                gscreentexpos[x*r->gdimy+y] = 0;
            }
        }*/

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
         
        // if (maxlevels)
            // (*df::global::window_z)+=1;

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
                        // And for adv. mode it's a bit more complicated.

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

                        if (ch == 0)
                        {
                            df::map_block *block0 = world->map.block_index[xxquot][yyquot][zz];
                            if (!block0->designation[xxrem][yyrem].bits.pile)
                            {
                                *((int*)gscreen+tile) = 0;
                                continue;
                            }
                        }

                        bool ramp = false;                        
                        if (ch == 31)
                        {
                            //TODO: zz0 or zz ??
                            df::map_block *block0 = world->map.block_index[xxquot][yyquot][zz];
                            if (block0->tiletype[xxrem][yyrem] != df::tiletype::RampTop || block0->designation[xxrem][yyrem].bits.flow_size || !block0->designation[xxrem][yyrem].bits.pile)
                                continue;
                        	ramp = true;                            
                        }
                        df::map_block *block0 = world->map.block_index[xxquot][yyquot][zz0];

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
                    	if (ramp && ch != 30 && ch != '@')
                    		continue;
                        if (p < maxp)
                        {
                            if (ch == 0)
                            {
                                df::map_block *block0 = world->map.block_index[xxquot][yyquot][zz-1];

                                if (block0->designation[xxrem][yyrem].bits.pile)
                                {
                                    empty_tiles_left = true;
                                    continue;
                                }
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

//                        #warning Adv. mode without rendering patch is not ready yet

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

                if (p++ >= maxp)
                    break;
            } while(empty_tiles_left);

            (*df::global::window_z) = zz0;

            //(*df::global::window_z)-=1;
        }
        //else
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
