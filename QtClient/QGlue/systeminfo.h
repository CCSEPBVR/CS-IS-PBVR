#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H
#include <QOpenGLContext>
namespace systemInfo {

    enum GPU_TYPE { GPU_UNKNOWN, GPU_ATI,GPU_NVIDA};
    GPU_TYPE getGPUType(QOpenGLContext* ctxt);
    void getMemoryInfo( size_t* rc, size_t* rt, size_t* rp);
    void GetVideoMem(int* c, int* t);

}


#endif // SYSTEMINFO_H
