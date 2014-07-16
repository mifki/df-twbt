#ifdef __APPLE__
//0x0079cb2a+4 0x14 - item name length
//0x0079cb18+3 0x14 - item name length

//0x0079e04e+3 0x1a - price, affects both sides
//0x0079cbd1+2 0x1f - weight, affects both sides
//0x0079ccbc+2 0x21 - [T], affects both sides

//0x0079ef96+2 0x29 - "Value:"
//0x0079d84d+2 0x39 - "Max weight"
//0x0079d779+2 0x2a - "offer marked"
//0x0079d07c+2 0x2a - "view good"

//0x0079c314+1 0x3b - our side name (center)
//0x0079cd6b+7 0x28 - our item name (+2)

//0x002e04cf+2 0x27 - border left
//0x002e0540+2 XXXX - border right

struct traderesize_hook : public df::viewscreen_tradegoodsst
{
    typedef df::viewscreen_tradegoodsst interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, render, ())
    {
        static bool checked = false, ok = false;

        if (!checked)
        {
            checked = true;

            //check only some of the addresses
            ok =
                *(unsigned char*)(0x002e04cf+2) == 0x27 &&
                *(unsigned char*)(0x0079ef96+2) == 0x29 &&
                *(unsigned char*)(0x0079cbd1+2) == 0x1f &&
                *(unsigned char*)(0x0079cb2a+4) == 0x14;

            if (ok)
            {
                //fixing drawing of the right border
                unsigned char t1[] = { 0x6b, 0xd1, 0x00 }; //imul edx, ecx, XXX
                Core::getInstance().p->patchMemory((void*)(0x002e0540), t1, sizeof(t1));

                unsigned char t2[] = { 0x01, 0xf2, 0x90 }; //add edx, esi; nop
                Core::getInstance().p->patchMemory((void*)(0x002e0545), t2, sizeof(t2));
            }
        }

        if (ok)
        {
            static int lastw = -1;
            if (gps->dimx != lastw)
            {
                lastw = gps->dimx;

                unsigned char x1 = lastw/2-1, x;

                //border
                x = x1;
                Core::getInstance().p->patchMemory((void*)(0x002e04cf+2), &x, 1);
                x = x1 + 1;
                Core::getInstance().p->patchMemory((void*)(0x002e0540+2), &x, 1);

                x = x1 + 1 + 2;
                Core::getInstance().p->patchMemory((void*)(0x0079d07c+2), &x, 1); //view good
                Core::getInstance().p->patchMemory((void*)(0x0079d779+2), &x, 1); //offer marked

                x = x1 + 1 + 1;
                Core::getInstance().p->patchMemory((void*)(0x0079ef96+2), &x, 1); //value

                x = x1 + 1 + 1 + 16;
                Core::getInstance().p->patchMemory((void*)(0x0079d84d+2), &x, 1); //max weight            

                x = x1 + 1 + 2 - 2;
                Core::getInstance().p->patchMemory((void*)(0x0079cd6b+7), &x, 1); //item name

                x = x1 - 2 - 3;
                Core::getInstance().p->patchMemory((void*)(0x0079ccbc+2), &x, 1); //[T]

                x = x1 - 2 - 3;
                Core::getInstance().p->patchMemory((void*)(0x0079cbd1+2), &x, 1); //item weight

                x = x1 - 2 - 3 - 5;
                Core::getInstance().p->patchMemory((void*)(0x0079e04e + 3), &x, 1); //item price

                x = x1 - 2 - 3 - 5 - 5;
                Core::getInstance().p->patchMemory((void*)(0x0079cb2a+4), &x, 1); //item name len
                Core::getInstance().p->patchMemory((void*)(0x0079cb18+3), &x, 1); //item name len

                x = (x1 + 2 + lastw) / 2;
                Core::getInstance().p->patchMemory((void*)(0x0079c314+1), &x, 1); //our side name
            }
        }

        INTERPOSE_NEXT(render)();
    }
};

IMPLEMENT_VMETHOD_INTERPOSE(traderesize_hook, render);
#endif