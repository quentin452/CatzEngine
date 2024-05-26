/******************************************************************************/
class PropExs {
    Memx<PropEx> props;

    PropEx &add(C Str &name = S, C MemberDesc &md = MemberDesc());
    PropExs &toGui();
    PropExs &toGui(cptr data);
    PropExs &autoData(ptr data);
    ptr autoData() C;
    PropExs &changed(void (*changed)(C Property &prop), void (*pre_changed)(C Property &prop) = null);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
