/*
 * PbvrFilterInformation.h
 *
 *  Created on: 2016/12/02
 *      Author: hira
 */

#ifndef PBVRFILTERINFORMATION_H_
#define PBVRFILTERINFORMATION_H_

#include "FilterInformation.h"
#include "PbvrFilter.h"

class PbvrFilterInformationFile : public FilterInformationFile
{
public:
    PbvrFilterInformationFile();
    virtual ~PbvrFilterInformationFile();

public:
    virtual int loadPFI( const std::string& filename );
    virtual bool isPbvrFilterInfo() const;
    virtual pbvr::filter::PbvrFilter* getPbvrFilter() const;
    void setFilterInfoList(std::vector<FilterInformationFile*> *list);
    void finalizePbvrFilterInfo();
    unsigned int getNodesSubvolume(int subvolume) const;
    unsigned int getConnsSubvolume(int subvolume) const;
    unsigned int getElemsSubvolume(int subvolume) const;
    int getConnection(int subvolume) const;
    int getConnectionCount(int subvolume) const;
    int getNumSubvoume() const;
    int getSubvolumeNode(int subvolume) const;
    int getElementType() const;
    virtual bool isPbvrSubvolume(int subvols, int rank, int nprocs) const;
    virtual bool isPbvrSubvolume(int subvols) const;
    bool hasSubvolmueId(int subvolume) const;
    virtual int loadMpiSubvolume(int step) const;
    virtual void releaseMpiSubvolume() const;
    virtual pbvr::filter::DIVISION_TYPE getFilterDivision() const;

protected:
    PbvrFilterInformationFile* createPbvrFilterInfo(pbvr::filter::PbvrFilterInfo *info, PbvrFilterInformationFile* pbvrinfo = NULL);
    int setPbvrFilterInfoList(std::vector<pbvr::filter::PbvrFilterInfo*> &info_list);
private:
    int element_type_num;
    pbvr::filter::PbvrFilter *m_pbvrfilter;
    std::vector<FilterInformationFile*> *m_list;
    int m_nvolumes;
    unsigned int *m_elems_per_subvolume;
    unsigned int *m_conns_per_subvolume;
    unsigned int *m_nodes_per_subvolume;
    int *m_subvolume_nodes;

    /****  modify 2017/03/20 : データコピーを止める。PbvrFilterInfo *infoから取得する  *****
    std::vector<int> m_pbvr_connections;      // ノードの接続情報:subvolumeすべて
    std::vector<int> m_pbvr_connect_count;    // ノードの接続情報のsubvolume毎のm_pbvr_connectionsの挿入数
    std::vector<int> m_pbvr_subvolumeids;     // ランクが担当したサブボリュームID(0〜)
    **************************/

    char m_element_type;
    int m_prov_len;
};

class PbvrFilterInformationList: public FilterInformationList {
public:
    PbvrFilterInformationList();
    virtual ~PbvrFilterInformationList();
    virtual int loadPFL( const std::string& filename );
    virtual bool isPbvrFilterInfo() const;

private:
    std::vector<PbvrFilterInformationFile*> m_pbvrlist;
};

#endif /* PBVRFILTERINFORMATION_H_ */
