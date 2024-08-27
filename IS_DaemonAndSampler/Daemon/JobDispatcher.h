#ifndef MPI_JOB_DISPATCHER_H_INCLUDE
#define MPI_JOB_DISPATCHER_H_INCLUDE

#include <vector>
#include <kvs/Vector3>

class JobDispatcher
{

public:
    struct job_info
    {
        int id;
        int volume;
        int step;
        int status;    // 0: yet, 1: working, 2: completed
    };

    struct worker_info
    {
        std::vector<job_info>::iterator job;
        int rank;
        int status;    //  0: idoling,
        //  1: running and writing file on slave(batch mode)
        //  2: running and writing file on master(client mode)
        // -1: exiting
        int jid_pack_size;
        int jid_count;
        std::vector<int> jid_pack;
        int latency_over_count;
    };

private:
    std::vector<job_info>    job_list;
    std::vector<worker_info> worker_list;
    std::vector<job_info>::iterator next_job;
    int jid_count;
    int jid_pack_size;
    int jid_pack_recv_size;
    std::vector<int> jid_pack_recv;
    double latency;
    double latency_threshold;
    int col_send_stat;
    int col_recv_stat;
    int count_volumes;

public:
    JobDispatcher();
    ~JobDispatcher();

#if 0
    int Initialize( int bstep, int estep, int volumes );
    int Initialize( int steps, int volumes );
    int Initialize( int bstep, int estep, int volumes, double latency );
    int Initialize( int bstep, int estep, int volumes, double latency, int pack_size );
#else
    int Initialize( int bstep, int estep, std::vector<int> volume_table );
    int Initialize( int bstep, int estep, int volumes,
                    std::vector<kvs::Vector3f> volume_min,
                    std::vector<kvs::Vector3f> volume_max,
                    double latency, int pack_size,
                    kvs::Vector3f crop_min,
                    kvs::Vector3f crop_max );
    int Initialize( int bstep, int estep, int volumes,
                    std::vector<kvs::Vector3f> volume_min,
                    std::vector<kvs::Vector3f> volume_max,
                    double latency, int pack_size );
#endif

    virtual int DispatchNext( int worker_id, int* step, int* volume );

    job_info GetWorkerJobInfo( int wid );
    void SetLatency( double L )
    {
        latency = L;
    };
    int GetColSendStat()
    {
        return ( col_send_stat );
    };
    int GetColRecvStat()
    {
        return ( col_recv_stat );
    };
    int GetNextJidPackRecv()
    {
        return ( jid_pack_recv[jid_count + 1] );
    };
    int GetCountVolumes()
    {
        return ( count_volumes );
    };
    void setWorkerStatus( int wid, int status );
};

void test_JobDispatcher();

#endif //  MPI_JOB_DISPATCHER_H_INCLUDE

