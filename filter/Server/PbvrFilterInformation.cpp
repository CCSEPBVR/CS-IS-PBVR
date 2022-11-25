/**
 * PbvrFilterInfoLst.cpp
 *
 *  Created on: 2016/12/02
 *      Author: hira
 */
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <string.h>
#ifndef CPU_VER
#include "mpi.h"
#endif
#ifdef WIN32
#include <iterator>
#endif

#include "PbvrFilterInformation.h"
#include "kvs/File"
#include "endian2.h"
#include "filter_utils.h"
#include "../../Server/timer_simple.h"

/**
 * コンストラクタ
 */
PbvrFilterInformationFile::PbvrFilterInformationFile() : FilterInformationFile() {
    this->m_pbvrfilter = NULL;
    this->element_type_num = 0;
    this->m_list = NULL;
    this->m_nvolumes = 0;
    this->m_elems_per_subvolume = NULL;
    this->m_conns_per_subvolume = NULL;
    this->m_nodes_per_subvolume = NULL;
    this->m_subvolume_nodes = NULL;

    /****  modify 2017/03/20 : データコピーを止める。PbvrFilterInfo *infoから取得する  *****
    this->m_pbvr_connections.clear();
    this->m_pbvr_connect_count.clear();
    this->m_pbvr_subvolumeids.clear();
    *********************/
}

/**
 * デストラクタ
 */
PbvrFilterInformationFile::~PbvrFilterInformationFile() {

    /**** delete by @hira at 2018/03/20 : データコピーからポインタコピーに変更のため削除
     * 実態は this->m_pbvrfilter->m_conns_per_subvolume,...
     **************
    if (this->m_conns_per_subvolume != NULL) {
        delete m_conns_per_subvolume;
        this->m_conns_per_subvolume = NULL;
    }
    if (this->m_nodes_per_subvolume != NULL) {
        delete m_nodes_per_subvolume;
        this->m_nodes_per_subvolume = NULL;
    }
    if (this->m_elems_per_subvolume != NULL) {
        delete m_elems_per_subvolume;
        this->m_elems_per_subvolume = NULL;

    }
    if (this->m_subvolume_nodes != NULL) {
        delete m_subvolume_nodes;
        this->m_subvolume_nodes = NULL;
    }
    *****************/

    /****  modify 2017/03/20 : データコピーを止める。PbvrFilterInfo *infoから取得する  *****
    this->m_pbvr_connections.clear();
    this->m_pbvr_connect_count.clear();
    this->m_pbvr_subvolumeids.clear();
    ***********************/

    if (this->m_pbvrfilter != NULL) {
        this->m_pbvrfilter->finalize();
        delete this->m_pbvrfilter;
        this->m_pbvrfilter = NULL;
    }
}

/**
 * 破棄処理
 */
void PbvrFilterInformationFile::finalizePbvrFilterInfo() {
    if (this->m_pbvrfilter != NULL) delete this->m_pbvrfilter;
}


/**
 * 読込PbvrFilterInfoリストを設定する。
 * @param info_list		読込PbvrFilterInfoリスト
 * @return		設定数
 */
int PbvrFilterInformationFile::setPbvrFilterInfoList(
        std::vector<pbvr::filter::PbvrFilterInfo*> &info_list)
{
    std::vector<pbvr::filter::PbvrFilterInfo*>::iterator itr;
    for (itr=info_list.begin(); itr!=info_list.end(); itr++) {
        pbvr::filter::PbvrFilterInfo* info = (*itr);

        PbvrFilterInformationFile* pbvrinfo = createPbvrFilterInfo(info);
        if (pbvrinfo == NULL) return 0;
        this->m_list->push_back(pbvrinfo);
    }

    return  this->m_list->size();
}


/**
 * フィルタライブラリ（フィルタ統合）を使用しているかをチェックする。
 * @return		true=フィルタライブラリ（フィルタ統合）を使用
 */
bool PbvrFilterInformationFile::isPbvrFilterInfo() const
{
    return (this->m_pbvrfilter != NULL);
}

/**
 * フィルタファイル情報を作成する
 * @param info		フィルタ情報
 * @param [out] pbvrinfo		フィルタファイル情報
 * @return		フィルタファイル情報
 */
PbvrFilterInformationFile* PbvrFilterInformationFile::createPbvrFilterInfo(pbvr::filter::PbvrFilterInfo *info, PbvrFilterInformationFile* pbvrinfo)
{
    if (info == NULL) return NULL;
    if (info->m_param == NULL) return NULL;

    if (pbvrinfo == NULL) {
        pbvrinfo = new PbvrFilterInformationFile();
    }
    pbvr::filter::FilterParam *param = info->m_param;

    pbvrinfo->m_number_nodes = info->m_nnodes;
    pbvrinfo->m_number_elements = info->m_nelements;
    if (info->m_param->struct_grid) {
        pbvrinfo->m_elem_type = info->m_element_type;
    } else {
        pbvrinfo->m_elem_type = info->m_element_type;
    }

    pbvrinfo->m_file_type = info->m_param->file_type;
    pbvrinfo->m_number_files = info->m_param->out_num;
    pbvrinfo->m_number_ingredients = info->m_nkinds;
    pbvrinfo->m_start_step = info->m_param->start_step;
    pbvrinfo->m_end_steps = info->m_param->end_step;
    pbvrinfo->m_number_subvolumes = info->m_nvolumes;
    pbvrinfo->m_number_steps = pbvrinfo->m_end_steps - pbvrinfo->m_start_step + 1;

    float x_min, y_min, z_min;
    float x_max, y_max, z_max;
    x_min = info->m_min_object_coord.x;
    y_min = info->m_min_object_coord.y;
    z_min = info->m_min_object_coord.z;
    x_max = info->m_max_object_coord.x;
    y_max = info->m_max_object_coord.y;
    z_max = info->m_max_object_coord.z;
    pbvrinfo->m_min_object_coord.set( x_min, y_min, z_min );
    pbvrinfo->m_max_object_coord.set( x_max, y_max, z_max );

    pbvrinfo->m_min_subvolume_coord.resize( pbvrinfo->m_number_subvolumes );
    pbvrinfo->m_max_subvolume_coord.resize( pbvrinfo->m_number_subvolumes );
    for ( int32_t vl = 0; vl < pbvrinfo->m_number_subvolumes; vl++ )
    {
        float sub_x_min, sub_y_min, sub_z_min;
        float sub_x_max, sub_y_max, sub_z_max;
        int id = info->m_tree_node_no - info->m_nvolumes + vl;
        sub_x_min = info->m_tree[id].min.x;
        sub_y_min = info->m_tree[id].min.y;
        sub_z_min = info->m_tree[id].min.z;
        sub_x_max = info->m_tree[id].max.x;
        sub_y_max = info->m_tree[id].max.y;
        sub_z_max = info->m_tree[id].max.z;
        pbvrinfo->m_min_subvolume_coord[vl].set( sub_x_min, sub_y_min, sub_z_min );
        pbvrinfo->m_max_subvolume_coord[vl].set( sub_x_max, sub_y_max, sub_z_max );
    }

    pbvrinfo->m_ingredient_step.clear();
    for ( int32_t s = 0; s < pbvrinfo->m_number_steps; s++ )
    {
        pbvrinfo->m_ingredient_step.push_back( IngredientsStep() );
        pbvrinfo->m_ingredient_step[s].m_ingredient.clear();
        for ( int32_t i = 0; i < pbvrinfo->m_number_ingredients; i++ )
        {
            float min, max;
            min = info->m_value_min[info->m_nkinds * s + i];
            max = info->m_value_max[info->m_nkinds * s + i];
            pbvrinfo->m_ingredient_step[s].m_ingredient.push_back( IngredientsMinMax() );
            pbvrinfo->m_ingredient_step[s].m_ingredient[i].m_min = min;
            pbvrinfo->m_ingredient_step[s].m_ingredient[i].m_max = max;
        }
    }

    pbvrinfo->m_min_value = pbvrinfo->m_ingredient_step[0].m_ingredient[0].m_min;
    pbvrinfo->m_max_value = pbvrinfo->m_ingredient_step[0].m_ingredient[0].m_max;
    for ( int32_t s = 0; s < pbvrinfo->m_number_steps; s++ )
    {
        for ( int32_t i = 0; i < pbvrinfo->m_number_ingredients; i++ )
        {
            float min = pbvrinfo->m_ingredient_step[s].m_ingredient[i].m_min;
            float max = pbvrinfo->m_ingredient_step[s].m_ingredient[i].m_max;
            if ( min < pbvrinfo->m_min_value ) pbvrinfo->m_min_value = min;
            if ( max > pbvrinfo->m_max_value ) pbvrinfo->m_max_value = max;
        }
    }

    pbvrinfo->element_type_num = info->element_type_num;

    /****  modify 2017/03/20 : データコピーを止める。PbvrFilterInfo *infoから取得する  *****
    pbvrinfo->m_pbvr_connections.clear();
    pbvrinfo->m_pbvr_connect_count.clear();
    pbvrinfo->m_pbvr_subvolumeids.clear();
    {
        std::vector<int>::iterator itr;
        for (itr = info->m_pbvr_connections.begin(); itr != info->m_pbvr_connections.end(); ++itr) {
            int value = *itr;
            pbvrinfo->m_pbvr_connections.push_back(value);
        }
    }
    {
        std::vector<int>::iterator itr;
        for (itr = info->m_pbvr_connect_count.begin(); itr != info->m_pbvr_connect_count.end(); ++itr) {
            int value = *itr;
            pbvrinfo->m_pbvr_connect_count.push_back(value);
        }
    }
    {
        std::vector<int>::iterator itr;
        for (itr = info->m_pbvr_subvolumeids.begin(); itr != info->m_pbvr_subvolumeids.end(); ++itr) {
            int value = *itr;
            pbvrinfo->m_pbvr_subvolumeids.push_back(value);
        }
    }
    *******************/

    pbvrinfo->m_nvolumes = info->m_nvolumes;
    pbvrinfo->m_element_type = info->m_element_type;

    pbvrinfo->m_conns_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    pbvrinfo->m_nodes_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    pbvrinfo->m_elems_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    memcpy(pbvrinfo->m_elems_per_subvolume, info->m_elems_per_subvolume, sizeof (unsigned int)*info->m_nvolumes);
    memcpy(pbvrinfo->m_conns_per_subvolume, info->m_conns_per_subvolume, sizeof (unsigned int)*info->m_nvolumes);
    memcpy(pbvrinfo->m_nodes_per_subvolume, info->m_nodes_per_subvolume, sizeof (unsigned int)*info->m_nvolumes);
    /****  modify 2017/03/20 : データコピーを止める。ポインタコピーとする  *****
    memcpy(pbvrinfo->m_subvolume_nodes, info->m_subvolume_nodes, sizeof (int)*info->prov_len);
    pbvrinfo->m_subvolume_nodes = (int *) malloc(sizeof (int)*info->prov_len);
    *****************/
    // pbvrinfo->m_elems_per_subvolume = info->m_elems_per_subvolume;
    // pbvrinfo->m_conns_per_subvolume = info->m_conns_per_subvolume;
    // pbvrinfo->m_nodes_per_subvolume = info->m_nodes_per_subvolume;
    pbvrinfo->m_subvolume_nodes = info->m_subvolume_nodes;

    pbvrinfo->m_prov_len = info->prov_len;
    pbvrinfo->m_pbvrfilter = new pbvr::filter::PbvrFilter();
    pbvrinfo->m_file_path = this->m_file_path;

    // pbvrinfo->m_pbvrfilter->setPbvrFilterInfo(new pbvr::filter::PbvrFilterInfo(*info));
    pbvrinfo->m_pbvrfilter->setPbvrFilterInfo(info);

    return pbvrinfo;
}

/**
 * フィルタパラメータファイルを読み込む。
 * フィルタパラメータファイル(ex:param.txt)とフィルタパラメータリストファイル(*.pfl)の
 * 自動読み込み切り替えを行う。
 * @param filename		フィルタパラメータファイル
 * @return	0=正常終了, 0以外=異常終了
 */
int PbvrFilterInformationFile::loadPFI( const std::string& filename )
{

    int rank = 0;
    int mpi_size = 1;
#ifndef CPU_VER
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#endif

    pbvr::filter::PbvrFilter *pbvr_filter = new pbvr::filter::PbvrFilter();
    pbvr::filter::DIVISION_TYPE type = pbvr::filter::MASTER_DIVISION;
    pbvr_filter->setFilterDivision(type, mpi_size-1);

    int n_elem_type = pbvr_filter->count_cellTypes(filename.c_str());

    this->m_file_path = filename;

    int status = pbvr_filter->loadPbvrFilter(filename.c_str());
    if (status != pbvr::filter::RTC_OK) {
        return -1;
    }
    std::vector<pbvr::filter::PbvrFilterInfo*> info_list = pbvr_filter->getPbvrFilterList();
    this->setPbvrFilterInfoList(info_list);

    pbvr::filter::PbvrFilterInfo *info = pbvr_filter->getPbvrFilterInfo();
    if (info == NULL) return -1;
    if (info->m_param == NULL) {
        printf("FilterParam  is null\n");
        return -1;
    }

    // createPbvrFilterInfo(info, this);
    delete pbvr_filter;
    pbvr_filter = NULL;
    this->m_pbvrfilter = pbvr_filter;

    return 0;
}

/**
 * フィルタ情報を取得する.
 * @return		フィルタ情報
 */
pbvr::filter::PbvrFilter* PbvrFilterInformationFile::getPbvrFilter() const
{
    return this->m_pbvrfilter;
}

/**
 * フィルタファイルリスト情報を設定する.
 * @param list		フィルタファイルリスト情報
 */
void PbvrFilterInformationFile::setFilterInfoList(std::vector<FilterInformationFile*> *list)
{
    this->m_list = list;
}

/**
 * サブボリュームのノード数を取得する.
 * @param subvolume		サブボリュームID
 * @return		サブボリュームのノード数
 */
unsigned int PbvrFilterInformationFile::getNodesSubvolume(int subvolume) const
{
    if (subvolume >= this->m_nvolumes) return 0;
    return this->m_nodes_per_subvolume[subvolume];
}

/**
 * サブボリュームの接続数を取得する.
 * @param subvolume		サブボリュームID
 * @return		サブボリュームの接続数
 */
unsigned int PbvrFilterInformationFile::getConnsSubvolume(int subvolume) const
{
    if (subvolume >= this->m_nvolumes) return 0;
    return this->m_conns_per_subvolume[subvolume];
}

/**
 * サブボリュームの要素数を取得する.
 * @param subvolume		サブボリュームID
 * @return		サブボリュームの要素数
 */
unsigned int PbvrFilterInformationFile::getElemsSubvolume(int subvolume) const
{
    if (subvolume >= this->m_nvolumes) return 0;
    return this->m_elems_per_subvolume[subvolume];
}

/**
 * 接続ノードIDを取得する.:未使用
 * @param connectid		接続ID
 * @return		接続ノードID
 */
int PbvrFilterInformationFile::getConnection(int connectid) const
{

    /****  modify 2017/03/20 : データコピーを止める。PbvrFilterInfo *infoから取得する  *****
    if (connectid >= this->m_pbvr_connections.size()) return 0;
    return this->m_pbvr_connections[connectid];
    ***********************/
    if (this->m_pbvrfilter == NULL) return 0;
    if (this->m_pbvrfilter->getPbvrFilterInfo() == NULL) return 0;
    if (connectid >= this->m_pbvrfilter->getPbvrFilterInfo()->m_pbvr_connections.size()) return 0;
    return this->m_pbvrfilter->getPbvrFilterInfo()->m_pbvr_connections[connectid];
}

/**
 * サブボリュームの接続数を取得する.
 * @param subvolume		サブボリュームID
 * @return		サブボリュームの接続数
 */
int PbvrFilterInformationFile::getConnectionCount(int subvolume) const
{
    /****  modify 2017/03/20 : データコピーを止める。PbvrFilterInfo *infoから取得する  *****
    if (subvolume >= this->m_pbvr_connect_count.size()) return 0;
    return this->m_pbvr_connect_count[subvolume];
    ******************/
    if (this->m_pbvrfilter == NULL) return 0;
    if (this->m_pbvrfilter->getPbvrFilterInfo() == NULL) return 0;
    if (subvolume >= this->m_pbvrfilter->getPbvrFilterInfo()->m_pbvr_connect_count.size()) return 0;
    return this->m_pbvrfilter->getPbvrFilterInfo()->m_pbvr_connect_count[subvolume];
}

/**
 * 読込対象のサブボリュームIDであるかチェックする.
 * @param subvolume		読込対象のサブボリュームID
 * @return		true=読込対象のサブボリュームIDである
 */
bool PbvrFilterInformationFile::hasSubvolmueId(int subvolume) const
{
    /****  modify 2017/03/20 : データコピーを止める。PbvrFilterInfo *infoから取得する  *****
    if (subvolume >= this->m_pbvr_subvolumeids.size()) return false;
    std::vector<int>::const_iterator itr;
    for (itr = this->m_pbvr_subvolumeids.begin(); itr != this->m_pbvr_subvolumeids.end(); ++itr) {
        if ( subvolume == (*itr) ) {
            return true;
        }
    }
    return false;
    ***************/

    if (this->m_pbvrfilter == NULL) return false;
    if (this->m_pbvrfilter->getPbvrFilterInfo() == NULL) return false;
    if (subvolume >= this->m_pbvrfilter->getPbvrFilterInfo()->m_pbvr_subvolumeids.size()) return false;
    std::vector<int>::const_iterator itr;
    for (itr = this->m_pbvrfilter->getPbvrFilterInfo()->m_pbvr_subvolumeids.begin(); itr != this->m_pbvrfilter->getPbvrFilterInfo()->m_pbvr_subvolumeids.end(); ++itr) {
        if ( subvolume == (*itr) ) {
            return true;
        }
    }
    return false;
}

/**
 * サブボリューム数を取得する
 * @return		サブボリューム数
 */
int PbvrFilterInformationFile::getNumSubvoume() const
{
    return this->m_nvolumes;
}

/**
 * サブボリュームのノードIDを取得する
 * @param n		インデックス
 * @return		ノードID
 */
int PbvrFilterInformationFile::getSubvolumeNode(int index) const
{
    if (index >= this->m_prov_len) return 0;
    return this->m_subvolume_nodes[index];
}

/**
 * 要素タイプを取得する.
 * @return		要素タイプ
 */
int PbvrFilterInformationFile::getElementType() const
{
    return this->m_element_type;
}

/**
 * 読み込み対象のサブボリュームであるかチェックする.
 * @param subvols	サブボリュームID
 * @param rank		ランク番号
 * @param nprocs	プロセス数
 * @return		true=読み込み対象のサブボリューム
 */
bool PbvrFilterInformationFile::isPbvrSubvolume(int subvols, int rank, int nprocs) const {
    if (this->m_pbvrfilter == NULL) return false;
    if (this->m_pbvrfilter->getPbvrFilterInfo() == NULL) return false;
    if (nprocs == 1) return true;

    pbvr::filter::PbvrFilterInfo *info = this->m_pbvrfilter->getPbvrFilterInfo();
    int numpe_v = info->numpe_v;
    int mype_v = info->mype_v;
    if (numpe_v == 0) return false;
    if (numpe_v == 1) return true;

    return this->m_pbvrfilter->isWriteMpiProcess(subvols);
}


/**
 * 読み込み対象のサブボリュームであるかチェックする.
 * @param subvols	サブボリュームID
 * @return		true=読み込み対象のサブボリューム
 */
bool PbvrFilterInformationFile::isPbvrSubvolume(int subvols) const {
    if (this->m_pbvrfilter == NULL) return false;
    if (this->m_pbvrfilter->getPbvrFilterInfo() == NULL) return false;

    int rank = 0;
    int nprocs = 1;
#ifndef CPU_VER
        MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
        MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#endif
    if (nprocs == 1) return true;

    return this->isPbvrSubvolume(subvols, rank, nprocs);
}


/**
 * 指定ステップ番号の物理量データを読み込みを行う。
 * @param step		ステップ番号(フィルタパラメータファイル記述(start_step〜end_step)
 * @return     0=true
 */
int PbvrFilterInformationFile::loadMpiSubvolume(int step) const
{
    if (this->m_pbvrfilter == NULL) return -1;
    int ret = this->m_pbvrfilter->loadMpiSubvolume(step, this->getElementType());

    return ret;
}

/**
 * 指定ステップ番号の物理量データの読込後処理を行う。
 */
void PbvrFilterInformationFile::releaseMpiSubvolume() const
{
    if (this->m_pbvrfilter == NULL) return;
    this->m_pbvrfilter->releaseMpiSubvolume();
    return;
}


pbvr::filter::DIVISION_TYPE PbvrFilterInformationFile::getFilterDivision() const {

    pbvr::filter::DIVISION_TYPE type = pbvr::filter::UNKNOWN_DIVISION;
    if (this->m_pbvrfilter == NULL) return type;
    if (this->m_pbvrfilter->getPbvrFilterInfo() == NULL) return type;
    if (this->m_pbvrfilter->getPbvrFilterInfo()->m_param == NULL) return type;
    pbvr::filter::PbvrFilterInfo *info = this->m_pbvrfilter->getPbvrFilterInfo();
    pbvr::filter::FilterParam *param = this->m_pbvrfilter->getPbvrFilterInfo()->m_param;

    return param->division_type;
}

/**
 * コンストラクタ
 */
PbvrFilterInformationList::PbvrFilterInformationList() {

}


/**
 * デストラクタ
 */
PbvrFilterInformationList::~PbvrFilterInformationList() {

    {
        std::vector<PbvrFilterInformationFile*>::iterator itr;
        for (itr = this->m_pbvrlist.begin(); itr != this->m_pbvrlist.end(); ++itr) {
            PbvrFilterInformationFile* pbvrinfo = *itr;
            pbvrinfo->finalizePbvrFilterInfo();
            if (pbvrinfo != NULL) delete pbvrinfo;
        }
        this->m_pbvrlist.clear();
    }
}


int PbvrFilterInformationList::loadPFL( const std::string& filename )
{
    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    kvs::File pfl( filename );
    if ( filename.size() < 4 || ! pfl.isExisted() ) return -1;

    if ( filename.substr( filename.size() - 3 ) != "pfl" )
    {
        PbvrFilterInformationFile *fi = new PbvrFilterInformationFile();
        fi->setFilterInfoList(&m_list);
        PBVR_TIMER_STA( 511 );
        if ( fi->loadPFI( filename ) < 0 ) return -1;
        PBVR_TIMER_END( 511 );
        // m_list.push_back( fi );
        this->m_pbvrlist.push_back( fi );
    }
    else {
        std::ifstream fin( filename.c_str(), std::ios::in );
        if ( ! fin ) return -1;
        std::string fbuff;
        if ( ! getline( fin, fbuff ) ) return -1;
        if ( fbuff != std::string( "#PBVR FILTER FILES" ) ) return -1;
        while ( getline( fin, fbuff ) )
        {
            kvs::File pfi( fbuff );
            if ( fbuff.size() < 4 ) continue;
            if ( ! pfi.isExisted() )
            {
                std::string xpath = pfl.pathName( true ) + pfl.Separator() + pfi.filePath( false );
                kvs::File xpfi( xpath );
                if ( ! xpfi.isExisted() ) continue;
                fbuff = xpath;
            }
            PbvrFilterInformationFile *fi = new PbvrFilterInformationFile();
            fi->setFilterInfoList(&m_list);
            PBVR_TIMER_STA( 510 );
            fi->loadPFI( fbuff );
            PBVR_TIMER_END( 510 );

            // m_list.push_back( fi );
            this->m_pbvrlist.push_back( fi );

        } // end of while()
        fin.close();
    }

    std::vector<FilterInformationFile*>::iterator itr;
    for (itr=this->m_list.begin(); itr!=this->m_list.end(); itr++) {
        FilterInformationFile* fi = (*itr);
        if ( itr==this->m_list.begin() ) {
            m_total_number_nodes = fi->m_number_nodes;
            m_total_number_elements = fi->m_number_elements;
            m_total_number_files = fi->m_number_files;
            m_total_start_steps = fi->m_start_step;
            m_total_end_step = fi->m_end_steps;
            m_total_number_steps = fi->m_number_steps;
            m_total_number_subvolumes = fi->m_number_subvolumes;
            m_total_min_object_coord = fi->m_min_object_coord;
            m_total_max_object_coord = fi->m_max_object_coord;
            m_total_min_subvolume_coord = fi->m_min_subvolume_coord;
            m_total_max_subvolume_coord = fi->m_max_subvolume_coord;
            m_total_min_value = fi->m_min_value;
            m_total_max_value = fi->m_max_value;
            m_total_number_ingredients = fi->m_number_ingredients;
        }
        else {
            m_total_number_nodes += fi->m_number_nodes;
            m_total_number_elements += fi->m_number_elements;
            m_total_number_files += fi->m_number_files;
            m_total_start_steps = std::min( m_total_start_steps, fi->m_start_step );
            m_total_end_step = std::max( m_total_end_step, fi->m_end_steps );
            m_total_number_steps = m_total_end_step - m_total_start_steps + 1;
            m_total_number_subvolumes += fi->m_number_subvolumes;
            m_total_min_object_coord[0]
                = std::min( m_total_min_object_coord[0], fi->m_min_object_coord[0] );
            m_total_min_object_coord[1]
                = std::min( m_total_min_object_coord[1], fi->m_min_object_coord[1] );
            m_total_min_object_coord[2]
                = std::min( m_total_min_object_coord[2], fi->m_min_object_coord[2] );
            m_total_max_object_coord[0]
                = std::max( m_total_max_object_coord[0], fi->m_max_object_coord[0] );
            m_total_max_object_coord[1]
                = std::max( m_total_max_object_coord[1], fi->m_max_object_coord[1] );
            m_total_max_object_coord[2]
                = std::max( m_total_max_object_coord[2], fi->m_max_object_coord[2] );
            std::copy( fi->m_min_subvolume_coord.begin(), fi->m_min_subvolume_coord.end(),
                       std::back_inserter( m_total_min_subvolume_coord ) );
            std::copy( fi->m_max_subvolume_coord.begin(), fi->m_max_subvolume_coord.end(),
                       std::back_inserter( m_total_max_subvolume_coord ) );
            m_total_min_value = std::min( m_total_min_value, fi->m_min_value );
            m_total_max_value = std::max( m_total_max_value, fi->m_max_value );
            m_total_number_ingredients = std::max( m_total_number_ingredients, fi->m_number_ingredients );

        }
    }

    m_total_ingredient.resize( m_total_number_ingredients );
    for (int32_t i = 0; i < m_total_number_ingredients; i++)
    {
        m_total_ingredient[i].m_min = FLT_MAX;
        m_total_ingredient[i].m_max = -FLT_MAX;
    }
    for ( int32_t idx = 0; idx < m_list.size(); idx++ )
    {
        calculate_ingredient_min_max( m_list[idx], &m_total_ingredient );
    }

    return m_list.size();
}

/**
 * フィルタパラメータファイル（フィルタライブラリ）からの読込であるかチェックする.
 * @return	フィルタパラメータファイルからの読込
 */
bool PbvrFilterInformationList::isPbvrFilterInfo() const
{
    std::vector<FilterInformationFile*>::const_iterator itr;
    for (itr=this->m_list.begin(); itr!=this->m_list.end(); itr++) {
        FilterInformationFile* fi = (*itr);
        if (fi->isPbvrFilterInfo()) {
            return true;
        }
    }

    return false;
}

