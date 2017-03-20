#include "df/item_armorstandst.h"
#include "df/item_bedst.h"
#include "df/item_cabinetst.h"
#include "df/item_slabst.h"
#include "df/item_coffinst.h"
#include "df/item_statuest.h"
#include "df/item_woodst.h"
#include "df/item_barrelst.h"
#include "df/item_binst.h"
#include "df/item_tablest.h"
#include "df/item_chairst.h"
#include "df/item_boulderst.h"
#include "df/item_rockst.h"
#include "df/item_gobletst.h"

#include "df/item_shoesst.h"
#include "df/item_glovesst.h"
#include "df/item_armorst.h"
#include "df/item_helmst.h"

#include "df/item_traction_benchst.h"
#include "df/item_toyst.h"
#include "df/item_bucketst.h"
#include "df/item_anvilst.h"
#include "df/item_toolst.h"

#define OVER2(cls) \
struct cls##_hook : public df::cls \
{ \
   typedef df::cls interpose_base; \
\
    DEFINE_VMETHOD_INTERPOSE(uint8_t, drawSelf, ()) \
    { \
        renderer_cool *r = (renderer_cool*)enabler->renderer; \
\
        df::coord _pos = Items::getPosition(this); \
        if (!screen_under_ptr[(_pos.x-gwindow_x)*r->gdimy + _pos.y-gwindow_y]) \
            ((uint32_t*)screen_under_ptr)[(_pos.x-gwindow_x)*r->gdimy + _pos.y-gwindow_y] = ((uint32_t*)screen_ptr)[(_pos.x-gwindow_x)*r->gdimy + _pos.y-gwindow_y]; \
\
        return INTERPOSE_NEXT(drawSelf)(); \
\
    } \
}; \
IMPLEMENT_VMETHOD_INTERPOSE(cls##_hook, drawSelf);

#define OVER2_ENABLE(cls) INTERPOSE_HOOK(cls##_hook, drawSelf).apply(true);

OVER2(item_armorstandst);
OVER2(item_bedst);
OVER2(item_cabinetst);
OVER2(item_slabst);
OVER2(item_coffinst);
OVER2(item_statuest);
OVER2(item_woodst);
OVER2(item_barrelst);
OVER2(item_binst);
OVER2(item_chairst);
OVER2(item_tablest);
OVER2(item_boulderst);
OVER2(item_rockst);
OVER2(item_gobletst);

OVER2(item_shoesst);
OVER2(item_glovesst);
OVER2(item_armorst);
OVER2(item_helmst);

OVER2(item_traction_benchst);
OVER2(item_toyst);
OVER2(item_bucketst);
OVER2(item_anvilst);
OVER2(item_toolst);

void enable_item_hooks()
{
    OVER2_ENABLE(item_armorstandst);
    OVER2_ENABLE(item_bedst);
    OVER2_ENABLE(item_cabinetst);
    OVER2_ENABLE(item_slabst);
    OVER2_ENABLE(item_coffinst);
    OVER2_ENABLE(item_statuest);
    OVER2_ENABLE(item_woodst);
    OVER2_ENABLE(item_barrelst);
    OVER2_ENABLE(item_binst);
    OVER2_ENABLE(item_chairst);
    OVER2_ENABLE(item_tablest);  
    OVER2_ENABLE(item_boulderst);    
    OVER2_ENABLE(item_rockst);        
    OVER2_ENABLE(item_gobletst);        

    OVER2_ENABLE(item_shoesst);
    OVER2_ENABLE(item_glovesst);        
    OVER2_ENABLE(item_armorst);        
    OVER2_ENABLE(item_helmst);        

    OVER2_ENABLE(item_traction_benchst);
    OVER2_ENABLE(item_toyst);
    OVER2_ENABLE(item_bucketst);
    OVER2_ENABLE(item_anvilst);
    OVER2_ENABLE(item_toolst);
}
