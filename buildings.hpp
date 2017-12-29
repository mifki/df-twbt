#include "df/building_drawbuffer.h"

#include "df/building_animaltrapst.h"
#include "df/building_archerytargetst.h"
#include "df/building_armorstandst.h"
#include "df/building_axle_horizontalst.h"
#include "df/building_axle_verticalst.h"
#include "df/building_bars_floorst.h"
#include "df/building_bars_verticalst.h"
#include "df/building_bedst.h"
#include "df/building_bookcasest.h"
#include "df/building_boxst.h"
#include "df/building_bridgest.h"
#include "df/building_cabinetst.h"
#include "df/building_cagest.h"
#include "df/building_chainst.h"
#include "df/building_chairst.h"
#include "df/building_civzonest.h"
#include "df/building_coffinst.h"
#include "df/building_constructionst.h"
#include "df/building_doorst.h"
#include "df/building_farmplotst.h"
#include "df/building_floodgatest.h"
#include "df/building_furnacest.h"
#include "df/building_gear_assemblyst.h"
#include "df/building_grate_floorst.h"
#include "df/building_grate_wallst.h"
#include "df/building_hatchst.h"
#include "df/building_hivest.h"
#include "df/building_instrumentst.h"
#include "df/building_nest_boxst.h"
#include "df/building_nestst.h"
#include "df/building_road_dirtst.h"
#include "df/building_road_pavedst.h"
#include "df/building_rollersst.h"
#include "df/building_screw_pumpst.h"
#include "df/building_shopst.h"
#include "df/building_siegeenginest.h"
#include "df/building_slabst.h"
#include "df/building_statuest.h"
#include "df/building_stockpilest.h"
#include "df/building_supportst.h"
#include "df/building_tablest.h"
#include "df/building_traction_benchst.h"
#include "df/building_tradedepotst.h"
#include "df/building_trapst.h"
#include "df/building_wagonst.h"
#include "df/building_water_wheelst.h"
#include "df/building_weaponrackst.h"
#include "df/building_weaponst.h"
#include "df/building_wellst.h"
#include "df/building_windmillst.h"
#include "df/building_window_gemst.h"
#include "df/building_window_glassst.h"
#include "df/building_windowst.h"
#include "df/building_workshopst.h"

#define OVER1(cls) \
struct cls##_hook : public df::cls \
{ \
   typedef df::cls interpose_base; \
\
    DEFINE_VMETHOD_INTERPOSE(void, drawBuilding, (df::building_drawbuffer* dbuf, int16_t smth)) \
    { \
        INTERPOSE_NEXT(drawBuilding)(dbuf, smth); \
\
        renderer_cool *r = (renderer_cool*)enabler->renderer; \
\
        int xmax = std::min(dbuf->x2-gwindow_x, r->gdimx-1); \
        int ymax = std::min(dbuf->y2-gwindow_y, r->gdimy-1); \
\
        for (int x = dbuf->x1-gwindow_x; x <= xmax; x++) \
            for (int y = dbuf->y1-gwindow_y; y <= ymax; y++) \
                if (x >= 0 && y >= 0 && x < r->gdimx && y < r->gdimy) \
                    ((uint32_t*)screen_under_ptr)[x*r->gdimy + y] = ((uint32_t*)screen_ptr)[x*r->gdimy + y]; \
    } \
}; \
IMPLEMENT_VMETHOD_INTERPOSE(cls##_hook, drawBuilding);

#define OVER1_ENABLE(cls) INTERPOSE_HOOK(cls##_hook, drawBuilding).apply(true);

OVER1(building_animaltrapst);
OVER1(building_archerytargetst);
OVER1(building_armorstandst);
OVER1(building_axle_horizontalst);
OVER1(building_axle_verticalst);
OVER1(building_bars_floorst);
OVER1(building_bars_verticalst);
OVER1(building_bedst);
OVER1(building_bookcasest);
OVER1(building_boxst);
OVER1(building_cabinetst);
OVER1(building_cagest);
OVER1(building_chainst);
OVER1(building_chairst);
OVER1(building_coffinst);
OVER1(building_doorst);
OVER1(building_furnacest);
OVER1(building_gear_assemblyst);
OVER1(building_hatchst);
OVER1(building_hivest);
OVER1(building_instrumentst);
OVER1(building_nest_boxst);
OVER1(building_rollersst);
OVER1(building_screw_pumpst);
OVER1(building_siegeenginest);
OVER1(building_slabst);
OVER1(building_statuest);
OVER1(building_supportst);
OVER1(building_tablest);
OVER1(building_traction_benchst);
OVER1(building_tradedepotst);
OVER1(building_trapst);
OVER1(building_weaponrackst);
OVER1(building_weaponst);
OVER1(building_wellst);
OVER1(building_workshopst);

struct stockpile_hook : public df::building_stockpilest
{
   typedef df::building_stockpilest interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, drawBuilding, (df::building_drawbuffer* dbuf, int16_t smth))
    {
        if (!hide_stockpiles ||
            df::global::ui->main.mode == df::ui_sidebar_mode::QueryBuilding ||
            df::global::ui->main.mode == df::ui_sidebar_mode::LookAround ||
            df::global::ui->main.mode == df::ui_sidebar_mode::Stockpiles)
            INTERPOSE_NEXT(drawBuilding)(dbuf, smth);
        else
        {
            memset(dbuf->tile, 32, 31*31);
        }
    }
}; 
IMPLEMENT_VMETHOD_INTERPOSE(stockpile_hook, drawBuilding);

void enable_building_hooks()
{
    OVER1_ENABLE(building_animaltrapst);
    OVER1_ENABLE(building_archerytargetst);
    OVER1_ENABLE(building_armorstandst);
    OVER1_ENABLE(building_axle_horizontalst);
    OVER1_ENABLE(building_axle_verticalst);
    OVER1_ENABLE(building_bars_floorst);
    OVER1_ENABLE(building_bars_verticalst);
    OVER1_ENABLE(building_bedst);
    OVER1_ENABLE(building_bookcasest);
    OVER1_ENABLE(building_boxst);
    OVER1_ENABLE(building_cabinetst);
    OVER1_ENABLE(building_cagest);
    OVER1_ENABLE(building_chainst);
    OVER1_ENABLE(building_chairst);    
    OVER1_ENABLE(building_coffinst);
    OVER1_ENABLE(building_doorst);
    //OVER1_ENABLE(building_furnacest);
    OVER1_ENABLE(building_gear_assemblyst);
    OVER1_ENABLE(building_hatchst);
    OVER1_ENABLE(building_hivest);
    OVER1_ENABLE(building_instrumentst);
    OVER1_ENABLE(building_nest_boxst);
    OVER1_ENABLE(building_rollersst);
    OVER1_ENABLE(building_screw_pumpst);
    OVER1_ENABLE(building_siegeenginest);
    OVER1_ENABLE(building_slabst);
    OVER1_ENABLE(building_statuest);
    OVER1_ENABLE(building_supportst);
    OVER1_ENABLE(building_tablest);
    OVER1_ENABLE(building_traction_benchst);
    //OVER1_ENABLE(building_tradedepotst);
    OVER1_ENABLE(building_trapst);
    OVER1_ENABLE(building_weaponrackst);
    OVER1_ENABLE(building_weaponst);
    OVER1_ENABLE(building_wellst);
    //OVER1_ENABLE(building_workshopst);

    INTERPOSE_HOOK(stockpile_hook, drawBuilding).apply(true);
}