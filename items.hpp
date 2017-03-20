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
#include "df/item_pantsst.h"
#include "df/item_shieldst.h"

#include "df/item_traction_benchst.h"
#include "df/item_toyst.h"
#include "df/item_bucketst.h"
#include "df/item_anvilst.h"
#include "df/item_toolst.h"
#include "df/item_trapcompst.h"
#include "df/item_trappartsst.h"
#include "df/item_animaltrapst.h"
#include "df/item_backpackst.h"
#include "df/item_barst.h"
#include "df/item_weaponrackst.h"
#include "df/item_braceletst.h"
#include "df/item_clothst.h"
#include "df/item_corpsest.h"
#include "df/item_corpsepiecest.h"
#include "df/item_figurinest.h"
#include "df/item_earringst.h"
#include "df/item_gemst.h"
#include "df/item_floodgatest.h"
#include "df/item_gratest.h"
#include "df/item_millstonest.h"
#include "df/item_chainst.h"
#include "df/item_sheetst.h"
#include "df/item_instrumentst.h"
#include "df/item_scepterst.h"
#include "df/item_roughst.h"
#include "df/item_remainsst.h"

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
OVER2(item_pantsst);
OVER2(item_shieldst);

OVER2(item_traction_benchst);
OVER2(item_toyst);
OVER2(item_bucketst);
OVER2(item_anvilst);
OVER2(item_toolst);
OVER2(item_trapcompst);
OVER2(item_trappartsst);
OVER2(item_animaltrapst);
OVER2(item_backpackst);
OVER2(item_barst);
OVER2(item_weaponrackst);
OVER2(item_braceletst);
OVER2(item_clothst);
OVER2(item_corpsest);
OVER2(item_corpsepiecest);
OVER2(item_figurinest);
OVER2(item_earringst);
OVER2(item_gemst);
OVER2(item_floodgatest);
OVER2(item_gratest);
OVER2(item_millstonest);
OVER2(item_chainst);
OVER2(item_sheetst);
OVER2(item_instrumentst);
OVER2(item_scepterst);
OVER2(item_roughst);
OVER2(item_remainsst);

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
    OVER2_ENABLE(item_pantsst);
    OVER2_ENABLE(item_shieldst);

    OVER2_ENABLE(item_traction_benchst);
    OVER2_ENABLE(item_toyst);
    OVER2_ENABLE(item_bucketst);
    OVER2_ENABLE(item_anvilst);
    OVER2_ENABLE(item_toolst);
    OVER2_ENABLE(item_trapcompst);
    OVER2_ENABLE(item_trappartsst);
    OVER2_ENABLE(item_animaltrapst);
    OVER2_ENABLE(item_backpackst);
    OVER2_ENABLE(item_barst);
    OVER2_ENABLE(item_weaponrackst);
    OVER2_ENABLE(item_braceletst);
    OVER2_ENABLE(item_clothst);
    OVER2_ENABLE(item_corpsest);
    OVER2_ENABLE(item_corpsepiecest);
    OVER2_ENABLE(item_figurinest);
    OVER2_ENABLE(item_earringst);
    OVER2_ENABLE(item_gemst);
    OVER2_ENABLE(item_floodgatest);
    OVER2_ENABLE(item_gratest);
    OVER2_ENABLE(item_millstonest);
    OVER2_ENABLE(item_chainst);
    OVER2_ENABLE(item_sheetst);
    OVER2_ENABLE(item_instrumentst);
    OVER2_ENABLE(item_scepterst);
    OVER2_ENABLE(item_roughst);
    OVER2_ENABLE(item_remainsst);
}
