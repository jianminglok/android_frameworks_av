#define LOG_TAG "MtkCamera"
#include <utils/Log.h>

#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <camera/MtkCamera.h>

namespace android {


MtkCamMsgExtDataHelper::
MtkCamMsgExtDataHelper()
    : mIsValid(false)
    , mspData(0)
    , mspHeap(0)
    , mDataOffset(0)
    , mDataSize(0)
{
    ::memset(&mExtDataHdr, 0, sizeof(mExtDataHdr));
}


MtkCamMsgExtDataHelper::
~MtkCamMsgExtDataHelper()
{
    uninit();
}


bool
MtkCamMsgExtDataHelper::
init(const sp<IMemory>& dataPtr)
{
    bool ret = false;
    //
    sp<IMemoryHeap> heap = 0;
    ssize_t         offset = 0;
    size_t          size = 0;
    //
    if  ( NULL == dataPtr.get() ) {
        ALOGE("[MtkCamMsgExtDataHelper] dataPtr is NULL \r\n");
        goto lbExit;
    }
    //
    heap = dataPtr->getMemory(&offset, &size);
    if  ( NULL == heap.get() || NULL == heap->base() ) {
        ALOGE("[MtkCamMsgExtDataHelper] heap or heap->base() is NULL - (heap,offset,size)=(%p,%d,%d) \r\n", heap.get(), offset, size);
        goto lbExit;
    }
    //
    if ( sizeof(DataHeader) > size ) {
        ALOGE("[MtkCamMsgExtDataHelper] sizeof(DataHeader)(%d) > size(%d) \r\n", sizeof(DataHeader), size);
        goto lbExit;
    }
    //
    ::memcpy(&mExtDataHdr, ((uint8_t*)heap->base()) + offset, sizeof(DataHeader));
    mspData = dataPtr;
    mspHeap = heap;
    mDataOffset = offset;
    mDataSize   = size;
    mIsValid= true;
    ret = true;
lbExit:
    return  ret;
}


bool
MtkCamMsgExtDataHelper::
uninit()
{
    mIsValid= false;
    mspData = NULL;
    mspHeap = NULL;
    mDataOffset = 0;
    mDataSize   = 0;
    ::memset(&mExtDataHdr, 0, sizeof(mExtDataHdr));
    return  true;
}


bool
MtkCamMsgExtDataHelper::
create(size_t const extParamSize, uint32_t const u4ExtMsgType)
{
    bool ret = false;
    //
    size_t const extDataSize = sizeof(DataHeader) + extParamSize;
    sp<IMemoryHeap> heap = 0;
    sp<IMemory> dataPtr = 0;

    //  (1) Check arguments.
    if  ( 0 == extParamSize )
    {
        ALOGW("[MtkCamMsgExtDataHelper::create] extParamSize==0 \r\n");
    }

    //  (2) Allocate memory
    heap = new MemoryHeapBase(extDataSize, 0, NULL);
    dataPtr = new MemoryBase(heap, 0, extDataSize);

    //  (3) Initialize.
    ret = init(dataPtr);
    if  ( ! ret )
    {
        ALOGE("[MtkCamMsgExtDataHelper::create] init fail \r\n");
        goto lbExit;
    }

    //  (4) Assign the header.
    mExtDataHdr.extMsgType = u4ExtMsgType;
    ::memcpy(((uint8_t*)mspHeap->base()) + mDataOffset, &mExtDataHdr, sizeof(DataHeader));

    ret = true;
lbExit:
    return  ret;
}


bool
MtkCamMsgExtDataHelper::
destroy()
{
    return  uninit();
}


uint8_t*
MtkCamMsgExtDataHelper::
getExtParamBase() const
{
    return  mIsValid
        ?   static_cast<uint8_t*>(mspHeap->base()) + mDataOffset + sizeof(DataHeader)
        :   NULL;
}


size_t
MtkCamMsgExtDataHelper::
getExtParamSize() const
{
    return  mIsValid
        ?   (mDataSize - sizeof(DataHeader))
        :   0;
}


ssize_t
MtkCamMsgExtDataHelper::
getExtParamOffset() const
{
    return  mIsValid
        ?   (mDataOffset + sizeof(DataHeader))
        :   0;
}


}; // namespace android
