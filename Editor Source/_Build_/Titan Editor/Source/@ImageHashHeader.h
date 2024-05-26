/******************************************************************************/
class ImageHashHeader // !! try to don't make any changes to this class layout, because doing so will require a new hash for every texture !!
{
    VecI size;
    IMAGE_TYPE type;
    byte flags;

    ImageHashHeader(C Image &image, IMAGE_TYPE type);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
