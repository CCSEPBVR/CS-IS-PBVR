#ifndef PBVR__JOB_DISPATCHER_H_INCLUDE
#define PBVR__JOB_DISPATCHER_H_INCLUDE

#include <vector>
#include <kvs/Vector3>

class JobDispatcher
{

public:
    class JobInfomation
    {
    public:
        int m_id;
        int m_volume;
        int m_step;
        int m_status;    // 0: yet, 1: working, 2: completed
    };

    class WorkerInfomation
    {
    public:
        std::vector<JobInfomation>::iterator m_job;
        int m_rank;
        int m_status;    //  0: idoling,
        //  1: running and writing file on slave(batch mode)
        //  2: running and writing file on master(client mode)
        // -1: exiting
        int m_job_id_pack_size;
        int m_job_id_count;
        std::vector<int> m_job_id_pack;
        int m_latency_over_count;
    };

private:
    std::vector<JobInfomation>    m_job_list;
    std::vector<WorkerInfomation> m_worker_list;
    std::vector<JobInfomation>::iterator m_next_job;
    int m_job_id_count;
    int m_job_id_pack_size;
    int m_job_id_pack_recv_size;
    std::vector<int> m_job_id_pack_recv;
    double m_latency;
    double m_latency_threshold;
    int m_collect_send_state;
    int m_collect_recv_state;
    int m_count_volumes;

public:
    JobDispatcher();
    ~JobDispatcher();

#if 0
    int initialize( int bstep, int estep, int volumes );
    int initialize( int steps, int volumes );
    int initialize( int bstep, int estep, int volumes, double latency );
    int initialize( int bstep, int estep, int volumes, double latency, int pack_size );
#else
    int initialize( const int bstep, const int estep, const std::vector<int>& volume_table );
    int initialize( const int bstep, const int estep, const int volumes,
                    const std::vector<kvs::Vector3f>& volume_min,
                    const std::vector<kvs::Vector3f>& volume_max,
                    const double latency, const int pack_size,
                    const kvs::Vector3f& crop_min,
                    const kvs::Vector3f& crop_max );
    int initialize( const int bstep, const int estep, const int volumes,
                    const std::vector<kvs::Vector3f>& volume_min,
                    const std::vector<kvs::Vector3f>& volume_max,
                    const double latency, const int pack_size );
#endif

    virtual int dispatchNext( const int worker_id, int* step, int* volume );

    JobInfomation getWorkerJobInfomation( const int wid );
    void setLatency( const double L )
    {
        m_latency = L;
    };
    int getCollectSendState()
    {
        return m_collect_send_state;
    };
    int getCollectReceiveState()
    {
        return m_collect_recv_state;
    };
    int getNextJobIDPackReceive()
    {
        return m_job_id_pack_recv[m_job_id_count + 1];
    };
    int getCountVolumes()
    {
        return m_count_volumes;
    };
    void setWorkerStatus( const int wid, const int status );
};

void TestJobDispatcher();

#endif //  PBVR__JOB_DISPATCHER_H_INCLUDE

