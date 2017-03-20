#include "df/building_drawbuffer.h"

#include "df/building_armorstandst.h"
#include "df/building_bedst.h"
#include "df/building_cabinetst.h"
#include "df/building_slabst.h"
#include "df/building_coffinst.h"
#include "df/building_statuest.h"
#include "df/building_trapst.h"
#include "df/building_tablest.h"
#include "df/building_chairst.h"
#include "df/building_weaponrackst.h"
#include "df/building_traction_benchst.h"
#include "df/building_workshopst.h"
#include "df/building_furnacest.h"
#include "df/building_cagest.h"
#include "df/building_chainst.h"
#include "df/building_bookcasest.h"
#include "df/building_hivest.h"
#include "df/building_rollersst.h"
#include "df/building_animaltrapst.h"
#include "df/building_archerytargetst.h"
#include "df/building_wellst.h"
#include "df/building_siegeenginest.h"
#include "df/building_bars_floorst.h"
#include "df/building_bars_verticalst.h"

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
        for (int x = dbuf->x1-gwindow_x; x <= dbuf->x2-gwindow_x; x++) \
            for (int y = dbuf->y1-gwindow_y; y <= dbuf->y2-gwindow_y; y++) \
                ((uint32_t*)screen_under_ptr)[x*r->gdimy + y] = ((uint32_t*)screen_ptr)[x*r->gdimy + y]; \
    } \
}; \
IMPLEMENT_VMETHOD_INTERPOSE(cls##_hook, drawBuilding);

#define OVER1_ENABLE(cls) INTERPOSE_HOOK(cls##_hook, drawBuilding).apply(true);

OVER1(building_armorstandst);
OVER1(building_bedst);
OVER1(building_cabinetst);
OVER1(building_slabst);
OVER1(building_coffinst);
OVER1(building_statuest);
OVER1(building_trapst);
OVER1(building_tablest);
OVER1(building_chairst);
OVER1(building_weaponrackst);
OVER1(building_traction_benchst);
OVER1(building_workshopst);
OVER1(building_furnacest);
OVER1(building_cagest);
OVER1(building_chainst);
OVER1(building_bookcasest);
OVER1(building_hivest);
OVER1(building_rollersst);
OVER1(building_animaltrapst);
OVER1(building_archerytargetst);
OVER1(building_wellst);
OVER1(building_siegeenginest);
OVER1(building_bars_floorst);
OVER1(building_bars_verticalst);

void enable_building_hooks()
{
    OVER1_ENABLE(building_armorstandst);
    OVER1_ENABLE(building_bedst);
    OVER1_ENABLE(building_cabinetst);
    OVER1_ENABLE(building_slabst);
    OVER1_ENABLE(building_coffinst);
    OVER1_ENABLE(building_statuest);
    OVER1_ENABLE(building_trapst);
    OVER1_ENABLE(building_tablest);
    OVER1_ENABLE(building_chairst);    
    OVER1_ENABLE(building_weaponrackst);        
    OVER1_ENABLE(building_traction_benchst);
    OVER1_ENABLE(building_workshopst);
    OVER1_ENABLE(building_furnacest);
    OVER1_ENABLE(building_cagest);
    OVER1_ENABLE(building_chainst);
    OVER1_ENABLE(building_bookcasest);
    OVER1_ENABLE(building_hivest);
    OVER1_ENABLE(building_rollersst);
    OVER1_ENABLE(building_animaltrapst);
    OVER1_ENABLE(building_archerytargetst);
    OVER1_ENABLE(building_wellst);
    OVER1_ENABLE(building_siegeenginest);
    OVER1_ENABLE(building_bars_floorst);
    OVER1_ENABLE(building_bars_verticalst);
}