#ifndef PBVR__MEMORY_MONITOR_H_INCLUDE
#define PBVR__MEMORY_MONITOR_H_INCLUDE

#include "kvs/Thread"
#include "kvs/Mutex"
#include <fstream>

#define PBVR_TIME 3

#ifndef WIN32
// add by @hira at 2016/12/01
#include <unistd.h>
#endif

class UpdateCPUMemory: public kvs::Thread
{
public:
    kvs::Mutex* m_mutex;
    void* m_args;
    void run();
};
class UpdateGPUMemory: public kvs::Thread
{
public:
    kvs::Mutex* m_mutex;
    void* m_args;
    void run();
};
//
// CPU???????X?V
// CPU MEMORY 2042/2097 97%
//
void UpdateCPUMemory::run()
{
#ifdef NOTOPEN
    char buff[512];
    char buf[256];
    while ( 1 )
    {
//   mutex->lock();
        fprintf( stdout, "called UpdateCPUMemory\n" );

        int t = kvs::SystemInformation::totalMemorySize() / 1024;
        int c = kvs::SystemInformation::freeMemorySize() / 1024;
        c_smemory = c;
        c = t - c;
        int p = c * 100 / t;

        sprintf( buff, "CPU MEMORY %d/%d %d%%", c, t, p );
        lbl_smemory->name = buff;
//        std::cout << "NOTEPEN:" << buff << std::endl;
//   mutex->unlock();
        this->sleep ( PBVR_TIME );
    }
#endif

#ifdef WIN32

    char buff[512];
    int c;
    int t;
    while ( 1 )
    {
//    mutex->lock();
#if 0
        MEMORYSTATUS stat;
        GlobalMemoryStatus ( &stat );

        t = ( stat.dwTotalPhys / 1024 / 1024 );
        c = ( stat.dwAvailPhys / 1024 / 1024 );
        c = t - c;

        int p = c * 100 / t;

        sprintf( buff, "CPU MEMORY %d/%d %d%%", c, t, p );
#else
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof ( statex );
        GlobalMemoryStatusEx ( &statex );

        DWORDLONG tx = ( statex.ullTotalPhys / 1024 / 1024 );
        DWORDLONG cx = ( statex.ullAvailPhys / 1024 / 1024 );
        c_smemory = cx;
        cx = tx - cx;

        DWORD px = statex.dwMemoryLoad;
        sprintf( buff, "CPU MEMORY %I64d/%I64d %ld%%", cx, tx, px );
#endif
        lbl_smemory->name = buff;
//    mutex->unlock();
        this->sleep ( PBVR_TIME );
    }
#else

//#ifdef MAC
#ifdef __APPLE__
    char* cmdline1 = "vm_stat |grep 'Pages free:' |awk '{print $3}'";
    char* cmdline2 = "sysctl -a |grep 'hw.memsize:' |awk '{print $2}'";
    char* cmdline3 = "vm_stat |grep 'Pages speculative:' |awk '{print $3}'";
    FILE* fp;
    char buff[512];
    char buf[256];
    int t, c, cs;
    size_t lt, lc;
    while ( 1 )
    {
//    mutex->lock();
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
        fprintf( stdout, "Free MEMORY : %d\n", c );
        t = lt / 1024 / 1024;
        fprintf( stdout, "Total MEMORY : %d\n", t );
        c_smemory = c;
        c = t - c;
        int p = c * 100 / t;
        if ( p > 100 ) p = 100;

        sprintf( buff, "CPU MEMORY %d/%d %d%%", c, t, p );
        lbl_smemory->name = buff;
//        std::cout << "MAC:" << buff << std::endl;
//    mutex->unlock();
        this->sleep ( PBVR_TIME );
    }
#else
    const char* cmdline = "free |grep '\\-\\/+ buff' |sed 's/\\s\\{1,\\}/ /g' | cut -d' ' -f4";
    FILE* fp;
    char buff[512];
    char buf[256] = {0x00};
    GLint c;
    GLint t;
    while ( 1 )
    {
        buf[0] = 0x00;
//       mutex->lock();
        fp = popen( cmdline, "r" );
        fgets( buf, 256, fp );
        pclose( fp );

#if 0
// #ifdef _DEBUG		// debug by @hira
    printf("[%s:%d] :: cmdline=%s, buf='%s' \n",
            __FILE__, __LINE__, cmdline, buf);
#endif

        sscanf( buf, "%d", &c );
        c = c / 1024;
        fprintf( stdout, "Free MEMORY : %ld\n", c );

        long pages = sysconf( _SC_PHYS_PAGES );
        long page_size = sysconf( _SC_PAGE_SIZE );

        t = pages * page_size / 1024 / 1024;
        fprintf( stdout, "Total MEMORY : %ld\n", t );
        c_smemory = c;
        c = t - c;
        GLint p = c * 100 / t;

        sprintf( buff, "CPU MEMORY %d/%d %d%%", c, t, p );
        lbl_smemory->name = buff;
//        lbl_smemory->redraw();
//        mutex->unlock();
//        std::cout << "other:" << buff << std::endl;
        this->sleep ( PBVR_TIME );
    }
#endif
#endif

}
//
// GPU???????X?V
// GPU MEMORY 2042/2097 97%
//
void UpdateGPUMemory::run()
{
    int availableKB[] = { -1, -1, -1, -1};
    char buff[512];
    while ( 1 )
    {
        m_mutex->lock();
#ifndef CPUMODE
        bool f = false;
        if ( GLEW_NVX_gpu_memory_info )
        {
            GLint c;
            GLint t;
            glGetIntegerv( GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableKB[0] );
            fprintf( stdout, "NVidia card : %d %d %d %d\n", availableKB[0], availableKB[1], availableKB[2], availableKB[3] );
            c = availableKB[0];
            glGetIntegerv( GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &availableKB[0] );
            fprintf( stdout, "NVidia card : %d %d %d %d\n", availableKB[0], availableKB[1], availableKB[2], availableKB[3] );
            t = availableKB[0];
            GLint p = c * 100 / t;

            sprintf( buff, "GPU MEMORY %d/%d %d%%", c, t, p );
            lbl_gmemory->name = buff;
            //lbl_gmemory->redraw();

            f = true;
        }
//        int temp = GLEW_ATI_meminfo;
        if ( GLEW_ATI_meminfo )
        {
            GLint c;
//                GLint t;

#ifdef ATI
            GLuint uNoOfGPUs = wglGetGPUIDsAMD( 0, 0 );
            GLuint* uGPUIDs = new GLuint[uNoOfGPUs];
            wglGetGPUIDsAMD( uNoOfGPUs, uGPUIDs );

            GLuint uTotalMemoryInMB = 0;
            wglGetGPUInfoAMD( uGPUIDs[0], WGL_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof( GLuint ), &uTotalMemoryInMB );
            t = uTotalMemoryInMB * 1024;
#endif

            glGetIntegerv( GL_TEXTURE_FREE_MEMORY_ATI, availableKB );
            fprintf( stdout, "ATI card\n" );

            c = availableKB[0];
#ifdef ATI
            GLint p = c * 100 / t;
            sprintf( buff, "GPU MEMORY %d/%d %d%%", c, t, p );
#else
            sprintf( buff, "GPU MEMORY %d/- -\%%", c );
#endif
            lbl_gmemory->name = buff;
            //lbl_gmemory->redraw();

            f = true;
        }
        if ( !f )
        {
            fprintf( stdout, "no card\n" );
            sprintf( buff, "GPU MEMORY -/- -\%%" );

            lbl_gmemory->name = buff;
        }
#else
        fprintf( stdout, "no card\n" );
        sprintf( buff, "GPU MEMORY -/- -%%" );

        lbl_gmemory->name = buff;
#endif
        m_mutex->unlock();
        this->sleep ( PBVR_TIME );
    }
}

#endif //PBVR__MEMORY_MONITOR_H_INCLUDE
