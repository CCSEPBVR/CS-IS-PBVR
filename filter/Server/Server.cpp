/*
 * Server.cpp
 *
 *  Created on: 2017/01/08
 *      Author: hira
 */

#ifndef CPU_VER
#include <mpi.h>
#endif

#include "Server.h"
#include "kvs/File"

/**
 * コンストラクタ
 */
Server::Server() : m_param(NULL), m_isloadpbvrfilter(false) {
}

/**
 * コンストラクタ
 * @param param		引数情報
 */
Server::Server(const Argument *param) : m_param(param), m_isloadpbvrfilter(false), m_loadfilename("") {
}

/**
 * デストラクタ
 */
Server::~Server() {
}

/**
 * フィルタファイルクラスを生成し、フィルタファイルをロードする
 * @return		フィルタファイルクラス
 */
FilterInformationList* Server::loadFilterInformationList()
{
    if (this->m_param == NULL) return NULL;

    FilterInformationList *fil = NULL;
    std::string loadfilename = "";
    std::string input_directory = "";
    this->m_isloadpbvrfilter = false;
    if (!this->m_param->m_filter_parameter_filename.empty()) {
        loadfilename = this->m_param->m_filter_parameter_filename;
        kvs::File pfi( loadfilename );
        if ( pfi.isExisted() ) {
            fil = new PbvrFilterInformationList();
            fil->loadPFL( loadfilename );
        }
        this->m_isloadpbvrfilter = true;
    }
    else if (!this->m_param->m_input_data_base.empty()) {
        std::string pflfile, pfifile;
        std::string ext = this->m_param->m_input_data_base.substr( this->m_param->m_input_data_base.size() - 3 );
        if ( ext == "pfl" || ext == "pfi" ) {
            loadfilename = this->m_param->m_input_data_base;
            input_directory = this->m_param->m_input_data_base.substr( 0, this->m_param->m_input_data_base.size() - 4 );
        }
        else {
            pflfile = this->m_param->m_input_data_base + ".pfl";
            pfifile = this->m_param->m_input_data_base + ".pfi";
            if ( kvs::File(pflfile).isExisted() ) {
                loadfilename = pflfile;
            }
            else if ( kvs::File(pfifile).isExisted() ) {
                loadfilename = pfifile;
            }
            input_directory = this->m_param->m_input_data_base;
        }
        if (!loadfilename.empty()) {
            fil = new FilterInformationList();
            fil->loadPFL( loadfilename );
        }
        this->m_isloadpbvrfilter = false;
    }
    if (this->m_isloadpbvrfilter) {
        this->m_loadfilename = loadfilename;
    }
    else {
        this->m_loadfilename = input_directory;
    }

    return fil;
}

/**
 * PBVRフィルタファイルをロードしたかを取得する
 * @return		true=PBVRフィルタファイルをロード
 */
bool Server::isLoadPbvrFilter()
{
    return this->m_isloadpbvrfilter;
}

/**
 * ロードフィルタファイル名を取得する
 * @return		フィルタファイル名
 */
std::string & Server::getLoadFilterFile()
{
    return this->m_loadfilename;
}


int Server::send_bcast(const jpv::ParticleTransferClientMessage *msg)
{

    int bsz = 0;
    char *buf = NULL;

    // send cltMes to all worker process >>
    bsz = msg->byteSize();
#ifndef CPU_VER
    MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
    if (bsz > 0) {
        buf = new char[bsz];
        msg->pack( buf );
#ifndef CPU_VER
        MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
    }

    if (buf != NULL) delete[] buf;

    return bsz;
}

int Server::recv_bcast(jpv::ParticleTransferClientMessage *msg)
{
    int bsz = -1;
    char *buf = NULL;
    // recv cltMes from process 0 >>
#ifndef CPU_VER
    MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
    if ( bsz > 0 ) {
        buf = new char[bsz];
#ifndef CPU_VER
        MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
        msg->unpack( buf );
    }

    if (buf != NULL) delete[] buf;

    return bsz;
}
