command_result mapshot_cmd (color_ostream &out, std::vector <std::string> & parameters)
{
    if (!enabled)
        return CR_FAILURE;

    CoreSuspender suspend;

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
            float val;
            if (parse_float(parameters[1], val))
                fogdensity = val;
            else
                return CR_WRONG_USAGE;
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
            patch_rendering(false);

            ((renderer_cool*)enabler->renderer)->needs_full_update = true;
        }
        else if (!newmaxlevels && maxlevels)
        {
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

        if (param0 == "tilesize" && pcnt >= 2)
        {
            renderer_cool *r = (renderer_cool*) enabler->renderer;
            std::string &param2 = parameters[1];

            if (parameters[1] == "bigger")
            {
                r->gdispx++;
                r->gdispy++;
                r->needs_reshape = true;
            }
            else if (parameters[1] == "smaller")
            {
                if (r->gdispx > 0 && r->gdispy > 0)
                {
                    r->gdispx--;
                    r->gdispy--;
                    r->needs_reshape = true;
                }
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