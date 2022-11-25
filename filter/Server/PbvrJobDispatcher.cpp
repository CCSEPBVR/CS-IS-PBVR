/*
 * PbvrJobDispatcher.cpp
 *
 *  Created on: 2017/01/08
 *      Author: hira
 */

#include "PbvrJobDispatcher.h"

/**
 * コンストラクタ
 */
PbvrJobDispatcher::PbvrJobDispatcher() : JobDispatcher(), m_filterinfos(NULL) {

}

/**
 * デストラクタ
 */
PbvrJobDispatcher::~PbvrJobDispatcher() {

}

/**
 * フィルタリスト情報を設定する
 * @param fil		フィルタリスト情報
 */
void PbvrJobDispatcher::setFilterInformationList(const FilterInformationList *fil)
{
    this->m_filterinfos = fil;
}



int PbvrJobDispatcher::dispatchNext( const int worker_id, int* step, int* volume )
{
    int rank = 0;
    int size = 1;
    int ret = 0;
    int next_step = -1;
    int next_volume = -1;

#ifndef CPU_VER
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#endif

    if (this->m_filterinfos == NULL || !this->m_filterinfos->isPbvrFilterInfo()) {
        return JobDispatcher::dispatchNext(worker_id, step, volume);
    }

    std::vector<JobInfomation>::iterator itr;
    for (itr = m_next_job; itr != m_job_list.end(); itr++) {
        int jid = itr->m_id;

        next_step   = m_job_list[jid].m_step;
        next_volume = m_job_list[jid].m_volume;

        m_next_job++;
        int xvl = -1;
        int fidx = this->m_filterinfos->getFileIndex( next_volume, &xvl );
        if (fidx < 0 || fidx >= this->m_filterinfos->m_list.size()) {
            printf("[rank:%d/%d] Error : not found subvoule Job : fidx=%d, step=%d, volume=%d\n",
                    rank, size, fidx, next_step, next_volume);
            return 0;
        }
        FilterInformationFile *fi = this->m_filterinfos->m_list[fidx];
        pbvr::filter::DIVISION_TYPE type = fi->getFilterDivision();
        // マスタースレーブ方式で、ランク０はすべてのジョブを回す。
        if (rank == 0 && type == pbvr::filter::MASTER_DIVISION) {
            ret = 1;
            break;
        }
        if (fi == NULL) continue;

        // このプロセスがサブボリュームの対象であるかチェックする。
        if (fi->isPbvrSubvolume(xvl, rank, size)) {
            ret = 1;
            break;
        }
    }

    if (ret == 1) {
        *step = next_step;
        *volume = next_volume;
        printf("[rank:%d/%d] Dispatched Job : step=%d, volume=%d\n",
                rank, size, *step, *volume);
        m_collect_send_state = 1;
    }
    else {
        std::cerr << "Dispatched No Job " << std::endl;
        m_collect_send_state = 0;
    }
    std::flush( std::cerr );

    return ret;
}


