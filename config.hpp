static vector<string> split(const char *str, char c = ' ')
{
    vector<string> result;

    do
    {
        const char *begin = str;

        while(*str != c && *str)
            str++;

        result.push_back(string(begin, str));
    } while (0 != *str++);

    return result;
}

static bool get_font_paths()
{
    string small_font_path, gsmall_font_path;
    string large_font_path, glarge_font_path;

    std::ifstream fseed("data/init/init.txt");
    if(fseed.is_open())
    {
        string str;

        while(std::getline(fseed,str))
        {
            size_t b = str.find("[");
            size_t e = str.rfind("]");

            if (b == string::npos || e == string::npos || str.find_first_not_of(" ") < b)
                continue;

            str = str.substr(b+1, e-1);
            vector<string> tokens = split(str.c_str(), ':');

            if (tokens.size() != 2)
                continue;
                                
            if(tokens[0] == "FONT")
            {
                small_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "FULLFONT")
            {
                large_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "GRAPHICS_FONT")
            {
                gsmall_font_path = "data/art/" + tokens[1];
                continue;
            }

            if(tokens[0] == "GRAPHICS_FULLFONT")
            {
                glarge_font_path = "data/art/" + tokens[1];
                continue;
            }                    
        }
    }

    fseed.close();
    
    if (!(small_font_path == gsmall_font_path && large_font_path == glarge_font_path))
    {
        struct tileset ts;
        ts.small_font_path = small_font_path;
        ts.large_font_path = large_font_path;

        tilesets.push_back(ts);
        return true;
    }
    else
    {
        struct tileset ts;
        tilesets.push_back(ts);
        return false;
    }
}

static bool load_overrides()
{
    bool found = false;

    std::ifstream fseed("data/init/overrides.txt");
    if(fseed.is_open())
    {
        string str;

        while(std::getline(fseed,str))
        {
            size_t b = str.find("[");
            size_t e = str.rfind("]");

            if (b == string::npos || e == string::npos || str.find_first_not_of(" ") < b)
                continue;

            str = str.substr(b+1, e-1);
            vector<string> tokens = split(str.c_str(), ':');

            if (tokens[0] == "TILESET")
            {
                struct tileset ts;
                ts.small_font_path = "data/art/" + tokens[1];
                ts.large_font_path = "data/art/" + tokens[2];
                tilesets.push_back(ts);
                continue;
            }
            
            if (tokens[0] == "OVERRIDE")
            {
                if (tokens.size() == 8)
                {
                    int tile = atoi(tokens[1].c_str());

                    struct override o;
                    o.building = (tokens[2] == "B");
                    if (o.building)
                    {
                        buildings_other_id::buildings_other_id id;
                        if (find_enum_item(&id, tokens[3]))
                            o.id = id;
                        else
                            o.id = -1;

                        building_type::building_type type;
                        if (find_enum_item(&type, tokens[4]))
                            o.type = type;
                        else
                            o.type = -1;
                    }
                    else
                    {
                        items_other_id::items_other_id id;
                        if (find_enum_item(&id, tokens[3]))
                            o.id = id;
                        else
                            o.id = -1;

                        item_type::item_type type;
                        if (find_enum_item(&type, tokens[4]))
                            o.type = type;
                        else
                            o.type = -1;
                    }

                    if (tokens[5].length() > 0)
                        o.subtype = atoi(tokens[5].c_str());
                    else
                        o.subtype = -1;

                    o.newtile.tilesetidx = atoi(tokens[6].c_str());
                    o.newtile.tile = atoi(tokens[7].c_str());

                    if (!overrides[tile])
                        overrides[tile] = new vector< struct override >;
                    overrides[tile]->push_back(o);
                }
                else if (tokens.size() == 4)
                {
                    int tile = atoi(tokens[1].c_str());
                    override_defs[tile].tilesetidx = atoi(tokens[2].c_str());
                    override_defs[tile].tile = atoi(tokens[3].c_str());
                }

                found = true;
                continue;
            }
        }
    }

    fseed.close();
    return found;
}

static int color_name_to_index(std::string &name)
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    static const char *names[] = {
        "black", "blue", "green", "cyan", "red", "magenta", "brown", "lgray",
        "dgray", "lblue", "lgreen", "lcyan", "lred", "lmagenta", "yellow", "white"
    };

    for (int i = 0; i < 16; i++)
        if (name == names[i])
            return i;

    return -1;
}

static void load_colormap()
{
    string small_font_path, gsmall_font_path;
    string large_font_path, glarge_font_path;

    std::ifstream fseed("data/init/colors.txt");
    if(fseed.is_open())
    {
        string str;

        while(std::getline(fseed,str))
        {
            size_t b = str.find("[");
            size_t e = str.rfind("]");

            if (b == string::npos || e == string::npos || str.find_first_not_of(" ") < b)
                continue;

            str = str.substr(b+1, e-1);
            vector<string> tokens = split(str.c_str(), ':');
            if (tokens.size() != 2)
                continue;

            std::string &token0 = tokens[0];
            if (token0.length() < 3)
                continue;

            std::string name (token0.substr(0, token0.length()-2));
            int cidx = color_name_to_index(name);
            if (cidx == -1)
                continue;

            char *_e;
            float val = strtod(tokens[1].c_str(), &_e) / 255.0;
            if (*_e != 0)
                continue;

            const std::string &comp = token0.substr(token0.length()-2);
            if (comp == "_R")
                enabler->ccolor[cidx][0] = val;
            else if (comp == "_G")
                enabler->ccolor[cidx][1] = val;
            else if (comp == "_B")
                enabler->ccolor[cidx][2] = val;
        }
    }

    fseed.close();
}