#include "systeminfo.h"


#include <QOpenGLExtraFunctions>
#ifdef __linux__
#include <unistd.h>
#endif
namespace systemInfo {


/**
 * @brief getMemoryInfoWin32
 * @param rc
 * @param rt
 * @param rp
 */
void getMemoryInfoWin32( size_t* rc, size_t* rt, size_t* rp)
{
#ifdef WIN32
    char buff[512];
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof ( statex );
    GlobalMemoryStatusEx ( &statex );
    DWORDLONG tx = ( statex.ullTotalPhys / 1024 / 1024 );
    DWORDLONG cx = ( statex.ullAvailPhys / 1024 / 1024 );
    cx = tx - cx;
    DWORD px = statex.dwMemoryLoad;
    sprintf( buff, "CPU MEMORY %I64d/%I64d %ld%%", cx, tx, px );
    *rc=cx;
    *rt=tx;
    *rp=px;
#endif
}

/**
 * @brief getMemoryInfoApple
 * @param rc
 * @param rt
 * @param rp
 */
void getMemoryInfoApple( size_t* rc, size_t* rt, size_t* rp)
{
#ifdef __APPLE__
    char* cmdline1 = "vm_stat |grep 'Pages free:' |awk '{print $3}'";
    char* cmdline2 = "sysctl -a |grep 'hw.memsize:' |awk '{print $2}'";
    char* cmdline3 = "vm_stat |grep 'Pages speculative:' |awk '{print $3}'";
    FILE* fp;
    //    char buff[512];
    char buf[256];
    int t, c, cs;
    size_t lt, lc;

    fp = popen( cmdline1, "r" );
    fgets( buf, 256, fp );
    pclose( fp );

    sscanf( buf, "%d", &c );

    fp = popen( cmdline3, "r" );
    fgets( buf, 256, fp );
    pclose( fp );

    sscanf( buf, "%d", &cs );

    fp = popen( cmdline2, "r" );
    fgets( buf, 256, fp );
    pclose( fp );

    sscanf( buf, "%ld", &lt );

    lc = c + cs;
    c = lc * 4096 / 1024 / 1024;
    //fprintf( stdout, "Free MEMORY : %d\n", c );
    t = lt / 1024 / 1024;
    //fprintf( stdout, "Total MEMORY : %d\n", t );

    c = t - c;
    int p;
    if(t==0){
        p=0;
    }
    else{
        p = c * 100 / t;
    }

    if ( p > 100 ) p = 100;

    sprintf( buf, "CPU MEMORY %d/%d %d%%", c, t, p );
    *rc=c;
    *rt=t;
    *rp=p;
#endif
}
/**
 * @brief getMemoryInfoLinux
 * @param rc
 * @param rt
 * @param rp
 */
void getMemoryInfoLinux( size_t* rc, size_t* rt, size_t* rp)
{
#ifdef __linux__
    const char* cmdline = "free |grep '\\-\\/+ buff' |sed 's/\\s\\{1,\\}/ /g' | cut -d' ' -f4";
    FILE* fp;
    char buf[256] = {0x00};
    GLint c;
    GLint t;

    buf[0] = 0x00;
    //       mutex->lock();
    fp = popen( cmdline, "r" );
    fgets( buf, 256, fp );
    pclose( fp );

    sscanf( buf, "%d", &c );
    c = c / 1024;
    fprintf( stdout, "Free MEMORY : %ld\n", c );

    long pages = sysconf( _SC_PHYS_PAGES );
    long page_size = sysconf( _SC_PAGE_SIZE );

    t = pages * page_size / 1024 / 1024;
    fprintf( stdout, "Total MEMORY : %ld\n", t );

    c = t - c;
    GLint p = c * 100 / t;

    sprintf( buf, "CPU MEMORY %d/%d %d%%", c, t, p );
    //    lbl_smemory->name = buff;
    *rc=c;
    *rt=t;
    *rp=p;
#endif
}

/**
 * @brief getMemoryInfo
 * @param rc
 * @param rt
 * @param rp
 */
void getMemoryInfo( size_t* rc, size_t* rt, size_t* rp){
#if defined( WIN32)
    getMemoryInfoWin32(rc,rt,rp);

#elif defined(__APPLE__)
    getMemoryInfoApple(rc,rt,rp);
#elif defined(__linux__)
    getMemoryInfoLinux(rc,rt,rp);
#endif;
}

GPU_TYPE getGPUType( QOpenGLContext* ctxt)
{
    if ( ctxt->hasExtension(QByteArrayLiteral("GL_ATI_meminfo")) ){
        return GPU_ATI;
    }
    else if(ctxt->hasExtension(QByteArrayLiteral("GL_NVX_gpu_memory_info")) )
    {
       return  GPU_NVIDA;
    }
    return GPU_UNKNOWN;
}
/**
 * @brief GetNVidaMem
 * @param c
 * @param t
 */
void GetNVidaMem(int* c, int* t)
{
    int availableKB[] = { -1, -1, -1, -1 };
    //    glGetIntegerv( GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableKB[0] );
    //    c = availableKB[0] / 1024;
    //    glGetIntegerv( GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &availableKB[0] );
    //    t = availableKB[0] / 1024;


}

/**
 * @brief GetAMDMem
 * @param c
 * @param t
 */
void GetAMDMem(int* c, int* t)
{
#ifdef ATI
    int availableKB[] = { -1, -1, -1, -1 };

    //    ui->gpu_usage_bar->setValue(c);
    GLuint uNoOfGPUs = glGetGPUIDsAMD( 0, 0 );
    GLuint* uGPUIDs = new GLuint[uNoOfGPUs];
    glGetGPUIDsAMD( uNoOfGPUs, uGPUIDs );

    GLuint uTotalMemoryInMB = 0;
    glGetGPUInfoAMD( uGPUIDs[0], WGL_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof( GLuint ), &uTotalMemoryInMB );
    t = uTotalMemoryInMB * 1024;
#endif
}

void GetVideoMem(int* c, int* t)
{
#if defined( ATI )
    GetAMDMem(c,t);
#elif defined (NVIDA)
    GetNVidaMem(c,t);
#elif defined(GL_TEXTURE_FREE_MEMORY_ATI)
    int availableKB[] = { -1, -1, -1, -1 };
    glGetIntegerv( GL_TEXTURE_FREE_MEMORY_ATI, availableKB );
    *c = availableKB[0] / 1024;
    *t=*c*2;
#endif
}
}
