static bool parse_int(std::string &str, int &ret, int base=10)
{
    char *e;
    ret = strtol(str.c_str(), &e, base);
    return (*e == 0);
}

static bool parse_float(std::string &str, float &ret)
{
    char *e;
    ret = strtod(str.c_str(), &e);
    return (*e == 0);
}

template <class T>
static bool parse_enum_or_int(std::string &str, int &ret, int def=-1)
{
    T val;

    if (str.length())
    {
        if (!parse_int(str, ret))
        {  
            if (find_enum_item(&val, str))
                ret = val;
            else
                return false;
        }
    }
    else
        ret = def;

    return true;
}

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


static bool load_text_font()
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
    
    //Text tileset - accessible at index 1
    if (!(small_font_path == gsmall_font_path && large_font_path == glarge_font_path))
    {
        // Load text font and set it as the main (and only) font in `init` structure
        load_tileset(small_font_path, (long *)init->font.small_font_texpos, 16, 16, (long*)&init->font.small_font_dispx, (long*)&init->font.small_font_dispy);
        if (large_font_path != small_font_path)
            load_tileset(large_font_path, (long *)init->font.large_font_texpos, 16, 16, (long*)&init->font.large_font_dispx, (long*)&init->font.large_font_dispy);
        else
        {
            memcpy(init->font.large_font_texpos, init->font.small_font_texpos, sizeof(init->font.large_font_texpos));
            init->font.large_font_dispx = init->font.small_font_dispx;
            init->font.large_font_dispy = init->font.small_font_dispy;
        }

        struct tileset ts;

        memcpy(ts.small_texpos, init->font.small_font_texpos, sizeof(ts.small_texpos));
        memcpy(ts.large_texpos, init->font.large_font_texpos, sizeof(ts.large_texpos));        

        tilesets.push_back(ts);
        return true;
    }
    else
    {
        tilesets.push_back(tilesets[0]);
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

        std::map<string, int> tilesetnames;
        tilesetnames["map"] = 0;
        tilesetnames["0"] = 0;
        tilesetnames["text"] = 1;
        tilesetnames["1"] = 1;

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
                if (tokens.size() == 3 || tokens.size() == 4)
                {
                    if (tokens.size() == 4 && tilesetnames.count(tokens[3]))
                    {
                        out2->color(COLOR_YELLOW);
                        *out2 << "TWBT: ignoring duplicate tileset with id " << tokens[3] << std::endl;
                        out2->color(COLOR_RESET);                                

                        continue;
                    }

                    struct tileset ts;
                    string small_font_path = "data/art/" + tokens[1];
                    string large_font_path = "data/art/" + tokens[2];

                    struct stat buf;
                    if (stat(small_font_path.c_str(), &buf) != 0)
                    {
                        out2->color(COLOR_YELLOW);
                        *out2 << "TWBT: " << small_font_path << " not found" << std::endl;
                        out2->color(COLOR_RESET);                                

                        continue;
                    }

                    long dx, dy;        
                    load_tileset(small_font_path, ts.small_texpos, 16, 16, &dx, &dy);

                    if (stat(large_font_path.c_str(), &buf) != 0)
                    {
                        out2->color(COLOR_YELLOW);
                        *out2 << "TWBT: " << small_font_path << " not found" << std::endl;
                        out2->color(COLOR_RESET);                                

                        large_font_path = small_font_path;
                    }

                    if (large_font_path != small_font_path)
                        load_tileset(large_font_path, ts.large_texpos, 16, 16, &dx, &dy);
                    else
                        memcpy(ts.large_texpos, ts.small_texpos, sizeof(ts.large_texpos));

                    tilesets.push_back(ts);

                    int idx = tilesets.size() - 1;
                    string n = (tokens.size() == 4) ?
                        tokens[3] :
                        static_cast<std::ostringstream*>(&(std::ostringstream() << idx))->str();
                    tilesetnames[n] = idx;
                }

                continue;
            }
            
            if (tokens[0] == "OVERRIDE")
            {
                if (tokens.size() == 8)
                {
                    if (!tilesetnames.count(tokens[6]))
                    {
                        out2->color(COLOR_YELLOW);
                        *out2 << "TWBT: no tileset with id " << tokens[6] << std::endl;
                        out2->color(COLOR_RESET);                                

                        continue;
                    }

                    int tile = atoi(tokens[1].c_str());
                    int tsidx = tilesetnames[tokens[6]];
                    int newtile = atoi(tokens[7].c_str());

                    struct override o;
                    o.kind = tokens[2][0];
                    if (o.kind == 'B')
                    {
                        if (!parse_enum_or_int<buildings_other_id::buildings_other_id>(tokens[3], o.id, buildings_other_id::IN_PLAY))
                            continue;
                        if (!parse_enum_or_int<building_type::building_type>(tokens[4], o.type))
                            continue;
                    }
                    else if (o.kind == 'I')
                    {
                        if (!parse_enum_or_int<items_other_id::items_other_id>(tokens[3], o.id, items_other_id::IN_PLAY))
                            continue;
                        if (!parse_enum_or_int<item_type::item_type>(tokens[4], o.type))
                            continue;
                    }
                    else
                        continue;

                    if (tokens[5].length() > 0)
                        o.subtype = atoi(tokens[5].c_str());
                    else
                        o.subtype = -1;

                    o.small_texpos = tilesets[tsidx].small_texpos[newtile];
                    o.large_texpos = tilesets[tsidx].large_texpos[newtile];

                    if (!overrides[tile])
                        overrides[tile] = new vector< struct override >;
                    overrides[tile]->push_back(o);
                }

                else if (tokens.size() == 6)
                {
                    if (!tilesetnames.count(tokens[4]))
                    {
                        out2->color(COLOR_YELLOW);
                        *out2 << "TWBT: no tileset with id " << tokens[4] << std::endl;
                        out2->color(COLOR_RESET);                                

                        continue;
                    }

                    int tile = atoi(tokens[1].c_str());
                    int tsidx = tilesetnames[tokens[4]];
                    int newtile = atoi(tokens[5].c_str());

                    struct override o;
                    o.kind = tokens[2][0];
                    if (o.kind == 'T')
                    {
                        std::string &typestr = tokens[3];
                        int ln = typestr.length();
                        if (!ln)
                            continue;

                        if (typestr[0] == '"' && typestr[ln-1] == '"')
                        {
                            std::string tn = typestr.substr(1, ln-2);

                            FOR_ENUM_ITEMS(tiletype, tt)
                            {
                                const char *ttn = tileName(tt);
                                if (ttn && tn == ttn)
                                {
                                    o.type = tt;
                                    break;
                                }
                            }
                        }
                        else if (!parse_enum_or_int<tiletype::tiletype>(tokens[3], o.type))
                            continue;
                    }
                    else
                        continue;

                    o.small_texpos = tilesets[tsidx].small_texpos[newtile];
                    o.large_texpos = tilesets[tsidx].large_texpos[newtile];

                    if (!overrides[tile])
                        overrides[tile] = new vector< struct override >;
                    overrides[tile]->push_back(o);
                }                

                else if (tokens.size() == 4)
                {
                    std::map<std::string,int>::iterator it;
                    if (!tilesetnames.count(tokens[2]))
                    {
                        out2->color(COLOR_YELLOW);
                        *out2 << "TWBT: no tileset with id " << tokens[2] << std::endl;
                        out2->color(COLOR_RESET);                                
                        continue;
                    }

                    int tile = atoi(tokens[1].c_str());
                    int tsidx = tilesetnames[tokens[2]];
                    int newtile = atoi(tokens[3].c_str());

                    tilesets[0].small_texpos[tile] = tilesets[tsidx].small_texpos[newtile];
                    tilesets[0].large_texpos[tile] = tilesets[tsidx].large_texpos[newtile];
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

            float val;
            if (!parse_float(tokens[1], val))
                continue;

            val /= 255.0f;

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