command_result mapshot_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    if (!enabled)
        return CR_FAILURE;

//    CoreSuspender suspend;

    for (int z = 50; z < df::global::world->map.z_count; z++)
    {
        for (int bx = 0; bx < df::global::world->map.x_count_block; bx++)
        {
            for (int by = 0; by < df::global::world->map.y_count_block; by++)
            {
                df::map_block *block = world->map.block_index[bx][by][z];
                if (block)
                {
                    memset(block->fog_of_war, 0x00, 16*16);

                    for (int x = 0; x < 16; x++)
                    {
                        for (int y = 0; y < 16; y++)
                        {
                            block->designation[x][y].bits.hidden = false;
                            block->designation[x][y].bits.pile = true;
                        }
                    }                    
                }
            }
        }
    }    

    int z;
    for (z = 50; z < df::global::world->map.z_count; z++)
    {
        for (int bx = 0; bx < df::global::world->map.x_count_block; bx++)
        {
            for (int by = 0; by < df::global::world->map.y_count_block; by++)
            {
                df::map_block *block = world->map.block_index[bx][by][z];
                if (block)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        for (int y = 0; y < 16; y++)
                        {
                            /*df::tiletype tt = block->tiletype[x][y];
                            if (tt == df::tiletype::StoneWall || tt == df::tiletype::SoilWall || tt == df::tiletype::LavaWall
                                || tt == df::tiletype::FeatureWall || tt == df::tiletype::FrozenWall || tt == df::tiletype::MineralWall)
                                goto nextz;*/
                            if (block->designation[x][y].bits.subterranean)
                                goto nextz;
                        }
                    }
                }
            }            
        }

        break;

        nextz:;
    }
    done:;
*out2 << "ZZ " << z << std::endl;
    df::global::world->units.active[0]->pos.z = z;

    domapshot = 10;

    return CR_OK;    
}

command_result multilevel_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    if (!enabled)
        return CR_FAILURE;

    CoreSuspender suspend;

    int pcnt = parameters.size();

    if (pcnt >= 1)
    {
        std::string &param0 = parameters[0];
        int newmaxlevels = maxlevels;

        if (param0 == "shadowcolor" && pcnt >= 5)
        {
            float c[4];
            bool ok = parse_float(parameters[1], c[0]) &&
                      parse_float(parameters[2], c[1]) &&
                      parse_float(parameters[3], c[2]) &&
                      parse_float(parameters[4], c[3]);

            if (ok)
                memcpy(shadowcolor, c, sizeof(shadowcolor));
            else
                return CR_WRONG_USAGE;
        }        

        else if (param0 == "fogcolor" && pcnt >= 4)
        {
            float c[3];
            bool ok = parse_float(parameters[1], c[0]) &&
                      parse_float(parameters[2], c[1]) &&
                      parse_float(parameters[3], c[2]);

            if (ok)
                memcpy(fogcolor, c, sizeof(fogcolor));
            else
                return CR_WRONG_USAGE;
        }

        else if (param0 == "fogdensity" && pcnt >= 2)
        {
            float val[3];
            bool ok;

            ok = parse_float(parameters[1], val[0]);
            if (pcnt >= 3)
                ok &= parse_float(parameters[2], val[1]);
            if (pcnt >= 4)
                ok &= parse_float(parameters[3], val[2]);

            if (!ok)
                return CR_WRONG_USAGE;                

            fogdensity = val[0];
            if (pcnt >= 3)
                fogstart = val[1];
            else
                fogstart = 0;
            if (pcnt >= 4)
                fogstep = val[2];
            else
                fogstep = 1;

            ((renderer_cool*)enabler->renderer)->needs_full_update = true;
        }

        else if (param0 == "more")
        {
            if (maxlevels < 15)
                newmaxlevels = maxlevels + 1;
        }
        else if (param0 == "less")
        {
            if (maxlevels > 0)
                newmaxlevels = maxlevels - 1;
        }        
        else 
        {
            int val;
            if (parse_int(param0, val))
                newmaxlevels = std::max (std::min(val, 15), 0);
            else
                return CR_WRONG_USAGE;
        }

        if (newmaxlevels && !maxlevels)
        {
            if (!legacy_mode)
                patch_rendering(false);

            ((renderer_cool*)enabler->renderer)->needs_full_update = true;
        }
        else if (!newmaxlevels && maxlevels)
        {
            if (!legacy_mode)
                patch_rendering(true);

            multi_rendered = false;

            ((renderer_cool*)enabler->renderer)->needs_full_update = true;
        }

        maxlevels = newmaxlevels;            
    }
    else
        return CR_WRONG_USAGE;

    return CR_OK;    
}

command_result twbt_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    if (!enabled)
        return CR_FAILURE;

    CoreSuspender suspend;

    int pcnt = parameters.size();

    if (pcnt >= 1)
    {
        std::string &param0 = parameters[0];

        if (param0 == "tilesize")
        {
            renderer_cool *r = (renderer_cool*) enabler->renderer;
            std::string &param2 = parameters[1];

            if (pcnt == 1)
            {
                *out2 << "Map tile size is " << r->gdispx << "x" << r->gdispy << std::endl;
                return CR_OK;
            }

            if (parameters[1] == "bigger")
            {
                r->gdispx++;
                r->gdispy++;

                r->needs_reshape = true;
            }
            else if (parameters[1] == "smaller")
            {
                if (r->gdispx > 1 && r->gdispy > 1 && (r->gdimxfull < world->map.x_count || r->gdimyfull < world->map.y_count))
                {
                    r->gdispx--;
                    r->gdispy--;

                    r->needs_reshape = true;
                }
            }

            else if (parameters[1] == "reset")
            {
                r->gdispx = enabler->fullscreen ? small_map_dispx : large_map_dispx;
                r->gdispy = enabler->fullscreen ? small_map_dispy : large_map_dispy;

                r->needs_reshape = true;                    
            }

            else if (parameters[1][0] == '+' || parameters[1][0] == '-')
            {
                int delta;
                if (!parse_int(parameters[1], delta))
                    return CR_WRONG_USAGE;

                r->gdispx += delta;
                r->gdispy += delta;

                r->needs_reshape = true;
            }

            else if (pcnt >= 3)
            {
                int w, h;
                bool ok = parse_int(parameters[1], w) &&
                          parse_int(parameters[2], h);

                if (ok)
                {
                    r->gdispx = w;
                    r->gdispy = h;
                    r->needs_reshape = true;
                }
                else
                    return CR_WRONG_USAGE;   

            }
            else
                return CR_WRONG_USAGE;
        }
    }

    return CR_OK;    
}

command_result colormap_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    if (!enabled)
        return CR_FAILURE;

    CoreSuspender suspend;

    int pcnt = parameters.size();

    if (pcnt == 1)
    {
        std::string &param0 = parameters[0];
        if (param0 == "reload")
        {
            load_colormap();
        }
        else
        {
            int cidx = color_name_to_index(param0);

            if (cidx != -1)
                out << param0 << " = " <<
                    roundf(enabler->ccolor[cidx][0] * 255) << " " <<
                    roundf(enabler->ccolor[cidx][1] * 255) << " " <<
                    roundf(enabler->ccolor[cidx][2] * 255) << std::endl;
        }

        gps->force_full_display_count = 1;
        ((renderer_cool*)enabler->renderer)->needs_full_update = true;
    }
    else if (pcnt == 4)
    {
        int cidx = color_name_to_index(parameters[0]);

        if (cidx != -1)
        {
            float c[3];
            bool ok = parse_float(parameters[1], c[0]) &&
                      parse_float(parameters[2], c[1]) &&
                      parse_float(parameters[3], c[2]);

            if (ok)
            {
                c[0] /= 255.0f, c[1] /= 255.0f, c[2] /= 255.0f;
                memcpy(enabler->ccolor[cidx], c, sizeof(enabler->ccolor[cidx]));

                gps->force_full_display_count = 1;
                ((renderer_cool*)enabler->renderer)->needs_full_update = true; 
            }
            else
                return CR_WRONG_USAGE;            
        }  
    }

    return CR_OK;    
}

typedef void (*TTT)(void *world, short rx, short ry, short ex, short ey, int unk1, int unk2, void *unk3);
typedef void (*QQQ)(void *world);

int _rx;
int _ry;
int _ex;
int _ey;

command_result ttt_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    TTT ttt = (TTT)0xd43a10;

    int kk[] = { 8, -1, 0, -1, -1 };
    int kk2[] = { 0, -1, 0, -1, 0x18 };

    int rx;
    int ry;
    int ex;
    int ey;
    if (parameters.size())
    {
        rx = atoi(parameters[0].c_str());
        ry = atoi(parameters[1].c_str());
        ex = atoi(parameters[2].c_str());
        ey = atoi(parameters[3].c_str());
    }
    else
    {
        rx = _rx;
        ry = _ry;
        ex = _ex;
        ey = _ey;
    }

    MemoryPatcher p(Core::getInstance().p);
    p.makeWritable((void*)0xd0681b, 5);
    memset((void*)0xd06817, 0x90, 5);

    ttt(df::global::world, rx,ry,ex,ey,   1, 0,   kk);

    //*df::global::window_x = 144/3*ex+1;
    //*df::global::window_y = 144/3*ey+1;
out << df::global::world->units.active[0]->pos.x << std::endl;
out << df::global::world->units.active[0]->pos.y << std::endl;
out << df::global::world->units.active[0]->pos.z << std::endl;
df::global::world->units.active[0]->pos.x = 10;
df::global::world->units.active[0]->pos.y = 10;
df::global::world->world_data->unk_x1 = 0;
df::global::world->world_data->unk_x2 = 144;
df::global::world->world_data->unk_y1 = 0;
df::global::world->world_data->unk_y2 = 144;
    int z;
    for (z = 50; z < df::global::world->map.z_count; z++)
    {
        for (int bx = 0; bx < df::global::world->map.x_count_block; bx++)
        {
            for (int by = 0; by < df::global::world->map.y_count_block; by++)
            {
                df::map_block *block = world->map.block_index[bx][by][z];
                if (block)
                {
                    for (int x = 0; x < 16; x++)
                    {
                        for (int y = 0; y < 16; y++)
                        {
                            /*df::tiletype tt = block->tiletype[x][y];
                            if (tt == df::tiletype::StoneWall || tt == df::tiletype::SoilWall || tt == df::tiletype::LavaWall
                                || tt == df::tiletype::FeatureWall || tt == df::tiletype::FrozenWall || tt == df::tiletype::MineralWall)
                                goto nextz;*/
                            if (block->designation[x][y].bits.subterranean)
                                goto nextz;
                        }
                    }
                }
            }            
        }

        break;

        nextz:;
    }
    done:;
*out2 << "ZZ " << z << std::endl;
    *df::global::window_z = z;

    return CR_OK;
}

command_result qqq_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    //QQQ qqq = (QQQ)0x34edd0;
    QQQ qqq = (QQQ)0xd454f0;
    //QQQ qqq = (QQQ)0xd42e40;

    qqq(df::global::world);
    return CR_OK;
}

command_result www_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    CoreSuspender suspend;
    qqq_cmd(out, parameters);
    ttt_cmd(out, parameters);
    df::viewscreen *ws = Gui::getCurViewscreen();
    for (int i = 0; i < 3; i++)
    {
        ws->logic();
        ws->render();
    }
    mapshot_cmd(out, parameters);
    return CR_OK;
}

typedef void (*PPP)(void *world, int unload, void *coords);

command_result ppp_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    std::vector <std::string> q;
    int x1 = atoi(parameters[0].c_str());
    int x2 = atoi(parameters[1].c_str());
    int y1 = atoi(parameters[2].c_str());
    int y2 = atoi(parameters[3].c_str());

    x1 /= 3;
    x1 *= 3;
    y1 /= 3;
    y1 *= 3;

    for (int y = y1; y <= y2; y+=3)
    {
        for (int x = x1; x <= x2; x+=3)
        {
            _rx = x / 16;
            _ry = y / 16;
            _ex = x % 16+1;
            _ey = y % 16+1;
    
            www_cmd(out, q);
            while(domapshot);
        }        
    }

    return CR_OK;

    while(df::global::world->map.region_x < 500){
    {
    CoreSuspender suspend;

 /*   PPP ppp = (PPP)0xd42e40;


    int x1 = atoi(parameters[0].c_str());
    int x2 = atoi(parameters[1].c_str());
    int y1 = atoi(parameters[2].c_str());
    int y2 = atoi(parameters[3].c_str());

    int kk[] = { x1, x2, y1, y2 };

    *out2 << "ppp 1" << std::endl;
    //ppp(df::global::world, 0, 0);
    *out2 << "ppp 2" << std::endl;
    ppp(df::global::world, 1, kk);
    *out2 << "ppp 3" << std::endl;

    *out2 << df::global::world->map.region_x << std::endl;*/

    *out2 << df::global::world->map.region_x << std::endl;
    df::global::world->units.active[0]->pos.x = 150;
    df::viewscreen *ws = Gui::getCurViewscreen();
    for (int i = 0; i < 3; i++)
    {
        ws->logic();
        ws->render();
    }
    *out2 << df::global::world->map.region_x << std::endl;
    mapshot_cmd(out, parameters);
}
    while(domapshot);

    *out2 << "done" << std::endl;
}
    return CR_OK;
}