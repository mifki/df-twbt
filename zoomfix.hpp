static void fix_zoom()
{
    if (df::global::cursor->x == -30000)
        return;

    renderer_cool *r = (renderer_cool*)enabler->renderer;
    *df::global::window_x = std::max(0, std::min(world->map.x_count - r->gdimxfull, df::global::cursor->x - r->gdimx / 2));
    *df::global::window_y = std::max(0, std::min(world->map.y_count - r->gdimyfull, df::global::cursor->y - r->gdimy / 2));    
}

struct viewscreen_unitlistst_zoomfix : public df::viewscreen_unitlistst
{
    typedef df::viewscreen_unitlistst interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, feed, (std::set<df::interface_key> *input))
    {
        int oldwx = *df::global::window_x, oldwy = *df::global::window_y;
        bool zooming = false;
        if (input->count(interface_key::UNITJOB_ZOOM_CRE) || input->count(interface_key::UNITJOB_ZOOM_BUILD))
            zooming = true;        

        INTERPOSE_NEXT(feed)(input);        

        if (zooming && (*df::global::window_x != oldwx || *df::global::window_y != oldwy))
            fix_zoom();
    }    
};

IMPLEMENT_VMETHOD_INTERPOSE(viewscreen_unitlistst_zoomfix, feed);


struct viewscreen_buildinglistst_zoomfix : public df::viewscreen_buildinglistst
{
    typedef df::viewscreen_buildinglistst interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, feed, (std::set<df::interface_key> *input))
    {
        int oldwx = *df::global::window_x, oldwy = *df::global::window_y;
        bool zooming = false;
        if (input->count(interface_key::BUILDINGLIST_ZOOM_T) || input->count(interface_key::BUILDINGLIST_ZOOM_Q))
            zooming = true;        

        INTERPOSE_NEXT(feed)(input);        

        if (zooming && (*df::global::window_x != oldwx || *df::global::window_y != oldwy))
            fix_zoom();
    }    
};

IMPLEMENT_VMETHOD_INTERPOSE(viewscreen_buildinglistst_zoomfix, feed);


struct viewscreen_layer_unit_relationshipst_zoomfix : public df::viewscreen_layer_unit_relationshipst
{
    typedef df::viewscreen_layer_unit_relationshipst interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, feed, (std::set<df::interface_key> *input))
    {
        int oldwx = *df::global::window_x, oldwy = *df::global::window_y;
        bool zooming = false;
        if (input->count(interface_key::UNITVIEW_RELATIONSHIPS_ZOOM))
            zooming = true;        

        INTERPOSE_NEXT(feed)(input);        

        if (zooming && (*df::global::window_x != oldwx || *df::global::window_y != oldwy))
            fix_zoom();
    }    
};

IMPLEMENT_VMETHOD_INTERPOSE(viewscreen_layer_unit_relationshipst_zoomfix, feed);
