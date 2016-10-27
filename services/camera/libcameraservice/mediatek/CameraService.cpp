#define LOG_TAG "CameraService"
//
#include <utils/Log.h>
#include "CameraService.h"
//
#include <dlfcn.h>


namespace android {


/******************************************************************************
 *
 ******************************************************************************/
status_t
CameraService::
getProperty(String8 const& key, String8& value) const
{
    char const* sym = "MtkCam_getProperty";
    void* pfn = ::dlsym(mModule->getDso(), sym);
    if  ( ! pfn ) {
        ALOGW("Cannot find symbol: %s", sym);
        return  INVALID_OPERATION;
    }
    return  reinterpret_cast<status_t(*)(String8 const&, String8&)>(pfn)(key, value);
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CameraService::
setProperty(String8 const& key, String8 const& value)
{
    char const* sym = "MtkCam_setProperty";
    void* pfn = ::dlsym(mModule->getDso(), sym);
    if  ( ! pfn ) {
        ALOGW("Cannot find symbol: %s", sym);
        return  INVALID_OPERATION;
    }
    return  reinterpret_cast<status_t(*)(String8 const&, String8 const&)>(pfn)(key, value);
}


};
