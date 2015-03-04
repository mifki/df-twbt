#include <df/building_doorst.h>

static long building_doorst_tiles[4];
static bool building_doorst_loaded;
static unsigned int rot;

struct building_doorst_twbt : public df::building_doorst
{
    typedef df::building_doorst interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, drawBuilding, (df::building_drawbuffer* dbuf, int16_t smth))
    {
        if (!building_doorst_loaded)
        {
            long dx, dy;        
            load_tileset("data/art/tiles/door.png", building_doorst_tiles, 4, 1, &dx, &dy);
            building_doorst_loaded = true;
        }

        INTERPOSE_NEXT(drawBuilding)(dbuf, smth);

        rot = df::global::enabler->gputicks.value / 10;
        dbuf->tile[0][0] = 0xfe + (rot%2);
        gscreen_over[dbuf->x1*256+dbuf->y1] = building_doorst_tiles[0+rot%4];
        //*out2 << (int)dbuf->x1 << " "  << (int)dbuf->y1 << " " << (int)dbuf->x2 << " " << gscreen_over[dbuf->x1*256+dbuf->y1] << std::endl;

        rot++;
    }
};

IMPLEMENT_VMETHOD_INTERPOSE(building_doorst_twbt, drawBuilding);
