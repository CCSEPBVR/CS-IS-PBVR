#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "../filter_log.h"
#include "../filter_convert.h"

static int ChangeChar( char Text[], char Chr1, char Chr2 ){
        int        i;
        int        len;
        len = (int)strlen(Text);
        for( i=0; i<len; i++ ){
                if( Text[i] == Chr1 ) Text[i] = Chr2;
        }
        return 0;
}

/*****************************************************************************/
/*
 * UCD コントロールファイル読込み
 *
 */
int read_inp_file( void )
{
    int          state = RTC_NG;
    FILE        *ifs = NULL;
    char        *ch;
    char         line [LINELEN_MAX];
//    char         alloc_flag;
    int          i;
    int          count;
//    int          do_step;
    FilterParam *param;

    count = 0;
//    do_step = 0;
//    alloc_flag = 0;

    param = &Fparam;
    sprintf(filename, "%s/%s",Fparam.in_dir,Fparam.ucd_inp );
    ifs = fopen(filename,"rb");
    if(ifs == NULL) {
        filter_log(FILTER_ERR,"Can't open file : %s\n",filename);
        return state;
    }

    param->nstep = 1;

    m_ucdbin = 0;
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
                m_cycle_type = 1;
                m_ucdbin = 1;
            }else if(strstr(line,"data")){
                m_cycle_type = 2;
                m_ucdbin = 1;
            }else if(strstr(line,"geom")){
                m_cycle_type = 3;
                m_ucdbin = 1;
            }else{
                param->nstep = atoi(line);
                m_ucdbin = 0;
            }
        }
        count++;
    }
    fseek(ifs,0,SEEK_SET);

    /* エラー */
    if(m_ucdbin){
        filter_log(FILTER_ERR,"Not support ucd-binary by avsinp-file : %s\n",filename);
        return state;
    }

    if(m_ucdbin) {
        m_ucdbinfile = (char**)malloc(sizeof(char*)*count); 
        for(i=0;i<count;i++){
            m_ucdbinfile[i] = (char*)malloc(sizeof(char)*LINELEN_MAX);
            memset(m_ucdbinfile[i],0x00,sizeof(char)*LINELEN_MAX);
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
                    m_cycle_type = 1;
                }else if(strstr(line,"data")){
                    m_cycle_type = 2;
                }else if(strstr(line,"geom")){
                    m_cycle_type = 3;
                }else{
                    param->nstep = atoi(line);
                    ;
                }
            }else{
                for(i=0;i<LINELEN_MAX;i++){
                    if(line[i]=='\n'){ 
                        break;
                    }
                    m_ucdbinfile[count-1][i]=line[i];
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
                m_ucdbin = 0;
            }else{
                if(strstr(line,"data_geom")){
                    m_cycle_type = 1;
                }else if(strstr(line,"data")){
                    m_cycle_type = 2;
                }else if(strstr(line,"geom")){
                    m_cycle_type = 3;
                }else{
                    //                    ;
                    /*
                    ここでサイクルタイプの記述がなかった場合旧式フォーマットと判定
                    旧フォーマットであった場合はステップ数を1とする(もし2以上のステップ数が来た場合は別途処理が必要になる。
                    またサイクルタイプはdataで固定される。
                    */
                    if(m_cycle_type == 0){
                        printf("Detected an old type of AVS format.\n");
                        param->nstep = 1;
                        m_cycle_type = 2;
                        m_old_avs_format = 1;
                    }
                }
            }
            count++;
            if(count>1){
                break;
            }
        }
        m_ucd_ascii_fp = ifs;
    }

    state = RTC_OK;
    return(state);
}

/*****************************************************************************/
/*
 * ASCII形式のUCD GEOM読込み
 *
 */
int skip_ucd_ascii( int step )
{
    FILE        *ifs = NULL;
    char         line [LINELEN_MAX];
    char         buf[10];
    char        *ch;
    char        *value=NULL;

    ifs = m_ucd_ascii_fp;
    if(ifs == NULL) {
        filter_log(FILTER_ERR,"Can't open file : %s\n",filename);
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
int read_ucd_ascii_geom( void )
{
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
    FilterParam *param;
    int          nd;
    int          em;

    count = 0;
    do_step = 0;
    alloc_flag = 0;

    param = &Fparam;
    ifs = m_ucd_ascii_fp;
    if(ifs == NULL) {
        filter_log(FILTER_ERR,"Can't open file : %s\n",filename);
        return state;
    }

        count = 0;
        if(m_old_avs_format == 1){
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
            m_nnodes = atoi(value);
            value = strtok(NULL," ");
            m_nelements = atoi(value);
            break;
        }
        m_ids         = (int *)malloc(sizeof(int)*m_nnodes);
        if(NULL==m_ids) {
            filter_log(FILTER_ERR, "can not allocate memory(m_ids).\n");
            goto garbage;
        }
        m_coords      = (float *)malloc(sizeof(float)*m_nnodes*3);
        if(NULL==m_coords) {
            filter_log(FILTER_ERR, "can not allocate memory(m_coords).\n");
            goto garbage;
        }
        m_elementids  = (int *)malloc(sizeof(int)*m_nelements);
        m_materials   = (int *)malloc(sizeof(int)*m_nelements);
        m_elemtypes   = (char *)malloc(sizeof(char)*m_nelements);
        m_connections = (int *)malloc(sizeof(int)*m_nelements*20);
        if(NULL==m_connections) {
            filter_log(FILTER_ERR, "can not allocate memory(m_connections).\n");
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
            m_ids[count] = atoi(value);
            value = strtok(NULL," ");
            m_coords[count*3] = atof(value);
            value = strtok(NULL," ");
            m_coords[count*3+1] = atof(value);
            value = strtok(NULL," ");
            m_coords[count*3+2] = atof(value);
            count++;
            if(count>=m_nnodes){
                break;
            }
        }

        Fparam.nodesearch = (char)0;
        for(nd=0;nd<m_nnodes;nd++){
            if(m_ids[nd] != (nd+1)){
                Fparam.nodesearch = (char)1;
                filter_log(FILTER_LOG,"     mids[%10d]        : %d\n",nd,m_ids[nd]       );
                filter_log(FILTER_LOG,"     Fparam.nodesearch : %d\n",Fparam.nodesearch);
                break;
            }
        }
        min.x = max.x = m_coords[0];
        min.y = max.y = m_coords[1];
        min.z = max.z = m_coords[2];
        for(nd=1; nd<m_nnodes; nd++){
            if (min.x > m_coords[nd*3  ]) min.x = m_coords[nd*3  ];
            if (min.y > m_coords[nd*3+1]) min.y = m_coords[nd*3+1];
            if (min.z > m_coords[nd*3+2]) min.z = m_coords[nd*3+2];

            if (max.x < m_coords[nd*3  ]) max.x = m_coords[nd*3  ];
            if (max.y < m_coords[nd*3+1]) max.y = m_coords[nd*3+1];
            if (max.z < m_coords[nd*3+2]) max.z = m_coords[nd*3+2];
        }
        filter_log(FILTER_LOG,"|||| max -> %8.2f, %8.2f, %8.2f\n",max.x,max.y,max.z);
        filter_log(FILTER_LOG,"|||| min -> %8.2f, %8.2f, %8.2f\n",min.x,min.y,min.z);

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
            m_elementids[count] = atoi(value);
            value = strtok(NULL," ");
            m_materials[count] = atoi(value);
            value = strtok(NULL," ");
            if(strstr(value,"hex2")){
                m_elemtypes[count] = 14;
                size = 20;
            }else if(strstr(value,"prism2")){
                m_elemtypes[count] = 13;
                size = 15;
            }else if(strstr(value,"pyr2")){
                m_elemtypes[count] = 12;
                size = 13;
            }else if(strstr(value,"tet2")){
                m_elemtypes[count] = 11;
                size = 10;
            }else if(strstr(value,"quad2")){
                m_elemtypes[count] = 10;
                size = 8;
            }else if(strstr(value,"tri2")){
                m_elemtypes[count] = 9;
                size = 6;
            }else if(strstr(value,"line2")){
                m_elemtypes[count] = 8;
                size = 3;
            }else if(strstr(value,"hex")){
                m_elemtypes[count] = 7;
                size = 8;
            }else if(strstr(value,"prism")){
                m_elemtypes[count] = 6;
                size = 6;
            }else if(strstr(value,"pyr")){
                m_elemtypes[count] = 5;
                size = 5;
            }else if(strstr(value,"tet")){
                m_elemtypes[count] = 4;
                size = 4;
            }else if(strstr(value,"quad")){
                m_elemtypes[count] = 3;
                size = 4;
            }else if(strstr(value,"tri")){
                m_elemtypes[count] = 2;
                size = 3;
            }else if(strstr(value,"line")){
                m_elemtypes[count] = 1;
                size = 2;
            }else{
                m_elemtypes[count] = 0;
                size = 1;
            }
            m_elemcoms = size;	
            m_element_type = m_elemtypes[count];
            conn_count = 0;
            while(conn_count<size) {
                value = strtok(NULL," ");
                m_connections[conn_size] = atof(value);
                conn_size++;
                conn_count++;
            }
            m_types[(int)m_elemtypes[count]]++;
            count++;
            if(count>=m_nelements){
                break;
            }
        }
        m_connectsize = conn_size;

        type = m_elemtypes[0];
        Fparam.mult_element_type = (char)0;
        for(em=1;em<m_nelements;em++){
            if(type!=m_elemtypes[em]){
                Fparam.mult_element_type = (char)1;
                break;
            }
        }

        filter_log(FILTER_LOG,"|||| m_nelements       : %10d\n",   m_nelements      );
        filter_log(FILTER_LOG,"|||| m_connectsize     : %10d\n",   m_connectsize    );
        filter_log(FILTER_LOG,"|||| Trai    (type 2)  : %10d\n",   m_types[ 2]      );
        filter_log(FILTER_LOG,"|||| Quad    (type 3)  : %10d\n",   m_types[ 3]      );
        filter_log(FILTER_LOG,"|||| Tetra   (type 4)  : %10d\n",   m_types[ 4]      );
        filter_log(FILTER_LOG,"|||| Pyramid (type 5)  : %10d\n",   m_types[ 5]      );
        filter_log(FILTER_LOG,"|||| Prism   (type 6)  : %10d\n",   m_types[ 6]      );
        filter_log(FILTER_LOG,"|||| Hexa    (type 7)  : %10d\n",   m_types[ 7]      );
        filter_log(FILTER_LOG,"|||| Trai2   (type 9)  : %10d\n",   m_types[ 9]      );
        filter_log(FILTER_LOG,"|||| Quad2   (type10)  : %10d\n",   m_types[10]      );
        filter_log(FILTER_LOG,"|||| Tetra2  (type11)  : %10d\n",   m_types[11]      );
        filter_log(FILTER_LOG,"|||| Pyramid2(type12)  : %10d\n",   m_types[12]      );
        filter_log(FILTER_LOG,"|||| Prism2  (type13)  : %10d\n",   m_types[13]      );
        filter_log(FILTER_LOG,"|||| Hexa2   (type14)  : %10d\n",   m_types[14]      );

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
int skip_ucd_ascii_geom( void )
{
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

    ifs = m_ucd_ascii_fp;
    if(NULL==ifs){
        goto garbage;
    }

        count = 0;
        //旧フォーマット分岐
        if(m_old_avs_format == 1){
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
int read_ucd_ascii_value( int step_loop )
{
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
    FilterParam *param;
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

    param = &Fparam;
    sprintf(filename, "%s/%s",Fparam.in_dir,Fparam.ucd_inp );
    ifs = m_ucd_ascii_fp;
    if(ifs == NULL) {
        filter_log(FILTER_ERR,"Can't open file : %s\n",filename);
        return state;
    }
    int file_line = 0;
        count = 0;

        if(m_old_avs_format == 1){
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

            if(m_old_avs_format == 1){
                value = strtok(NULL," ");
                value = strtok(NULL," ");
            }

            m_nnodekinds = atoi(value);
            value = strtok(NULL," ");
            m_nelemkinds = atoi(value);
            break;
        }

        if(m_old_avs_format == 1){
            fseek(ifs,file_line,SEEK_SET);
        }

        if(m_nnodekinds != 0){
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
                m_nnodecomponents = atoi(value);
                mn_veclen = (int*)malloc(sizeof(int)*m_nnodecomponents);
                for(i=0;i<m_nnodecomponents;i++){
                    value = strtok(NULL," ");
                    mn_veclen[i] = atoi(value);
                }
                break;
            }

            pmn_name = (char**)malloc(sizeof(char*)*m_nnodecomponents);
            for(i=0;i<m_nnodecomponents;i++){
                pmn_name[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
            }
            pmn_unit = (char**)malloc(sizeof(char*)*m_nnodecomponents);
            for(i=0;i<m_nnodecomponents;i++){
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
                if(count>=m_nnodecomponents){
                    break;
                }
            }

            m_ids = (int*)malloc(sizeof(int)*m_nnodes);
            node_value = (float*)malloc(sizeof(float)*m_nnodes*m_nnodekinds);
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
                m_ids[count] = atoi(value);
                for(i=0;i<m_nnodekinds;i++){
                    value = strtok(NULL," ");
                    node_value[count*m_nnodekinds+i] =  atof(value);
                }
                count++;
                if(count>=m_nnodes){
                    break;
                }
            }
        }
        if(m_nelemkinds != 0){
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
                m_nelemcomponents = atoi(value);
                me_veclen = (int*)malloc(sizeof(int)*m_nelemcomponents);
                for(i=0;i<m_nelemcomponents;i++){
                    value = strtok(NULL," ");
                    me_veclen[i] = atoi(value);
                }
                break;
            }

            pme_name = (char**)malloc(sizeof(char*)*m_nelemcomponents);
            for(i=0;i<m_nelemcomponents;i++){
                pme_name[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
            }
            pme_unit = (char**)malloc(sizeof(char*)*m_nelemcomponents);
            for(i=0;i<m_nelemcomponents;i++){
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
                if(count>=m_nelemcomponents){
                    break;
                }
            }

            m_elementids = (int*)malloc(sizeof(int)*m_nelements);
            elem_value = (float*)malloc(sizeof(float)*m_nelements*m_nelemkinds);
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
                m_elementids[count] = atoi(value);
                for(i=0;i<m_nelemkinds;i++){
                    value = strtok(NULL," ");
                    elem_value[count*m_nelemkinds+i] =  atof(value);
                }
                count++;
                if(count>=m_nelements){
                    break;
                }
            }
        }

    m_nkinds = m_nnodekinds + m_nelemkinds;
    m_ncomponents = m_nnodecomponents + m_nelemcomponents;

    if(m_ncomponents>0){
        pm_names = (char**)malloc(sizeof(char*)*m_ncomponents);
        for(i=0;i<m_ncomponents;i++){
            pm_names[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }
        pm_units = (char**)malloc(sizeof(char*)*m_ncomponents);
        for(i=0;i<m_ncomponents;i++){
            pm_units[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }

        m_veclens = (int *)malloc(sizeof(int)*m_ncomponents );
        m_nullflags = (int *)malloc(sizeof(int)*m_nkinds );
        m_nulldatas = (float *)malloc(sizeof(float)*m_nkinds );
        if(m_nnodecomponents>0){
            nkind = 0;
            for(i=0;i<m_nnodecomponents;i++ ){
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[i][m] = pmn_name[i][m];
                    pm_units[i][m] = pmn_unit[i][m];
                }
                m_veclens[i] = mn_veclen[i];
                for(j=0;j<m_veclens[i];j++){
                    m_nullflags[nkind] = 0;
                    m_nulldatas[nkind] = 0;
                    nkind++;
                }
            }
            if(pmn_name!=NULL){
                for(i=0;i<m_nnodecomponents;i++){
                    free(pmn_name[i]);
                }
                free(pmn_name);
                pmn_name=NULL;
            }
            if(pmn_unit!=NULL){
                for(i=0;i<m_nnodecomponents;i++){
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
        if(m_nelemcomponents>0){
            nkind = 0;
            for(i=0;i<m_nelemcomponents;i++ ){
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[m_nnodecomponents+i][m] = pme_name[i][m];
                    pm_units[m_nnodecomponents+i][m] = pme_unit[i][m];
                }
                m_veclens[m_nnodecomponents+i] = me_veclen[i];
                for(j=0;j<me_veclen[i];j++){
                    m_nullflags[m_nnodecomponents+nkind] = 0;
                    m_nulldatas[m_nnodecomponents+nkind] = 0;
                }
            }
            if(pme_name!=NULL){
                for(i=0;i<m_nelemcomponents;i++){
                    free(pme_name[i]);
                }
                free(pme_name);
                pme_name=NULL;
            }
            if(pme_unit!=NULL){
                for(i=0;i<m_nelemcomponents;i++){
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
        if(m_nnodekinds>0){
            for(i=0;i<m_nnodes;i++){
                for(j=0;j<m_nkinds;j++){
                    m_values[i*m_nkinds+j] = node_value[i*m_nnodekinds+j];
                }
            }
        }
        if(m_nelemkinds>0){
            fvalue = (float *)malloc(sizeof(float) * m_nelemkinds);
            effe_num = (int *)malloc(sizeof(int) * m_nelemkinds);
            if(NULL==fvalue||NULL==effe_num){
                filter_log(FILTER_ERR, "can not allocate memory(fvalues).\n");
                goto garbage;
            }
            for(i=0;i<m_nnodes;i++){
                for(k=0;k<m_nelemkinds;k++){
                    fvalue[k] = 0.0;
                    effe_num[k] = 0;
                }
                for(j=0;j<m_elem_in_node[i];j++){
                    eid = m_elem_in_node_array[m_elem_in_node_addr[i]+j] - 1;
                    for(k=0;k<m_nelemkinds;k++){
                        fvalue[k] = fvalue[k] + elem_value[eid*m_nelemkinds+k];
                        effe_num[k]++ ;
                    }
                }
                for(k=0;k<m_nelemkinds;k++){
                    if(effe_num[k]!=0){
                        m_values[i*m_nkinds+m_nnodekinds+k] = fvalue[k] / effe_num[k];
                    }else{
                        m_values[i*m_nkinds+m_nnodekinds+k] = m_nulldatas[m_nnodekinds+k];
                    }
                }
            }
            free(fvalue);
            fvalue = NULL;
            free(effe_num);
            effe_num = NULL;
        }
        size = m_nkinds*(step_loop-Fparam.start_step);
        for(j=0;j<m_nkinds;j++){
            m_value_max[size+j] = m_values[j];
            m_value_min[size+j] = m_values[j];
            for(i=1;i<m_nnodes;i++){
                if(m_value_max[size+j]<m_values[i*m_nkinds+j]) m_value_max[size+j] = m_values[i*m_nkinds+j];
                if(m_value_min[size+j]>m_values[i*m_nkinds+j]) m_value_min[size+j] = m_values[i*m_nkinds+j];
            }
        }
    }

    if(elem_value!=NULL){
        free(elem_value);
        elem_value = NULL;
    }

    state = RTC_OK;
    return(state);
garbage:
    return(RTC_NG);
}

