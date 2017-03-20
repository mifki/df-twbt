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
}