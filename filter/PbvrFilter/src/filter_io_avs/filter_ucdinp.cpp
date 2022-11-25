#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "filter_io_avs/FilterIoAvs.h"

namespace pbvr {
namespace filter {


/*****************************************************************************/
/*
 * UCD コントロールファイル読込み
 *
 */
int FilterIoAvs::read_inp_file( void )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char* filename = info->filename;

    int          state = RTC_NG;
    FILE        *ifs = NULL;
    char        *ch;
    char         line [LINELEN_MAX];
//    char         alloc_flag;
    int          i;
    int          count;
//    int          do_step;

    count = 0;
//    do_step = 0;
//    alloc_flag = 0;

    sprintf(filename, "%s/%s",param->in_dir,param->ucd_inp );
    ifs = fopen(filename,"rb");
    if(ifs == NULL) {
        LOGOUT(FILTER_ERR,(char*)"Can't open file : %s\n",filename);
        return state;
    }

    param->nstep = 1;

    info->m_ucdbin = 0;
    count = 0;
    while(fgets(line,LINELEN_MAX,ifs) != NULL) {
        ch=line;
        if(*ch=='\n'){
            continue;
        }
        if(*ch=='#'){
            continue;
        }
        if(count==0) {
            if(strstr(line,"data_geom")){
                info->m_cycle_type = 1;
                info->m_ucdbin = 1;
            }else if(strstr(line,"data")){
                info->m_cycle_type = 2;
                info->m_ucdbin = 1;
            }else if(strstr(line,"geom")){
                info->m_cycle_type = 3;
                info->m_ucdbin = 1;
            }else{
                param->nstep = atoi(line);
                info->m_ucdbin = 0;
            }
        }
        count++;
    }
    fseek(ifs,0,SEEK_SET);

    /* エラー */
    if(info->m_ucdbin){
        LOGOUT(FILTER_ERR, (char*)"Not support ucd-binary by avsinp-file : %s\n",filename);
        return state;
    }

    if(info->m_ucdbin) {
        info->m_ucdbinfile = (char**)malloc(sizeof(char*)*count);
        info->m_ucdbinfile_size = count;
        for(i=0;i<count;i++){
            info->m_ucdbinfile[i] = (char*)malloc(sizeof(char)*LINELEN_MAX);
            memset(info->m_ucdbinfile[i],0x00,sizeof(char)*LINELEN_MAX);
        }
        count = 0;
        while(fgets(line,LINELEN_MAX,ifs) != NULL) {
            ch=line;
            if(*ch=='\n'){
                continue;
            }
            if(*ch=='#'){
                continue;
            }
            if(count==0) {
                if(strstr(line,"data_geom")){
                    info->m_cycle_type = 1;
                }else if(strstr(line,"data")){
                    info->m_cycle_type = 2;
                }else if(strstr(line,"geom")){
                    info->m_cycle_type = 3;
                }else{
                    param->nstep = atoi(line);
                    ;
                }
            }else{
                for(i=0;i<LINELEN_MAX;i++){
                    if(line[i]=='\n'){
                        break;
                    }
                    info->m_ucdbinfile[count-1][i]=line[i];
                }
            }
            count++;
        }
        fclose(ifs);
    }else{
        count = 0;
        while(fgets(line,LINELEN_MAX,ifs) != NULL) {
            ch=line;
            if(*ch=='\n'){
                continue;
            }
            if(*ch=='#'){
                continue;
            }
            if(count==0) {
                param->nstep = atoi(line);
                info->m_ucdbin = 0;
            }else{
                if(strstr(line,"data_geom")){
                    info->m_cycle_type = 1;
                }else if(strstr(line,"data")){
                    info->m_cycle_type = 2;
                }else if(strstr(line,"geom")){
                    info->m_cycle_type = 3;
                }else{
                    //;
                    /*
                    ここでサイクルタイプの記述がなかった場合旧式フォーマットと判定
                    旧フォーマットであった場合はステップ数を1とする(もし2以上のステップ数が来た場合は別途処理が必要になる。
                    またサイクルタイプはdataで固定される。
                    */
                    if(info->m_cycle_type == 0){
                        std::cout << "Detected an old type of AVS format." << std::endl;
                        param->nstep = 1;
                        info->m_cycle_type = 2;
                        info->m_old_avs_format = 1;
                    }
                }
            }
            count++;
            if(count>1){
                break;
            }
        }
        info->m_ucd_ascii_fp = ifs;
    }

    state = RTC_OK;
    return(state);
}

/*****************************************************************************/
/*
 * ASCII形式のUCD GEOM読込み
 *
 */
int FilterIoAvs::skip_ucd_ascii( int step )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    char* filename = info->filename;

    FILE        *ifs = NULL;
    char         line [LINELEN_MAX];
    char         buf[10];
    char        *ch;
    char        *value=NULL;

    ifs = info->m_ucd_ascii_fp;
    if(ifs == NULL) {
        LOGOUT(FILTER_ERR, (char*)"Can't open file : %s\n",filename);
        return RTC_NG;
    }

    memset(buf,0x000,10);
    sprintf(buf,"step%d",step);
    while(fgets(line,LINELEN_MAX,ifs) != NULL) {
        ch=line;
        if(*ch=='\n'){
            continue;
        }
        if(*ch=='#'){
            continue;
        }
        if(strstr(line,buf)){
            value = strtok(line," ");
            value = strtok(NULL," ");
            break;
        }
    }
    return RTC_OK;
}

/*****************************************************************************/
/*
 * ASCII形式のUCD GEOM読込み
 *
 */
int FilterIoAvs::read_ucd_ascii_geom( void )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char* filename = info->filename;
    float *m_coords = NULL;
    NodeInf *min = &info->node_min;
    NodeInf *max = &info->node_max;

    int          state = RTC_NG;
    FILE        *ifs = NULL;
    char         line [LINELEN_MAX];
    char        *ch;
    char        *value=NULL;
    char         alloc_flag;
    char         type;
    int          count;
    int          size;
    int          do_step;
    int          conn_count = 0;
    int          conn_size = 0;
    int          nd;
    int          em;

    count = 0;
    do_step = 0;
    alloc_flag = 0;

    ifs = info->m_ucd_ascii_fp;
    if(ifs == NULL) {
        LOGOUT(FILTER_ERR, (char*)"Can't open file : %s\n",filename);
        return state;
    }

        count = 0;
        if(info->m_old_avs_format == 1){
            fseek(ifs,0,SEEK_SET);
        }
        while(fgets(line,LINELEN_MAX,ifs) != NULL) {
            ch=line;
            if(*ch=='\n'){
                continue;
            }
            if(*ch=='#'){
                continue;
            }
            value = strtok(line," ");
            info->m_nnodes = atoi(value);
            value = strtok(NULL," ");
            info->m_nelements = atoi(value);
            break;
        }
        if (info->m_ids != NULL) free(info->m_ids);
        info->m_ids         = (int *)malloc(sizeof(int)*info->m_nnodes);
        if(NULL==info->m_ids) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(info->m_ids).\n");
            goto garbage;
        }
        if (info->m_coords != NULL) free(info->m_coords);
        info->m_coords      = (float *)malloc(sizeof(float)*info->m_nnodes*3);
        if(NULL==info->m_coords) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_coords).\n");
            goto garbage;
        }
        m_coords = info->m_coords;
        if (info->m_elementids != NULL) free(info->m_elementids);
        if (info->m_materials != NULL) free(info->m_materials);
        if (info->m_elemtypes != NULL) free(info->m_elemtypes);
        if (info->m_connections != NULL) free(info->m_connections);
        info->m_elementids  = (int *)malloc(sizeof(int)*info->m_nelements);
        info->m_materials   = (int *)malloc(sizeof(int)*info->m_nelements);
        info->m_elemtypes   = (char *)malloc(sizeof(char)*info->m_nelements);
        info->m_connections = (int *)malloc(sizeof(int)*info->m_nelements*20);
        if(NULL==info->m_connections) {
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_connections).\n");
            goto garbage;
        }
        count = 0;
        while(fgets(line,LINELEN_MAX,ifs) != NULL) {
            ch=line;
            if(*ch=='\n'){
                continue;
            }
            if(*ch=='#'){
                continue;
            }
            ChangeChar(line,'\t',' ');
            value = strtok(line," ");
            info->m_ids[count] = atoi(value);
            value = strtok(NULL," ");
            m_coords[count*3] = atof(value);
            value = strtok(NULL," ");
            m_coords[count*3+1] = atof(value);
            value = strtok(NULL," ");
            m_coords[count*3+2] = atof(value);
            count++;
            if(count>=info->m_nnodes){
                break;
            }
        }

        param->nodesearch = (char)0;
        for(nd=0;nd<info->m_nnodes;nd++){
            if(info->m_ids[nd] != (nd+1)){
                param->nodesearch = (char)1;
                LOGOUT(FILTER_LOG, (char*)"     mids[%10d]        : %d\n",nd,info->m_ids[nd]       );
                LOGOUT(FILTER_LOG, (char*)"     param->nodesearch : %d\n",param->nodesearch);
                break;
            }
        }
        min->x = max->x = m_coords[0];
        min->y = max->y = m_coords[1];
        min->z = max->z = m_coords[2];
        for(nd=1; nd<info->m_nnodes; nd++){
            if (min->x > m_coords[nd*3  ]) min->x = m_coords[nd*3  ];
            if (min->y > m_coords[nd*3+1]) min->y = m_coords[nd*3+1];
            if (min->z > m_coords[nd*3+2]) min->z = m_coords[nd*3+2];

            if (max->x < m_coords[nd*3  ]) max->x = m_coords[nd*3  ];
            if (max->y < m_coords[nd*3+1]) max->y = m_coords[nd*3+1];
            if (max->z < m_coords[nd*3+2]) max->z = m_coords[nd*3+2];
        }
        LOGOUT(FILTER_LOG, (char*)"|||| max -> %8.2f, %8.2f, %8.2f\n",max->x,max->y,max->z);
        LOGOUT(FILTER_LOG, (char*)"|||| min -> %8.2f, %8.2f, %8.2f\n",min->x,min->y,min->z);

        count = 0;
        conn_size = 0;
        while(fgets(line,LINELEN_MAX,ifs) != NULL) {
            ch=line;
            if(*ch=='\n'){
                continue;
            }
            if(*ch=='#'){
                continue;
            }
            ChangeChar(line,'\t',' ');
            value = strtok(line," ");
            info->m_elementids[count] = atoi(value);
            value = strtok(NULL," ");
            info->m_materials[count] = atoi(value);
            value = strtok(NULL," ");
            if(strstr(value,"hex2")){
                info->m_elemtypes[count] = 14;
                size = 20;
            }else if(strstr(value,"prism2")){
                info->m_elemtypes[count] = 13;
                size = 15;
            }else if(strstr(value,"pyr2")){
                info->m_elemtypes[count] = 12;
                size = 13;
            }else if(strstr(value,"tet2")){
                info->m_elemtypes[count] = 11;
                size = 10;
            }else if(strstr(value,"quad2")){
                info->m_elemtypes[count] = 10;
                size = 8;
            }else if(strstr(value,"tri2")){
                info->m_elemtypes[count] = 9;
                size = 6;
            }else if(strstr(value,"line2")){
                info->m_elemtypes[count] = 8;
                size = 3;
            }else if(strstr(value,"hex")){
                info->m_elemtypes[count] = 7;
                size = 8;
            }else if(strstr(value,"prism")){
                info->m_elemtypes[count] = 6;
                size = 6;
            }else if(strstr(value,"pyr")){
                info->m_elemtypes[count] = 5;
                size = 5;
            }else if(strstr(value,"tet")){
                info->m_elemtypes[count] = 4;
                size = 4;
            }else if(strstr(value,"quad")){
                info->m_elemtypes[count] = 3;
                size = 4;
            }else if(strstr(value,"tri")){
                info->m_elemtypes[count] = 2;
                size = 3;
            }else if(strstr(value,"line")){
                info->m_elemtypes[count] = 1;
                size = 2;
            }else{
                info->m_elemtypes[count] = 0;
                size = 1;
            }
            info->m_elemcoms = size;
            info->m_element_type = info->m_elemtypes[count];
            conn_count = 0;
            while(conn_count<size) {
                value = strtok(NULL," ");
                info->m_connections[conn_size] = atof(value);
                conn_size++;
                conn_count++;
            }
            info->m_types[(int)info->m_elemtypes[count]]++;
            count++;
            if(count>=info->m_nelements){
                break;
            }
        }
        info->m_connectsize = conn_size;

        type = info->m_elemtypes[0];
        param->mult_element_type = (char)0;
        for(em=1;em<info->m_nelements;em++){
            if(type!=info->m_elemtypes[em]){
                param->mult_element_type = (char)1;
                break;
            }
        }

        LOGOUT(FILTER_LOG, (char*)"|||| info->m_nelements       : %10d\n",   info->m_nelements      );
        LOGOUT(FILTER_LOG, (char*)"|||| m_connectsize     : %10d\n",   info->m_connectsize    );
        LOGOUT(FILTER_LOG, (char*)"|||| Trai    (type 2)  : %10d\n",   info->m_types[ 2]      );
        LOGOUT(FILTER_LOG, (char*)"|||| Quad    (type 3)  : %10d\n",   info->m_types[ 3]      );
        LOGOUT(FILTER_LOG, (char*)"|||| Tetra   (type 4)  : %10d\n",   info->m_types[ 4]      );
        LOGOUT(FILTER_LOG, (char*)"|||| Pyramid (type 5)  : %10d\n",   info->m_types[ 5]      );
        LOGOUT(FILTER_LOG, (char*)"|||| Prism   (type 6)  : %10d\n",   info->m_types[ 6]      );
        LOGOUT(FILTER_LOG, (char*)"|||| Hexa    (type 7)  : %10d\n",   info->m_types[ 7]      );
        LOGOUT(FILTER_LOG, (char*)"|||| Trai2   (type 9)  : %10d\n",   info->m_types[ 9]      );
        LOGOUT(FILTER_LOG, (char*)"|||| Quad2   (type10)  : %10d\n",   info->m_types[10]      );
        LOGOUT(FILTER_LOG, (char*)"|||| Tetra2  (type11)  : %10d\n",   info->m_types[11]      );
        LOGOUT(FILTER_LOG, (char*)"|||| Pyramid2(type12)  : %10d\n",   info->m_types[12]      );
        LOGOUT(FILTER_LOG, (char*)"|||| Prism2  (type13)  : %10d\n",   info->m_types[13]      );
        LOGOUT(FILTER_LOG, (char*)"|||| Hexa2   (type14)  : %10d\n",   info->m_types[14]      );

    state = RTC_OK;
    return(state);
garbage:
    return(RTC_NG);
}

/*****************************************************************************/
/*
 * ASCII形式のUCD GEOM読込み
 *
 */
int FilterIoAvs::skip_ucd_ascii_geom( void )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    int          state = RTC_NG;
    FILE        *ifs = NULL;
    char        *ch = NULL;
    char        *value = NULL;
    char         line [LINELEN_MAX];
    int          count;
    int          nnode;
    int          nelement;

    count = 0;
    nnode = 0;
    nelement = 0;

    ifs = info->m_ucd_ascii_fp;
    if(NULL==ifs){
        goto garbage;
    }

        count = 0;
        //旧フォーマット分岐
        if(info->m_old_avs_format == 1){
            fseek(ifs,0,SEEK_SET);
        }
        while(fgets(line,LINELEN_MAX,ifs) != NULL) {
            ch=line;
            if(*ch=='\n'){
                continue;
            }
            if(*ch=='#'){
                continue;
            }
            value = strtok(line," ");
            nnode = atoi(value);
            value = strtok(NULL," ");
            nelement = atoi(value);
            break;
        }
        count = 0;
        while(fgets(line,LINELEN_MAX,ifs) != NULL) {
            ch=line;
            if(*ch=='\n'){
                continue;
            }
            if(*ch=='#'){
                continue;
            }
            count++;
            if(count==(nnode+nelement)){
                break;
            }
        }

    state = RTC_OK;
    return(state);
garbage:
    return(RTC_NG);
}

/*****************************************************************************/
/*
 * ASCII形式のUCD VALUE読込み
 *
 */
int FilterIoAvs::read_ucd_ascii_value( int step_loop )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char* filename = info->filename;

    int          state = RTC_NG;
    FILE        *ifs = NULL;
    char         line [LINELEN_MAX];
    char        *ch;
    char        *value=NULL;
    char         alloc_flag;
    int          i,j,k,m;
    int          count;
    int          size;
    int          eid;
    int          nkind;
    int          do_step;
    char        **pme_name = NULL;
    char        **pme_unit = NULL;
    char        **pmn_name = NULL;
    char        **pmn_unit = NULL;
    char         *mn_name=NULL;
    char         *mn_unit=NULL;
    char         *me_name=NULL;
    char         *me_unit=NULL;
    int          *mn_veclen=NULL;
    int          *me_veclen=NULL;
    float        *node_value=NULL;
    float        *elem_value=NULL;
    float        *fvalue = NULL;
    int          *effe_num   = NULL;

    count = 0;
    do_step = 0;
    alloc_flag = 0;

    sprintf(filename, "%s/%s",param->in_dir,param->ucd_inp );
    ifs = info->m_ucd_ascii_fp;
    if(ifs == NULL) {
        LOGOUT(FILTER_ERR, (char*)"Can't open file : %s\n",filename);
        return state;
    }
    int file_line = 0;
        count = 0;
        if(info->m_old_avs_format == 1){
            file_line = ftell(ifs);
            fseek(ifs,0,SEEK_SET);
        }
        while(fgets(line,LINELEN_MAX,ifs) != NULL) {
            ch=line;
            if(*ch=='\n'){
                continue;
            }
            if(*ch=='#'){
                continue;
            }
            ChangeChar(line,'\t',' ');
            value = strtok(line," ");
	    
            if(info->m_old_avs_format == 1){
                value = strtok(NULL," ");
                value = strtok(NULL," ");
            }

            info->m_nnodekinds = atoi(value);
            value = strtok(NULL," ");
            info->m_nelemkinds = atoi(value);
            break;
        }

        if(info->m_old_avs_format == 1){
            fseek(ifs,file_line,SEEK_SET);
        }
	
        if(info->m_nnodekinds != 0){
            while(fgets(line,LINELEN_MAX,ifs) != NULL) {
                ch=line;
                if(*ch=='\n'){
                    continue;
                }
                if(*ch=='#'){
                    continue;
                }
                ChangeChar(line,'\t',' ');
                value = strtok(line," ");
                info->m_nnodecomponents = atoi(value);
                mn_veclen = (int*)malloc(sizeof(int)*info->m_nnodecomponents);
                for(i=0;i<info->m_nnodecomponents;i++){
                    value = strtok(NULL," ");
                    mn_veclen[i] = atoi(value);
                }
                break;
            }

            pmn_name = (char**)malloc(sizeof(char*)*info->m_nnodecomponents);
            for(i=0;i<info->m_nnodecomponents;i++){
                pmn_name[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
            }
            pmn_unit = (char**)malloc(sizeof(char*)*info->m_nnodecomponents);
            for(i=0;i<info->m_nnodecomponents;i++){
                pmn_unit[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
            }
            count = 0;
            while(fgets(line,LINELEN_MAX,ifs) != NULL) {
                ch=line;
                if(*ch=='\n'){
                    continue;
                }
                if(*ch=='#'){
                    continue;
                }
                ChangeChar(line,'\t',' ');
                value = strtok(line,",");
                value = strtok(NULL,",");
                count++;
                if(count>=info->m_nnodecomponents){
                    break;
                }
            }

            if (info->m_ids != NULL) free(info->m_ids);
            info->m_ids = (int*)malloc(sizeof(int)*info->m_nnodes);
            node_value = (float*)malloc(sizeof(float)*info->m_nnodes*info->m_nnodekinds);
            count = 0;
            while(fgets(line,LINELEN_MAX,ifs) != NULL) {
                ch=line;
                if(*ch=='\n'){
                    continue;
                }
                if(*ch=='#'){
                    continue;
                }
                ChangeChar(line,'\t',' ');
                value = strtok(line," ");
                info->m_ids[count] = atoi(value);
                for(i=0;i<info->m_nnodekinds;i++){
                    value = strtok(NULL," ");
                    node_value[count*info->m_nnodekinds+i] =  atof(value);
                }
                count++;
                if(count>=info->m_nnodes){
                    break;
                }
            }
        }
        if(info->m_nelemkinds != 0){
            while(fgets(line,LINELEN_MAX,ifs) != NULL) {
                ch=line;
                if(*ch=='\n'){
                    continue;
                }
                if(*ch=='#'){
                    continue;
                }
                ChangeChar(line,'\t',' ');
                value = strtok(line," ");
                info->m_nelemcomponents = atoi(value);
                me_veclen = (int*)malloc(sizeof(int)*info->m_nelemcomponents);
                for(i=0;i<info->m_nelemcomponents;i++){
                    value = strtok(NULL," ");
                    me_veclen[i] = atoi(value);
                }
                break;
            }

            pme_name = (char**)malloc(sizeof(char*)*info->m_nelemcomponents);
            for(i=0;i<info->m_nelemcomponents;i++){
                pme_name[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
            }
            pme_unit = (char**)malloc(sizeof(char*)*info->m_nelemcomponents);
            for(i=0;i<info->m_nelemcomponents;i++){
                pme_unit[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
            }
            count = 0;
            while(fgets(line,LINELEN_MAX,ifs) != NULL) {
                ch=line;
                if(*ch=='\n'){
                    continue;
                }
                if(*ch=='#'){
                    continue;
                }
                ChangeChar(line,'\t',' ');
                value = strtok(line,",");
                value = strtok(NULL,",");
                count++;
                if(count>=info->m_nelemcomponents){
                    break;
                }
            }
            if (info->m_elementids != NULL) free(info->m_elementids);
            info->m_elementids = (int*)malloc(sizeof(int)*info->m_nelements);
            elem_value = (float*)malloc(sizeof(float)*info->m_nelements*info->m_nelemkinds);
            count = 0;
            while(fgets(line,LINELEN_MAX,ifs) != NULL) {
                ch=line;
                if(*ch=='\n'){
                    continue;
                }
                if(*ch=='#'){
                    continue;
                }
                ChangeChar(line,'\t',' ');
                value = strtok(line," ");
                info->m_elementids[count] = atoi(value);
                for(i=0;i<info->m_nelemkinds;i++){
                    value = strtok(NULL," ");
                    elem_value[count*info->m_nelemkinds+i] =  atof(value);
                }
                count++;
                if(count>=info->m_nelements){
                    break;
                }
            }
        }

    info->m_nkinds = info->m_nnodekinds + info->m_nelemkinds;
    info->m_ncomponents = info->m_nnodecomponents + info->m_nelemcomponents;

    if(info->m_ncomponents>0){
        info->pm_names = (char**)malloc(sizeof(char*)*info->m_ncomponents);
        for(i=0;i<info->m_ncomponents;i++){
            info->pm_names[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }
        info->pm_units = (char**)malloc(sizeof(char*)*info->m_ncomponents);
        for(i=0;i<info->m_ncomponents;i++){
            info->pm_units[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }

        info->m_veclens = (int *)malloc(sizeof(int)*info->m_ncomponents );
        info->m_nullflags = (int *)malloc(sizeof(int)*info->m_nkinds );
        info->m_nulldatas = (float *)malloc(sizeof(float)*info->m_nkinds );
        if(info->m_nnodecomponents>0){
            nkind = 0;
            for(i=0;i<info->m_nnodecomponents;i++ ){
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[i][m] = pmn_name[i][m];
                    info->pm_units[i][m] = pmn_unit[i][m];
                }
                info->m_veclens[i] = mn_veclen[i];
                for(j=0;j<info->m_veclens[i];j++){
                    info->m_nullflags[nkind] = 0;
                    info->m_nulldatas[nkind] = 0;
                    nkind++;
                }
            }
            if(pmn_name!=NULL){
                for(i=0;i<info->m_nnodecomponents;i++){
                    free(pmn_name[i]);
                }
                free(pmn_name);
                pmn_name=NULL;
            }
            if(pmn_unit!=NULL){
                for(i=0;i<info->m_nnodecomponents;i++){
                    free(pmn_unit[i]);
                }
                free(pmn_unit);
                pmn_unit=NULL;
            }

            if(mn_name!=NULL)     free(mn_name);
            if(mn_unit!=NULL)     free(mn_unit);
            if(mn_veclen!=NULL)   free(mn_veclen);
            mn_name=NULL;
            mn_unit=NULL;
            mn_veclen=NULL;
        }
        if(info->m_nelemcomponents>0){
            nkind = 0;
            for(i=0;i<info->m_nelemcomponents;i++ ){
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[info->m_nnodecomponents+i][m] = pme_name[i][m];
                    info->pm_units[info->m_nnodecomponents+i][m] = pme_unit[i][m];
                }
                info->m_veclens[info->m_nnodecomponents+i] = me_veclen[i];
                for(j=0;j<me_veclen[i];j++){
                    info->m_nullflags[info->m_nnodecomponents+nkind] = 0;
                    info->m_nulldatas[info->m_nnodecomponents+nkind] = 0;
                }
            }
            if(pme_name!=NULL){
                for(i=0;i<info->m_nelemcomponents;i++){
                    free(pme_name[i]);
                }
                free(pme_name);
                pme_name=NULL;
            }
            if(pme_unit!=NULL){
                for(i=0;i<info->m_nelemcomponents;i++){
                    free(pme_unit[i]);
                }
                free(pme_unit);
                pme_unit=NULL;
            }
            if(me_name!=NULL)     free(me_name);
            if(me_unit!=NULL)     free(me_unit);
            if(me_veclen!=NULL)   free(me_veclen);
            me_name=NULL;
            me_unit=NULL;
            me_veclen=NULL;

            free(elem_value);
            elem_value = NULL;
        }
    }

    if(step_loop>=0){
        if(info->m_nnodekinds>0){
            for(i=0;i<info->m_nnodes;i++){
                for(j=0;j<info->m_nkinds;j++){
                    info->m_values[i*info->m_nkinds+j] = node_value[i*info->m_nnodekinds+j];
                }
            }
        }
        if(info->m_nelemkinds>0){
            fvalue = (float *)malloc(sizeof(float) * info->m_nelemkinds);
            effe_num = (int *)malloc(sizeof(int) * info->m_nelemkinds);
            if(NULL==fvalue||NULL==effe_num){
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(fvalues).\n");
                goto garbage;
            }
            for(i=0;i<info->m_nnodes;i++){
                for(k=0;k<info->m_nelemkinds;k++){
                    fvalue[k] = 0.0;
                    effe_num[k] = 0;
                }
                for(j=0;j<info->m_elem_in_node[i];j++){
                    eid = info->m_elem_in_node_array[info->m_elem_in_node_addr[i]+j] - 1;
                    for(k=0;k<info->m_nelemkinds;k++){
                        fvalue[k] = fvalue[k] + elem_value[eid*info->m_nelemkinds+k];
                        effe_num[k]++ ;
                    }
                }
                for(k=0;k<info->m_nelemkinds;k++){
                    if(effe_num[k]!=0){
                        info->m_values[i*info->m_nkinds+info->m_nnodekinds+k] = fvalue[k] / effe_num[k];
                    }else{
                        info->m_values[i*info->m_nkinds+info->m_nnodekinds+k] = info->m_nulldatas[info->m_nnodekinds+k];
                    }
                }
            }
            free(fvalue);
            fvalue = NULL;
            free(effe_num);
            effe_num = NULL;
        }
        size = info->m_nkinds*(step_loop-param->start_step);
        for(j=0;j<info->m_nkinds;j++){
            info->m_value_max[size+j] = info->m_values[j];
            info->m_value_min[size+j] = info->m_values[j];
            for(i=1;i<info->m_nnodes;i++){
                if(info->m_value_max[size+j]<info->m_values[i*info->m_nkinds+j]) info->m_value_max[size+j] = info->m_values[i*info->m_nkinds+j];
                if(info->m_value_min[size+j]>info->m_values[i*info->m_nkinds+j]) info->m_value_min[size+j] = info->m_values[i*info->m_nkinds+j];
            }
        }
    }

    if(elem_value!=NULL){
        free(elem_value);
        elem_value = NULL;
    }
    if (node_value != NULL) {
        free(node_value);
        node_value = NULL;
    }
    state = RTC_OK;
    return(state);
garbage:
    return(RTC_NG);
}

} /* namespace filter */
} /* namespace pbvr */
