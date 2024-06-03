/******************************************************************************/
#include "../../stdafx.h"
/******************************************************************************/
StoreFiles IC;
/******************************************************************************/

/******************************************************************************/
void StoreFiles::init() {
    if (!initialized) {
        initialized = true;
        create("Bin/Store.dat", &WorkerThreads, null, COMPRESS_LZ4);
    }
}
ImagePtr StoreFiles::getImage(C Str &url) {
    init();
    return super::getImage(url);
}
StoreFiles::StoreFiles() : initialized(false) {}

/******************************************************************************/
