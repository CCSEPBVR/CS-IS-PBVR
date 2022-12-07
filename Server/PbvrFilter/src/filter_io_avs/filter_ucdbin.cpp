#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <sys/stat.h>
#include "filter_io_avs/FilterIoAvs.h"

#ifdef _OPENMP
#include "omp.h"
#endif


namespace pbvr {
namespace filter {

/*****************************************************************************/
/*
 * ＵＣＤバイナリファイル全読込み
 *
 */
int FilterIoAvs::read_ucdbin_geom( void )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char* buff = info->buff;
    char* filename = info->filename;
    float *m_coords = NULL;
    NodeInf *min = &info->node_min;
    NodeInf *max = &info->node_max;

    int       state = RTC_NG;
    FILE     *ifs = NULL;
    Int64  i;
    Int64  i3;
    int       m,j;
    int       header;
    int       footer;
    int       veclen;
    char      keyword[7];
    float     version;
    int       stepn;
    float     stept = 0.0f;
    int       nnodes;
    int       desc_type;
    int       id;
    float     coord[3];
    int       nelements;
    char      element_type;
    int       nelem_com;
    int       ncomponents;
    int       data_type;
    char      component_name[COMPONENT_LEN];
    char      unit[COMPONENT_LEN];
    int       null_flag = 0;
    float     null_data = 0.0f;
    int       nkind;
    char      headfoot = 1;
    int       type;
    int       size;
    int       bsize;
    int       div;
    int       mod;
    char  *mn_name=NULL;
    char  *mn_unit=NULL;
    char  **pmn_name=NULL;
    char  **pmn_unit=NULL;
    int   *mn_veclen=NULL;
    int   *mn_nullflag=NULL;
    float *mn_nulldata=NULL;
    char  *me_name=NULL;
    char  *me_unit=NULL;
    char  **pme_name=NULL;
    char  **pme_unit=NULL;
    int   *me_veclen=NULL;
    int   *me_nullflag=NULL;
    float *me_nulldata=NULL;

    bsize = param->blocksize;
    if(strlen(param->ucd_inp)!=0&&info->m_multi_element==0){
        sprintf(filename,"%s",info->m_ucdbinfile[0]);
    }else{
        sprintf(buff,param->format,param->start_step);
        sprintf(filename,"%s/%s%s%s",param->in_dir,param->in_prefix,buff,param->in_suffix);
    }
    /* オープン */
    ifs = fopen(filename, "rb");
    if(NULL==ifs){
        LOGOUT(FILTER_ERR, (char*)"can not open %s .\n", filename);
        goto garbage;
    }
    // check first byte size.
    fread(&header, 4, 1, ifs);
    if(info->ENDIAN) header = ConvertEndianI(header);
    LOGOUT(FILTER_LOG, (char*)"|||| headder           : %10d\n",  header           );
    if(header==7){
        headfoot = (char) 1;
    }else{
        headfoot = (char) 0;
    }
    info->m_headfoot = headfoot;
    rewind(ifs);
    header = footer = 0;
    LOGOUT(FILTER_LOG, (char*)"|||| filename          : %s\n",    filename         );
    LOGOUT(FILTER_LOG, (char*)"|||| m_headfoot        : %10d\n",  info->m_headfoot       );

    // keyword
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( keyword, 7, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );

    // version
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &version, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) version = ConvertEndianF(version);

    // title
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( info->m_title, 70, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );

    // step num.
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &stepn, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) stepn = ConvertEndianI(stepn);
    info->m_stepn = stepn;

    // step time
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &stept, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) stept = ConvertEndianF(stept);
    info->m_stept = stept;
    LOGOUT(FILTER_LOG, (char*)"|||| keyword           : %s\n",    keyword          );
    LOGOUT(FILTER_LOG, (char*)"|||| version           : %10.3f\n",version          );
    /*LOGOUT(FILTER_LOG, (char*)"|||| m_title           : %s\n",    m_title           ); */
    LOGOUT(FILTER_LOG, (char*)"|||| step_no           : %10d\n",  stepn            );
    LOGOUT(FILTER_LOG, (char*)"|||| step_time         : %10.3f\n",stept            );

    //Node information

    // num. nodes
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &nnodes, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) nnodes = ConvertEndianI(nnodes);
    info->m_nnodes = nnodes;

    // description type should be 1.
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &desc_type, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) desc_type = ConvertEndianI(desc_type);
    info->m_desctype = desc_type;
    LOGOUT(FILTER_LOG, (char*)"|||| m_nnodes          : %10d\n",   info->m_nnodes         );
    LOGOUT(FILTER_LOG, (char*)"|||| m_desctype        : %10d\n",   info->m_desctype       );

    // coordinates
    if (info->m_coords != NULL) free(info->m_coords);
    info->m_coords = (float *)malloc(sizeof(float)*(info->m_nnodes)*3) ;
    if(NULL==info->m_coords){
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(coords).\n");
        goto garbage;
    }
    m_coords = info->m_coords;
    if (info->m_ids != NULL) free(info->m_ids);
    info->m_ids = (int *)malloc(sizeof(int)*(info->m_nnodes)) ;
    if(NULL==info->m_ids){
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(ids).\n");
        goto garbage;
    }

    if(info->m_desctype==1){
        i3 = 0;
        for(i=0;i<info->m_nnodes;i++,i3+= 3){
            if(headfoot) fread( &header, 4, 1, ifs );
            fread( &id, 4, 1, ifs );
            fread( coord, 4, 3, ifs );
            if(headfoot) fread( &footer, 4, 1, ifs );
            if(info->ENDIAN){
                id       = ConvertEndianI(id);
                coord[0] = ConvertEndianF(coord[0]);
                coord[1] = ConvertEndianF(coord[1]);
                coord[2] = ConvertEndianF(coord[2]);
            }
            info->m_ids[i] = id;
            m_coords[ i3     ] = coord[0];
            m_coords[ i3 + 1 ] = coord[1];
            m_coords[ i3 + 2 ] = coord[2];
        }
    }else{
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( info->m_ids, 4, info->m_nnodes, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        /* X corrdinate */
        if(headfoot) fread( &header, 4, 1, ifs );
        i3 = 0;
        for(i=0;i<info->m_nnodes;i++,i3+= 3){
            fread( &m_coords[i3], 4, 1, ifs );
        }
        if(headfoot) fread( &footer, 4, 1, ifs );
        /* Y corrdinate */
        if(headfoot) fread( &header, 4, 1, ifs );
        i3 = 1;
        for(i=0;i<info->m_nnodes;i++,i3+= 3){
            fread( &m_coords[i3], 4, 1, ifs );
        }
        if(headfoot) fread( &footer, 4, 1, ifs );
        /* Z corrdinate */
        if(headfoot) fread( &header, 4, 1, ifs );
        i3 = 2;
        for(i=0;i<info->m_nnodes;i++,i3+= 3){
            fread( &m_coords[i3], 4, 1, ifs );
        }
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(info->ENDIAN){
            for(i=0;i<info->m_nnodes;i++){
                info->m_ids[i] = ConvertEndianF(info->m_ids[i]);
                m_coords[i*3  ] = ConvertEndianF(m_coords[i*3  ]);
                m_coords[i*3+1] = ConvertEndianF(m_coords[i*3+1]);
                m_coords[i*3+2] = ConvertEndianF(m_coords[i*3+2]);
            }
        }
    }

    for(i=0;i<info->m_nnodes;i++){
        if(info->m_ids[i] != (i+1)){
            param->nodesearch = (char)1;
            LOGOUT(FILTER_LOG, (char*)"     mids[%10d]        : %d\n",i,info->m_ids[i]       );
            LOGOUT(FILTER_LOG, (char*)"     param->nodesearch : %d\n",param->nodesearch);
            break;
        }
    }

    /* 最大・最小を取得 */
#if defined(_OPENMP) && !defined(_WIN32) && !defined(_WIN64)
    int    myth,numth;
    float *min_x, *min_y, *min_z;
    float *max_x, *max_y, *max_z;
#pragma omp parallel private(myth,i)
  {
    myth = omp_get_thread_num();
    numth = omp_get_num_threads();
#pragma omp master
   {
    min_x = (float *)malloc(sizeof(float)*numth);
    min_y = (float *)malloc(sizeof(float)*numth);
    min_z = (float *)malloc(sizeof(float)*numth);
    max_x = (float *)malloc(sizeof(float)*numth);
    max_y = (float *)malloc(sizeof(float)*numth);
    max_z = (float *)malloc(sizeof(float)*numth);
   }
#pragma omp barrier
    min_x[myth] = FLT_MAX;
    min_y[myth] = FLT_MAX;
    min_z[myth] = FLT_MAX;
    max_x[myth] = -FLT_MAX;
    max_y[myth] = -FLT_MAX;
    max_z[myth] = -FLT_MAX;
#pragma omp for
    for(i=0;i<info->m_nnodes;i++){
        if (min_x[myth] > m_coords[i*3  ]) min_x[myth] = m_coords[i*3  ];
        if (min_y[myth] > m_coords[i*3+1]) min_y[myth] = m_coords[i*3+1];
        if (min_z[myth] > m_coords[i*3+2]) min_z[myth] = m_coords[i*3+2];

        if (max_x[myth] < m_coords[i*3  ]) max_x[myth] = m_coords[i*3  ];
        if (max_y[myth] < m_coords[i*3+1]) max_y[myth] = m_coords[i*3+1];
        if (max_z[myth] < m_coords[i*3+2]) max_z[myth] = m_coords[i*3+2];
    }
#pragma omp master
   {
    min->x = min_x[0];
    min->y = min_y[0];
    min->z = min_z[0];
    max->x = max_x[0];
    max->y = max_y[0];
    max->z = max_z[0];
    for(i=1;i<numth;i++){
        if (min->x > min_x[i]) min->x = min_x[i];
        if (min->y > min_y[i]) min->y = min_y[i];
        if (min->z > min_z[i]) min->z = min_z[i];

        if (max->x < max_x[i]) max->x = max_x[i];
        if (max->y < max_y[i]) max->y = max_y[i];
        if (max->z < max_z[i]) max->z = max_z[i];
    }
    free(max_x);
    free(max_y);
    free(max_z);
    free(min_x);
    free(min_y);
    free(min_z);
   }
  }
#else
    min->x = max->x = m_coords[0];
    min->y = max->y = m_coords[1];
    min->z = max->z = m_coords[2];
    for(i=1; i<info->m_nnodes; i++){
        if (min->x > m_coords[i*3  ]) min->x = m_coords[i*3  ];
        if (min->y > m_coords[i*3+1]) min->y = m_coords[i*3+1];
        if (min->z > m_coords[i*3+2]) min->z = m_coords[i*3+2];

        if (max->x < m_coords[i*3  ]) max->x = m_coords[i*3  ];
        if (max->y < m_coords[i*3+1]) max->y = m_coords[i*3+1];
        if (max->z < m_coords[i*3+2]) max->z = m_coords[i*3+2];
    }
#endif
    LOGOUT(FILTER_LOG, (char*)"|||| max -> %8.2f, %8.2f, %8.2f\n",max->x,max->y,max->z);
    LOGOUT(FILTER_LOG, (char*)"|||| min -> %8.2f, %8.2f, %8.2f\n",min->x,min->y,min->z);

    //Cell information

    // num. element
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &nelements, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) nelements = ConvertEndianI(nelements);
    info->m_nelements = nelements;
    LOGOUT(FILTER_LOG, (char*)"|||| m_nelements       : %10d\n",   info->m_nelements      );

    // element num.
    if(headfoot) fread( &header, 4, 1, ifs );
    if (info->m_elementids != NULL) free(info->m_elementids);
    info->m_elementids = (int *)malloc(sizeof(int) * nelements );
#ifdef BLOCK_READ
    div = nelements / bsize;
    mod = nelements % bsize;
    for(i=0;i<div;i++){
        fread( &info->m_elementids[bsize*i], 4, bsize , ifs );
    }
    if(mod>0){
        fread( &info->m_elementids[bsize*div], 4, mod , ifs );
    }
#else
    fread( info->m_elementids, 4, nelements , ifs );
#endif
    if(info->ENDIAN){
        for(i=0;i<nelements;i++){
            info->m_elementids[i] = ConvertEndianI(info->m_elementids[i]);
        }
    }
    LOGOUT(FILTER_LOG, (char*)"|||| element id read         \n"                     );
    if(headfoot) fread( &footer, 4, 1, ifs );

    // material num.
    if(headfoot) fread( &header, 4, 1, ifs );
    if (info->m_materials == NULL) free(info->m_materials);
    info->m_materials = (int *)malloc(sizeof(int) * nelements );
#ifdef BLOCK_READ
    div = nelements / bsize;
    mod = nelements % bsize;
    for(i=0;i<div;i++){
        fread( &info->m_materials[bsize*i], 4, bsize , ifs );
    }
    if(mod>0){
        fread( &info->m_materials[bsize*div], 4, mod , ifs );
    }
#else
    fread( info->m_materials, 4, nelements , ifs );
#endif
    if(info->ENDIAN){
        for(i=0;i<nelements;i++){
            info->m_materials[i] = ConvertEndianI(info->m_materials[i]);
        }
    }
    LOGOUT(FILTER_LOG, (char*)"|||| element material read   \n"                     );
    if(headfoot) fread( &footer, 4, 1, ifs );

    // element type
    if (info->m_elemtypes != NULL) free(info->m_elemtypes);
    info->m_elemtypes = (char *)malloc(sizeof(char) * nelements );
    if(NULL==info->m_elemtypes){
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(elemtypes).\n");
        goto garbage;
    }
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( info->m_elemtypes, 1, nelements, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    element_type = info->m_elemtypes[0];

    if( info->m_elemtypes[0] == 2 ){
        nelem_com = 3;
    }else if( info->m_elemtypes[0] == 3 ){
        nelem_com = 4;
    }else if( info->m_elemtypes[0] == 4 ){
        nelem_com = 4;
    }else if( info->m_elemtypes[0] == 5 ){
        nelem_com = 5;
    }else if( info->m_elemtypes[0] == 6 ){
        nelem_com = 6;
    }else if( info->m_elemtypes[0] == 7 ){
        nelem_com = 8;
    }else if( info->m_elemtypes[0] == 9 ){
        nelem_com = 6;
    }else if( info->m_elemtypes[0] == 10){
        nelem_com = 8;
    }else if( info->m_elemtypes[0] == 11){
        nelem_com = 10;
    }else if( info->m_elemtypes[0] == 14){
        nelem_com = 20;
    }else{
        LOGOUT(FILTER_ERR, (char*)"Unkown element type : %d\n",info->m_element_type);
        goto garbage;
    }
    info->m_elemcoms = nelem_com;
    info->m_element_type = info->m_elemtypes[0];

    LOGOUT(FILTER_LOG, (char*)"|||| element_type      : %10d\n",   element_type     );
    LOGOUT(FILTER_LOG, (char*)"|||| m_element_type    : %10d\n",   info->m_element_type   );
    LOGOUT(FILTER_LOG, (char*)"|||| m_elemcoms        : %10d\n",   info->m_elemcoms       );

    param->mult_element_type = (char)0;
    info->m_connectsize = 0;
    type = info->m_elemtypes[0];
    for(i=0;i<info->m_nelements;i++){
        if(type!=info->m_elemtypes[i]){
            param->mult_element_type = (char)1;
        }
        if(info->m_elemtypes[i]<=1  ||
           info->m_elemtypes[i]==8  ||
           info->m_elemtypes[i]==12 ||
           info->m_elemtypes[i]==13 ||
           info->m_elemtypes[i]>=15){
            LOGOUT(FILTER_ERR, (char*)"Unkown element type : %d\n",info->m_elemtypes[i]);
            goto garbage;
        }
        info->m_connectsize+=info->m_elementsize[(int)info->m_elemtypes[i]];
        info->m_types[(int)info->m_elemtypes[i]]++;
    }

    LOGOUT(FILTER_LOG, (char*)"|||| m_nelements       : %10d\n",   info->m_nelements      );
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

//  m_connections = (int *)malloc(sizeof(int)*nelem_com*m_nelements );
    if (info->m_connections != NULL) free(info->m_connections);
    info->m_connections = (int *)malloc(sizeof(int)*info->m_connectsize );
    if(NULL==info->m_connections){
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(connections).\n");
        goto garbage;
    }

    if(headfoot) fread( &header, 4, 1, ifs );
//  fread( m_connections, 4, nelem_com * m_nelements , ifs );
#ifdef BLOCK_READ
    div = info->m_connectsize / bsize;
    mod = info->m_connectsize % bsize;
    for(i = 0; i < div; i++ ) {
        fread( &info->m_connections[bsize*i], 4, bsize , ifs );
    }
    if(mod>0){
        fread( &info->m_connections[bsize*div], 4, mod , ifs );
    }
#else
    fread( info->m_connections, 4, info->m_connectsize , ifs );
#endif
    if(headfoot) fread( &footer, 4, 1, ifs );

    /// adjust connection by minus 1 ///
    if(info->ENDIAN){
#ifdef _OPENMP
        #pragma omp parallel for default(shared) private(i)
#endif
        for(i = 0; i < info->m_connectsize; i++ ) {
            info->m_connections[i] = ConvertEndianI(info->m_connections[i]);
        }
    }
    if (param->nodesearch ==1) {
        int max_id = 0;
        int *conv_ids = NULL;

        for(i=0;i<info->m_nnodes;i++){
            if(max_id<info->m_ids[i]){
                max_id = info->m_ids[i];
            }
        }

        conv_ids = (int *)malloc(sizeof(float)*(max_id+1));

        for(i=0;i<info->m_nnodes;i++){
            conv_ids[info->m_ids[i]] = i+1;
        }

        for(i = 0; i < info->m_connectsize; i++ ) {
            info->m_connections[i] = conv_ids[info->m_connections[i]];
        }

        free(conv_ids);

        param->nodesearch = 0;
    }

    //Data information

    // num. component of nodes
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &ncomponents, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    info->m_nnodecomponents = ncomponents;
    LOGOUT(FILTER_LOG, (char*)"|||| m_nnodecomponents : %10d\n",   info->m_nnodecomponents);

    nkind = 0;
    if(info->m_nnodecomponents>0){
        // data type
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &data_type, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(info->ENDIAN) data_type = ConvertEndianI(data_type);
        info->m_datatype = data_type;
        LOGOUT(FILTER_LOG, (char*)"||||  m_datatype        : %10d\n",   info->m_datatype      );
        mn_name = (char *)malloc(sizeof(char)*info->m_nnodecomponents*COMPONENT_LEN );
        if(NULL==mn_name){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mn_name).\n");
            goto garbage;
        }
        pmn_name = (char**)malloc(sizeof(char*)*info->m_nnodecomponents);
        for(i=0;i<info->m_nnodecomponents;i++){
            pmn_name[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }

        mn_unit = (char *)malloc(sizeof(char)*info->m_nnodecomponents*COMPONENT_LEN );
        if(NULL==mn_unit){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mn_unit).\n");
            goto garbage;
        }
        pmn_unit = (char**)malloc(sizeof(char*)*info->m_nnodecomponents);
        for(i=0;i<info->m_nnodecomponents;i++){
            pmn_unit[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }

        mn_veclen = (int *)malloc(sizeof(int)*info->m_nnodecomponents );
        if(NULL==mn_veclen){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mn_veclen).\n");
            goto garbage;
        }
        mn_nullflag = (int *)malloc(sizeof(int)*info->m_nnodecomponents );
        if(NULL==mn_nullflag){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mn_nullflag).\n");
            goto garbage;
        }
        mn_nulldata = (float *)malloc(sizeof(float)*info->m_nnodecomponents );
        if(NULL==mn_nulldata){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mn_nulldata).\n");
            goto garbage;
        }

        if(info->m_datatype==1){
            // data header
            for(i=0;i<info->m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, 1, COMPONENT_LEN, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                    null_flag = ConvertEndianI(null_flag);
                    null_data = ConvertEndianF(null_data);
                }
                mn_veclen[i] = veclen;
                mn_nullflag[i] = null_flag;
                mn_nulldata[i] = null_data;
//              strcpy(pmn_name[i],component_name);
//              strcpy(pmn_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pmn_name[i][m] = component_name[m];
                    pmn_unit[i][m] = unit[m]          ;
                }
                LOGOUT(FILTER_LOG, (char*)"||||  i  : %2d\n",i);
                LOGOUT(FILTER_LOG, (char*)"||||  component_name    : %s\n",     component_name  );
                LOGOUT(FILTER_LOG, (char*)"||||  unit              : %s\n",     unit            );
                LOGOUT(FILTER_LOG, (char*)"||||  veclen            : %10d\n",   veclen          );
                LOGOUT(FILTER_LOG, (char*)"||||  null_flag         : %10d\n",   null_flag       );
                LOGOUT(FILTER_LOG, (char*)"||||  null_data         : %10.5f\n", null_data       );
                nkind += veclen;
            }
            size = 4*nkind*info->m_nnodes;
            if(headfoot) size += (4+4)*info->m_nnodes;
            fseek( ifs, size, SEEK_CUR );
        }else if(info->m_datatype==2){
            // data header
            for(i=0;i<info->m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                    null_flag = ConvertEndianI(null_flag);
                    null_data = ConvertEndianF(null_data);
                }
                mn_veclen[i] = veclen;
                mn_nullflag[i] = null_flag;
                mn_nulldata[i] = null_data;
//              strcpy(pmn_name[i],component_name);
//              strcpy(pmn_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pmn_name[i][m] = component_name[m];
                    pmn_unit[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            size = sizeof(float)*info->m_nnodes*nkind;
            if(headfoot) size += (4+4)*nkind;
            fseek( ifs, size, SEEK_CUR );
        } else if(info->m_datatype==3){
            for(i=0;i<info->m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                mn_veclen[i] = veclen;
                mn_nullflag[i] = 1;
                mn_nulldata[i] = -99999999.;
//              strcpy(pmn_name[i],component_name);
//              strcpy(pmn_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pmn_name[i][m] = component_name[m];
                    pmn_unit[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            for(i=0;i<info->m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nnodes, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    nnodes = ConvertEndianI(nnodes);
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nnodes*(info->m_veclens[i]+1), SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
            }
        } else if(info->m_datatype==4){
            for(i=0;i<info->m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                mn_veclen[i] = veclen;
                mn_nullflag[i] = 1;
                mn_nulldata[i] = -99999999.;
//              strcpy(pmn_name[i],component_name);
//              strcpy(pmn_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pmn_name[i][m] = component_name[m];
                    pmn_unit[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            for(i=0;i<info->m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nnodes, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    nnodes = ConvertEndianI(nnodes);
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nnodes, SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nnodes*info->m_veclens[i], SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
            }
        }
    }
    info->m_nnodekinds = nkind;
    LOGOUT(FILTER_LOG, (char*)"|||| m_nnodekinds      : %10d\n",   info->m_nnodekinds     );

    // num. component of elements
    if(headfoot) fread( &header, 4, 1, ifs );
    size = fread( &ncomponents, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    info->m_nelemcomponents = ncomponents;

    LOGOUT(FILTER_LOG, (char*)"|||| m_nelemcomponents : %10d\n",   info->m_nelemcomponents);
    nkind = 0;

    if(info->m_nelemcomponents>0){
        // data type
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &data_type, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(info->ENDIAN) data_type = ConvertEndianI(data_type);
        info->m_datatype = data_type;
        LOGOUT(FILTER_LOG, (char*)"||||  m_datatype        : %10d\n",   info->m_datatype       );

        me_name = (char *)malloc(sizeof(char)*info->m_nelemcomponents*COMPONENT_LEN );
        if(NULL==me_name){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(me_name).\n");
            goto garbage;
        }
        pme_name = (char**)malloc(sizeof(char*)*info->m_nelemcomponents);
        for(i=0;i<info->m_nelemcomponents;i++){
            pme_name[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }

        me_unit = (char *)malloc(sizeof(char)*info->m_nelemcomponents*COMPONENT_LEN );
        if(NULL==me_unit){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(me_unit).\n");
            goto garbage;
        }
        pme_unit = (char**)malloc(sizeof(char*)*info->m_nelemcomponents);
        for(i=0;i<info->m_nelemcomponents;i++){
            pme_unit[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }

        me_veclen = (int *)malloc(sizeof(int)*info->m_nelemcomponents );
        if(NULL==me_veclen){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(me_veclen).\n");
            goto garbage;
        }
        me_nullflag = (int *)malloc(sizeof(int)*info->m_nelemcomponents );
        if(NULL==me_nullflag){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(me_nullflag).\n");
            goto garbage;
        }
        me_nulldata = (float *)malloc(sizeof(float)*info->m_nelemcomponents );
        if(NULL==me_nulldata){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(me_nulldata).\n");
            goto garbage;
        }

        if(info->m_datatype==1){
            // data header
            for(i=0;i<info->m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                    null_flag = ConvertEndianI(null_flag);
                    null_data = ConvertEndianF(null_data);
                }
                me_veclen[i] = veclen;
                me_nullflag[i] = null_flag;
                me_nulldata[i] = null_data;
//              strcpy(pme_name[i],component_name);
 //             strcpy(pme_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pme_name[i][m] = component_name[m];
                    pme_unit[i][m] = unit[m]          ;
                }
                unit[COMPONENT_LEN-1]='\0';
                component_name[COMPONENT_LEN-1]='\0';
                LOGOUT(FILTER_LOG, (char*)"||||  i  : %2d\n",i);
                LOGOUT(FILTER_LOG, (char*)"||||  component_name    : %s\n",     component_name  );
                LOGOUT(FILTER_LOG, (char*)"||||  unit              : %s\n",     unit            );
                LOGOUT(FILTER_LOG, (char*)"||||  veclen            : %10d\n",   veclen          );
                LOGOUT(FILTER_LOG, (char*)"||||  null_flag         : %10d\n",   null_flag       );
                LOGOUT(FILTER_LOG, (char*)"||||  null_data         : %10.5f\n", null_data       );
                nkind += veclen;
            }
            size = sizeof(float)*info->m_nelements*nkind;
            if(headfoot) size += (4+4)*info->m_nelements;
            fseek( ifs, size, SEEK_CUR );
        }else if(info->m_datatype==2){
            // data header
            for(i=0;i<info->m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                    null_flag = ConvertEndianI(null_flag);
                    null_data = ConvertEndianF(null_data);
                }
                me_veclen[i] = veclen;
                me_nullflag[i] = null_flag;
                me_nulldata[i] = null_data;
//              strcpy(pme_name[i],component_name);
//              strcpy(pme_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pme_name[i][m] = component_name[m];
                    pme_unit[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            size = sizeof(float)*info->m_nelements*nkind;
            if(headfoot) size += (4+4)*nkind;
            fseek( ifs, size, SEEK_CUR );
        } else if(info->m_datatype==3){
            for(i=0;i<info->m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                me_veclen[i] = veclen;
                me_nullflag[i] = 1;
                me_nulldata[i] = -99999999.;
//              strcpy(pme_name[i],component_name);
//              strcpy(pme_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pme_name[i][m] = component_name[m];
                    pme_unit[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            for(i=0;i<info->m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nelements, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    nelements = ConvertEndianI(nelements);
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nelements*(info->m_veclens[i]+1), SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
            }
        } else if(info->m_datatype==4){
            for(i=0;i<info->m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                me_veclen[i] = veclen;
                me_nullflag[i] = 1;
                me_nulldata[i] = -99999999.;
//              strcpy(pme_name[i],component_name);
//              strcpy(pme_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pme_name[i][m] = component_name[m];
                    pme_unit[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            for(i=0;i<info->m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nelements, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    nelements = ConvertEndianI(nelements);
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nelements, SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nelements*info->m_veclens[i], SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
            }
        }
    }
    info->m_nelemkinds = nkind;
    info->m_nkinds = info->m_nnodekinds + info->m_nelemkinds;
    info->m_ncomponents = info->m_nnodecomponents + info->m_nelemcomponents;
    LOGOUT(FILTER_LOG, (char*)"|||| m_nelemkinds      : %10d\n",   info->m_nelemkinds     );
    LOGOUT(FILTER_LOG, (char*)"|||| m_ncomponents     : %10d\n",   info->m_ncomponents    );
    LOGOUT(FILTER_LOG, (char*)"|||| m_nkinds          : %10d\n",   info->m_nkinds         );

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
//              strcpy(pm_names[i],pmn_name[i]);
//              strcpy(pm_units[i],pmn_unit[i]);
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[i][m] = pmn_name[i][m];
                    info->pm_units[i][m] = pmn_unit[i][m];
                }
                info->m_veclens[i] = mn_veclen[i];
                for(j=0;j<info->m_veclens[i];j++){
                    info->m_nullflags[nkind] = mn_nullflag[i];
                    info->m_nulldatas[nkind] = mn_nulldata[i];
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
            if(mn_nullflag!=NULL) free(mn_nullflag);
            if(mn_nulldata!=NULL) free(mn_nulldata);
               mn_name=NULL;
               mn_unit=NULL;
               mn_veclen=NULL;
               mn_nullflag=NULL;
               mn_nulldata=NULL;
        }
        if(info->m_nelemcomponents>0){
            nkind = 0;
            for(i=0;i<info->m_nelemcomponents;i++ ){
//              strcpy(pm_names[info->m_nnodecomponents+i],pme_name[i]);
//              strcpy(pm_units[info->m_nnodecomponents+i],pme_unit[i]);
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[info->m_nnodecomponents+i][m] = pme_name[i][m];
                    info->pm_units[info->m_nnodecomponents+i][m] = pme_unit[i][m];
                }
                info->m_veclens[info->m_nnodecomponents+i] = me_veclen[i];
                for(j=0;j<me_veclen[i];j++){
                    info->m_nullflags[info->m_nnodecomponents+nkind] = me_nullflag[i];
                    info->m_nulldatas[info->m_nnodecomponents+nkind] = me_nulldata[i];
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
            if(me_nullflag!=NULL) free(me_nullflag);
            if(me_nulldata!=NULL) free(me_nulldata);
               me_name=NULL;
               me_unit=NULL;
               me_veclen=NULL;
               me_nullflag=NULL;
               me_nulldata=NULL;
        }
    }
    fseek(ifs,0,SEEK_END);
    info->m_filesize = ftell(ifs);
  /*fpos_t  pos;
    fgetpos(ifs,&pos);
    m_filesize = pos.__pos;*/
    LOGOUT(FILTER_LOG, (char*)"|||| m_filesize        : %10ld\n",   info->m_filesize       );

    state = RTC_OK;
garbage:
    if (NULL != ifs) fclose(ifs);
    return(state);
}

/*****************************************************************************/
/*
 * ＵＣＤバイナリファイル全読込み
 *
 */
int FilterIoAvs::read_ucdbin_data( int step_loop )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *buff = info->buff;
    char* filename = info->filename;
    float *m_coords = info->m_coords;
    NodeInf *min = &info->node_min;
    NodeInf *max = &info->node_max;

    int       state = RTC_NG;
    FILE     *ifs = NULL;
    Int64  i;
    Int64  j;
    Int64  k;
    int       m;
    Int64  i3;
    Int64  size;
    int       header;
    int       footer;
    char      keyword[7];
    float     version;
    int       stepn;
    float     stept = 0.0f;
    int       desc_type;
    int       id;
    float     coord[3];
    int       ncomponents;
    int       datatype;
    char      component_name[COMPONENT_LEN];
    char      unit[COMPONENT_LEN];
    int       null_flag;
    float     null_data;
    int       nkind;
    int       nnodes;
    int       nelements;
    int       veclen;
    float    *value = NULL;
    int      *ids;
    int       eid;
    char      headfoot = 0;
    Int64  f_size;
    Int64  geom_size;
    float dummy_coords;
    char *elemtypes = NULL;
    Int64  connectsize = 0;
    int nnodecomponents = 0;

    if(strlen(param->ucd_inp)!=0&&info->m_multi_element==0){
        sprintf(filename,"%s",info->m_ucdbinfile[step_loop-1]);
    }else{
        sprintf(buff,param->format,step_loop);
        sprintf(filename,"%s/%s%s%s",param->in_dir,param->in_prefix,buff,param->in_suffix);
    }
    /* オープン */
    ifs = fopen(filename, "rb");
    if(NULL==ifs){
        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
        goto garbage;
    }
    //Head information

    // check first byte size.
    fread(&header, 4, 1, ifs);
    if(info->ENDIAN) header = ConvertEndianI(header);
    LOGOUT(FILTER_LOG, (char*)"|||+ header            : %10d\n",  header           );
    if(header==7){
        headfoot = (char) 1;
    }else{
        headfoot = (char) 0;
    }

    geom_size = 0;
    fseek(ifs,0,SEEK_END);
    f_size = ftell(ifs);
  /*fpos_t  pos;
    fgetpos(ifs,&pos);
    f_size = pos.__pos;*/

    geom_size = 0;
    if(f_size!=info->m_filesize){
        // num. nodes
        geom_size += 4;
        if(headfoot) geom_size+=8;
        // description type
        geom_size += 4;
        if(headfoot) geom_size+=8;
        // id & coordinates
        if(info->m_desctype==1){
            geom_size += (4+4+4+4)*info->m_nnodes;
            if(headfoot) geom_size+=8*info->m_nnodes;
        }else{
            geom_size += (4+4+4+4)*info->m_nnodes;
            if(headfoot) geom_size+=8*4;
        }
        //Cell information
        // num. element
        geom_size += 4;
        if(headfoot) geom_size+=8;
        // element num.
        geom_size += (4*info->m_nelements);
        if(headfoot) geom_size+=8;
        // material num.
        geom_size += (4*info->m_nelements);
        if(headfoot) geom_size+=8;
        // element type
        geom_size += 1*info->m_nelements;
        if(headfoot) geom_size+=8;
        // connection
        geom_size += 4*info->m_connectsize;
        if(headfoot) geom_size+=8;
    }

    LOGOUT(FILTER_LOG, (char*)"|||+ filename          : %s\n",    filename         );
    LOGOUT(FILTER_LOG, (char*)"|||+ m_headfoot        : %10d\n",  info->m_headfoot       );
    LOGOUT(FILTER_LOG, (char*)"|||+ headfoot          : %10d\n",  headfoot         );
    LOGOUT(FILTER_LOG, (char*)"|||+ f_size            : %10ld\n",  f_size           );
    LOGOUT(FILTER_LOG, (char*)"|||+ m_filesize        : %10ld\n",  info->m_filesize       );
    LOGOUT(FILTER_LOG, (char*)"|||+ geom_size         : %10ld\n",  geom_size        );

    rewind(ifs);
    header = footer = 0;

    // keyword
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( keyword, 7, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );

    // version
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &version, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) version = ConvertEndianF(version);

    // title
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( info->m_title, 70, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );

    // step num.
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &stepn, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) stepn = ConvertEndianI(stepn);
    info->m_stepn = stepn;

    // step time
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &stept, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) stept = ConvertEndianF(stept);
    info->m_stept = stept;
    LOGOUT(FILTER_LOG, (char*)"|||+ keyword           : %s\n",    keyword          );
    LOGOUT(FILTER_LOG, (char*)"|||+ version           : %10.3f\n",version          );
    /*LOGOUT(FILTER_LOG, (char*)"|||+ m_title           : %s\n",    m_title          );*/
    LOGOUT(FILTER_LOG, (char*)"|||+ step_no           : %10d\n",  stepn            );
    LOGOUT(FILTER_LOG, (char*)"|||+ step_time         : %10.3f\n",stept            );

    if(param->start_step==step_loop||info->m_filesize==f_size||info->m_headfoot!=headfoot){
#if 1
        // num. nodes
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &nnodes, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(info->ENDIAN) nnodes = ConvertEndianI(nnodes);
        // description type should be 1.
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &desc_type, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(info->ENDIAN) desc_type = ConvertEndianI(desc_type);
        LOGOUT(FILTER_LOG, (char*)"|||+ nnodes            : %10d\n",   nnodes           );
        LOGOUT(FILTER_LOG, (char*)"|||+ desc_type         : %10d\n",   desc_type        );

        if (info->m_ids != NULL) free(info->m_ids);
        info->m_ids         = (int *)malloc(sizeof(int)*nnodes);

        if(info->m_desctype==1){
            i3 = 0;
            for(i=0;i<nnodes;i++,i3+= 3){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &id, 4, 1, ifs );
                fread( coord, 4, 3, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    id       = ConvertEndianI(id);
                    coord[0] = ConvertEndianF(coord[0]);
                    coord[1] = ConvertEndianF(coord[1]);
                    coord[2] = ConvertEndianF(coord[2]);
                }
                info->m_ids[i] = id;
                // add null check by @hira at 2017/04/15
                if (m_coords != NULL) m_coords[ i3     ] = coord[0];
                if (m_coords != NULL) m_coords[ i3 + 1 ] = coord[1];
                if (m_coords != NULL) m_coords[ i3 + 2 ] = coord[2];
            }
        }else{
            if(headfoot) fread( &header, 4, 1, ifs );
            fread( info->m_ids, 4, info->m_nnodes, ifs );
            if(headfoot) fread( &footer, 4, 1, ifs );
            /* X corrdinate */
            if(headfoot) fread( &header, 4, 1, ifs );
            i3 = 0;
            for(i=0;i<nnodes;i++,i3+= 3){
                // add null check by @hira at 2017/04/15
                if (m_coords != NULL) fread( &m_coords[i3], 4, 1, ifs );
                else fread( &dummy_coords, 4, 1, ifs );
            }
            if(headfoot) fread( &footer, 4, 1, ifs );
            /* Y corrdinate */
            if(headfoot) fread( &header, 4, 1, ifs );
            i3 = 1;
            for(i=0;i<nnodes;i++,i3+= 3){
                // add null check by @hira at 2017/04/15
                if (m_coords != NULL) fread( &m_coords[i3], 4, 1, ifs );
                else fread( &dummy_coords, 4, 1, ifs );
            }
            if(headfoot) fread( &footer, 4, 1, ifs );
            /* Z corrdinate */
            if(headfoot) fread( &header, 4, 1, ifs );
            i3 = 2;
            for(i=0;i<nnodes;i++,i3+= 3){
                // add null check by @hira at 2017/04/15
                if (m_coords != NULL) fread( &m_coords[i3], 4, 1, ifs );
                else fread( &dummy_coords, 4, 1, ifs );
            }
            if(headfoot) fread( &footer, 4, 1, ifs );
            if(info->ENDIAN){
                for(i=0;i<nnodes;i++){
                    info->m_ids[i] = ConvertEndianF(info->m_ids[i]);
                    // add null check by @hira at 2017/04/15
                    if (m_coords != NULL) m_coords[i*3  ] = ConvertEndianF(m_coords[i*3  ]);
                    if (m_coords != NULL) m_coords[i*3+1] = ConvertEndianF(m_coords[i*3+1]);
                    if (m_coords != NULL) m_coords[i*3+2] = ConvertEndianF(m_coords[i*3+2]);
                }
            }
        }
        for(i=0;i<nnodes;i++){
            if(info->m_ids[i] != (i+1)){
                param->nodesearch = (char)1;
                LOGOUT(FILTER_LOG, (char*)"  param->nodesearch : %d\n",param->nodesearch);
                break;
            }
        }
        /* 最大・最小を取得 */
        // add null check by @hira at 2017/04/15
        if (m_coords != NULL) {
            min->x = max->x = m_coords[0];
            min->y = max->y = m_coords[1];
            min->z = max->z = m_coords[2];
            for(i=1; i<nnodes; i++){
                if (min->x > m_coords[i*3  ]) min->x = m_coords[i*3  ];
                if (min->y > m_coords[i*3+1]) min->y = m_coords[i*3+1];
                if (min->z > m_coords[i*3+2]) min->z = m_coords[i*3+2];

                if (max->x < m_coords[i*3  ]) max->x = m_coords[i*3  ];
                if (max->y < m_coords[i*3+1]) max->y = m_coords[i*3+1];
                if (max->z < m_coords[i*3+2]) max->z = m_coords[i*3+2];
            }
            LOGOUT(FILTER_LOG, (char*)"|||+  max -> %8.2f, %8.2f, %8.2f\n",max->x,max->y,max->z);
            LOGOUT(FILTER_LOG, (char*)"|||+  min -> %8.2f, %8.2f, %8.2f\n",min->x,min->y,min->z);
        }
#else
        size = 0;
        // num. nodes
        size += 4;
        if(headfoot) size+=8;
        // description type should be 1.
        size += 4;
        if(headfoot) size+=8;
        // id & coordinates
        size += (4+4*3)*info->m_nnodes;
        if(headfoot) size+=8*info->m_nnodes;
        fseek( ifs, size, SEEK_CUR );
#endif

#if 0
        //Cell information
        size = 0;
        // num. element
        size += 4;
        if(headfoot) size+=8;
        // element num.
        size += (4*info->m_nelements);
        if(headfoot) size+=8;
        // material num.
        size += (4*info->m_nelements);
        if(headfoot) size+=8;
        // element type
        size += 1*info->m_nelements;
        if(headfoot) size+=8;
        // connection
        size += 4*info->m_connectsize;
        if(headfoot) size+=8;
        fseek( ifs, size, SEEK_CUR );
#endif
        // num. element
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &nelements, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        // element num.
        size = nelements*sizeof(int);
        if(headfoot) size += 2*sizeof(int);
        fseek( ifs, size, SEEK_CUR );
        // material num.
        fseek( ifs, size, SEEK_CUR );
        // element type
        elemtypes = (char *)malloc(sizeof(char) * nelements );
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( elemtypes, 1, nelements, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );

        // connection
        connectsize = 0;
        for(i=0;i<nelements;i++){
            if(elemtypes[i]<=1  ||
               elemtypes[i]==8  ||
               elemtypes[i]==12 ||
               elemtypes[i]==13 ||
               elemtypes[i]>=15) {
                LOGOUT(FILTER_ERR, (char*)"Unkown element type : %d\n",elemtypes[i]);
                goto garbage;
            }
            connectsize+=info->m_elementsize[(int)elemtypes[i]];
        }
        size = connectsize*sizeof(int);
        if(headfoot) size+=2*sizeof(int);
        fseek( ifs, size, SEEK_CUR );
        free(elemtypes);
    }

    //Data information

    // num. component of nodes
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &ncomponents, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) header      = ConvertEndianI(header     );
    if(info->ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    if(info->ENDIAN) footer      = ConvertEndianI(footer     );
    LOGOUT(FILTER_LOG, (char*)"|||+ ncomponents       : %10d\n",   ncomponents      );
    LOGOUT(FILTER_LOG, (char*)"|||+ m_nnodes          : %10d\n",   info->m_nnodes         );
    LOGOUT(FILTER_LOG, (char*)"|||+ m_nnodekinds      : %10d\n",   info->m_nnodekinds     );
    nnodecomponents = ncomponents;
    if(ncomponents>0){
        // data type
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &datatype, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(info->ENDIAN) header   = ConvertEndianI(header     );
        if(info->ENDIAN) datatype = ConvertEndianI(datatype);
        if(info->ENDIAN) footer   = ConvertEndianI(footer     );
        LOGOUT(FILTER_LOG, (char*)"|||+ datatype          : %10d\n",   datatype         );
        nkind = 0;
        if(datatype==1){
            // data header
            for(i=0;i<ncomponents;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, 1, COMPONENT_LEN, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    header    = ConvertEndianI(header);
                    footer    = ConvertEndianI(footer);
                    veclen    = ConvertEndianI(veclen);
                    null_flag = ConvertEndianI(null_flag);
                    null_data = ConvertEndianF(null_data);
                }
                info->m_veclens[i] = veclen;
                for(j=0;j<veclen;j++){
                    info->m_nullflags[nkind+j] = null_flag;
                    info->m_nulldatas[nkind+j] = null_data;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[i][m] = component_name[m];
                    info->pm_units[i][m] = unit[m]          ;
                }
                nkind += veclen;
                LOGOUT(FILTER_LOG, (char*)"||||  component_name    : %s\n",     component_name  );
            }
            value = (float *)malloc(sizeof(float) * nkind );
            if(NULL==value){
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
                goto garbage;
            }

            for(i=0;i<nnodes;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( value, 4, nkind, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    for(j=0;j<nkind;j++){
                        value[j] = ConvertEndianF(value[j]);
                    }
                }
                if (i<info->m_nnodes) {
                    for(j=0;j<nkind;j++){
                        info->m_values[i*info->m_nkinds+j]=value[j];
                    }
                }
            }
            free(value);
            value = NULL;
        }else if(datatype==2){
            for(i=0;i<ncomponents;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, 1, COMPONENT_LEN, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                info->m_veclens[i] = veclen;
                for(j=0;j<veclen;j++){
                    info->m_nullflags[nkind+j] = null_flag;
                    info->m_nulldatas[nkind+j] = null_data;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[i][m] = component_name[m];
                    info->pm_units[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }

            value = (float *)malloc(sizeof(float) * nnodes );
            if(NULL==value){
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
                goto garbage;
            }

            for(i=0;i<info->m_nkinds;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( value, 4, nnodes, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    for(j=0;j<info->m_nnodes;j++){
                        value[j] = ConvertEndianF(value[j]);
                    }
                }
                for(j=0;j<info->m_nnodes;j++){
                    info->m_values[j*info->m_nkinds+i]=value[j];
                }
            }
            free(value);
            value = NULL;
        }else if(datatype==3){
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                info->m_veclens[i] = veclen;
                for(j=0;j<veclen;j++){
                    info->m_nullflags[nkind+j] = 1;
                    info->m_nulldatas[nkind+j] = -99999999.;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[i][m] = component_name[m];
                    info->pm_units[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            nkind = 0;
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nnodes, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    nnodes = ConvertEndianI(nnodes);
                }
                ids   = (int   *)malloc(sizeof(float) * nnodes );
                value = (float *)malloc(sizeof(float) * info->m_veclens[i] );
                if(NULL==value||NULL==ids){
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
                    goto garbage;
                }
                for(j=0;j<info->m_nnodes;j++){
                    for(k=0;k<info->m_veclens[i];k++){
                        info->m_values[j*info->m_nkinds+nkind+k] = info->m_nulldatas[nkind+k];
                    }
                }
                for(j=0;j<nnodes;j++){
                    if(headfoot) fread( &header, 4, 1, ifs );
                    fread( &ids[j], 4, 1, ifs );
                    fread( value  , 4, info->m_veclens[i], ifs );
                    if(headfoot) fread( &footer, 4, 1, ifs );
                    if(info->ENDIAN){
                        ids[j]   = ConvertEndianI(ids[j]);
                        for(k=0;k<info->m_veclens[i];k++){
                            value[k] = ConvertEndianF(value[k]);
                        }
                    }
                    for(k=0;k<info->m_veclens[i];k++){
                        info->m_values[(ids[j]-1)*info->m_nkinds+nkind+k] = value[k];
                    }
                }
                nkind += info->m_veclens[i];
                free(value);
                value = NULL;
                free(ids);
                ids = NULL;
            }
        }else if(info->m_datatype==4){
            nkind = 0;
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                info->m_veclens[i] = veclen;
                for(j=0;j<veclen;j++){
                    info->m_nullflags[nkind+j] = 1;
                    info->m_nulldatas[nkind+j] = -99999999.;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[i][m] = component_name[m];
                    info->pm_units[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            nkind = 0;
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nnodes, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    nnodes = ConvertEndianI(nnodes);
                }

                ids   = (int   *)malloc(sizeof(float) * nnodes );
                value = (float *)malloc(sizeof(float) * nnodes );
                if(NULL==value||NULL==ids){
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
                    goto garbage;
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( ids, 4, nnodes, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    for(j=0;j<nnodes;j++){
                        ids[j] = ConvertEndianI(ids[j]);
                    }
                }
                for(j=0;j<info->m_veclens[i];j++){
                    if(headfoot) fread( &header, 4, 1, ifs );
                    fread( value , 4, nnodes, ifs );
                    if(headfoot) fread( &footer, 4, 1, ifs );
                    if(info->ENDIAN) {
                        for(k=0;k<nnodes;k++){
                            value[k] = ConvertEndianF(value[k]);
                        }
                    }
                    for(k=0;k<info->m_nnodes;k++){
                        info->m_values[k*info->m_nkinds+nkind+j] = info->m_nulldatas[nkind+j];
                    }
                    for(k=0;k<nnodes;k++){
                        info->m_values[(ids[k]-1)*info->m_nkinds+nkind+j] = value[k];
                    }
                }
                nkind += info->m_veclens[i];
                free(value);
                value = NULL;
                free(ids);
                ids = NULL;
            }
        }
    }

    // num. component of elements
    if(headfoot) fread( &header, 4, 1, ifs );
    size = fread( &ncomponents, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(info->ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    LOGOUT(FILTER_LOG, (char*)"|||+ ncomponents       : %10d\n",   ncomponents      );
    LOGOUT(FILTER_LOG, (char*)"|||+ m_nelements       : %10d\n",   info->m_nelements      );
    LOGOUT(FILTER_LOG, (char*)"|||+ m_nelemkinds      : %10d\n",   info->m_nelemkinds     );

    if(ncomponents>0){
        // data type
        float    *elem_value = NULL;
        int      *effe_num   = NULL;
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &datatype, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(info->ENDIAN) datatype = ConvertEndianI(datatype);
        LOGOUT(FILTER_LOG, (char*)"|||+ datatype          : %10d\n",   datatype         );

        elem_value = (float*)malloc(sizeof(float)*info->m_nelements*info->m_nelemkinds);
        if(NULL==elem_value){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
            goto garbage;
        }
        nkind = 0;
        if(datatype==1){
            // data header
            for(i=0;i<ncomponents;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, 1, COMPONENT_LEN, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                info->m_veclens[nnodecomponents+i] = veclen;
                for(j=0;j<veclen;j++){
                    info->m_nullflags[info->m_nnodekinds+nkind+j] = null_flag;
                    info->m_nulldatas[info->m_nnodekinds+nkind+j] = null_data;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[nnodecomponents+i][m] = component_name[m];
                    info->pm_units[nnodecomponents+i][m] = unit[m]          ;
                }
                nkind += veclen;
            }

            value    = (float *)malloc(sizeof(float)*nkind );
            if(NULL==value){
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
                goto garbage;
            }

            for(i=0;i<info->m_nelements;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( value, 4, nkind, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    for(j=0;j<nkind;j++){
                        value[j] = ConvertEndianF(value[j]);
                    }
                }
                for(j=0;j<nkind;j++){
                    elem_value[i*nkind+j]=value[j];
                }
            }
        }else if(datatype==2){
            for(i=0;i<ncomponents;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, 1, COMPONENT_LEN, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                info->m_veclens[nnodecomponents+i] = veclen;
                for(j=0;j<veclen;j++){
                    info->m_nullflags[info->m_nnodekinds+nkind+j] = null_flag;
                    info->m_nulldatas[info->m_nnodekinds+nkind+j] = null_data;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[nnodecomponents+i][m] = component_name[m];
                    info->pm_units[nnodecomponents+i][m] = unit[m]          ;
                }
                nkind += veclen;
            }

            value = (float *)malloc(sizeof(float) * info->m_nelements );
            if(NULL==value){
                LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
                goto garbage;
            }
            for(i=0;i<nkind;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( value, 4, info->m_nelements, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    for(j=0;j<info->m_nelements;j++){
                        value[j] = ConvertEndianF(value[j]);
                    }
                }
                for(j=0;j<info->m_nelements;j++){
                    elem_value[j*info->m_nelemkinds+i] = value[j];
                }
            }
            free(value);
            value = NULL;
        }else if(datatype==3){
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                info->m_veclens[nnodecomponents+i] = veclen;
                for(j=0;j<veclen;j++){
                    info->m_nullflags[info->m_nnodekinds+nkind+j] = 1;
                    info->m_nulldatas[info->m_nnodekinds+nkind+j] = -99999999.;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[nnodecomponents+i][m] = component_name[m];
                    info->pm_units[nnodecomponents+i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            nkind = 0;
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nelements, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    nelements = ConvertEndianI(nelements);
                }
                ids   = (int   *)malloc(sizeof(float) * nelements );
                value = (float *)malloc(sizeof(float) * info->m_veclens[nnodecomponents-1+i] );
                if(NULL==value||NULL==ids){
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
                    goto garbage;
                }
                for(j=0;j<info->m_nelements;j++){
                    for(k=0;k<info->m_veclens[nnodecomponents+i];k++){
                        elem_value[j*info->m_nelemkinds+nkind+k] = info->m_nulldatas[info->m_nnodekinds+nkind+k];
                    }
                }
                for(j=0;j<nelements;j++){
                    if(headfoot) fread( &header, 4, 1, ifs );
                    fread( &ids[j], 4, 1, ifs );
                    fread( value  , 4, info->m_veclens[nnodecomponents+i], ifs );
                    if(headfoot) fread( &footer, 4, 1, ifs );
                    if(info->ENDIAN){
                        ids[j]   = ConvertEndianI(ids[j]);
                        for(k=0;k<info->m_veclens[nnodecomponents+i];k++){
                            value[k] = ConvertEndianF(value[k]);
                        }
                    }
                    eid=0;
                    for(k=0;k<info->m_nelements;k++){
                        if(ids[j]==info->m_elementids[k]){
                            eid = k;
                            break;
                        }
                    }
                    for(k=0;k<info->m_veclens[nnodecomponents+i];k++){
                        elem_value[eid*info->m_nelemkinds+nkind+k] = value[k];
                    }
                }
                nkind += info->m_veclens[nnodecomponents+i];
                free(value);
                value = NULL;
                free(ids);
                ids = NULL;
            }
        }else if(info->m_datatype==4){
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                info->m_veclens[nnodecomponents+i] = veclen;
                for(j=0;j<veclen;j++){
                    info->m_nullflags[info->m_nnodekinds+nkind+j] = 1;
                    info->m_nulldatas[info->m_nnodekinds+nkind+j] = -99999999.;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    info->pm_names[nnodecomponents+i][m] = component_name[m];
                    info->pm_units[nnodecomponents+i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            nkind = 0;
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nelements, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    nelements = ConvertEndianI(nelements);
                }

                ids   = (int   *)malloc(sizeof(float) * nelements );
                value = (float *)malloc(sizeof(float) * nelements );
                if(NULL==value||NULL==ids){
                    LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
                    goto garbage;
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( ids, 4, nelements, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(info->ENDIAN){
                    for(j=0;j<nelements;j++){
                        ids[j] = ConvertEndianI(ids[j]);
                    }
                }
                for(j=0;j<info->m_veclens[nnodecomponents+i];j++){
                    if(headfoot) fread( &header, 4, 1, ifs );
                    fread( value , 4, nelements, ifs );
                    if(headfoot) fread( &footer, 4, 1, ifs );
                    if(info->ENDIAN) {
                        for(k=0;k<nelements;k++){
                            value[k] = ConvertEndianF(value[k]);
                        }
                    }
                    for(k=0;k<info->m_nelements;k++){
                        elem_value[k*info->m_nelemkinds+nkind+j] = info->m_nulldatas[info->m_nnodekinds+nkind+j];
                    }
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(k,m)
#endif
                    for(k=0;k<nelements;k++){
                        for(m=0;m<info->m_nelements;m++){
                            if(ids[k]==info->m_elementids[m]){
                                ids[k] = m+1;
                                break;
                            }
                        }
                    }
                    for(k=0;k<nelements;k++){
                        elem_value[(ids[k]-1)*info->m_nelemkinds+nkind+j] = value[k];
                    }
                }
                nkind += info->m_veclens[nnodecomponents+i];
                free(value);
                value = NULL;
                free(ids);
                ids = NULL;
            }
        }

        value = (float *)malloc(sizeof(float) * info->m_nelemkinds);
        effe_num = (int *)malloc(sizeof(int) * info->m_nelemkinds);
        if(NULL==value||NULL==effe_num){
            LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
            goto garbage;
        }
        for(i=0;i<info->m_nnodes;i++){
            for(k=0;k<info->m_nelemkinds;k++){
                value[k] = 0.0;
                effe_num[k] = 0;
            }
            for(j=0;j<info->m_elem_in_node[i];j++){
                eid = info->m_elem_in_node_array[info->m_elem_in_node_addr[i]+j] - 1;
                for(k=0;k<info->m_nelemkinds;k++){
                    if((info->m_nullflags[info->m_nnodekinds+k])&&
                       (info->m_nulldatas[info->m_nnodekinds+k]==elem_value[eid*info->m_nelemkinds+k])){
                        ;
                    }else{
                        value[k] = value[k] + elem_value[eid*info->m_nelemkinds+k];
                        effe_num[k]++ ;
                    }
                }
            }
            for(k=0;k<info->m_nelemkinds;k++){
                if(effe_num[k]!=0){
                    info->m_values[i*info->m_nkinds+info->m_nnodekinds+k] = value[k] / effe_num[k];
                }else{
                    info->m_values[i*info->m_nkinds+info->m_nnodekinds+k] = info->m_nulldatas[info->m_nnodekinds+k];
                }
            }
        }
        free(value);
        value = NULL;
        free(effe_num);
        effe_num = NULL;
        free(elem_value);
        elem_value = NULL;
    }

    size = info->m_nkinds*(step_loop-param->start_step);
    for(j=0;j<info->m_nkinds;j++){
        if(info->m_nullflags[j]){
            info->m_value_max[size+j] = FLT_MIN;
            info->m_value_min[size+j] = FLT_MAX;
            for(i=0;i<info->m_nnodes;i++){
                if(info->m_nulldatas[j]!=info->m_values[i*info->m_nkinds+j]){
                    if(info->m_value_max[size+j]<info->m_values[i*info->m_nkinds+j]) info->m_value_max[size+j] = info->m_values[i*info->m_nkinds+j];
                    if(info->m_value_min[size+j]>info->m_values[i*info->m_nkinds+j]) info->m_value_min[size+j] = info->m_values[i*info->m_nkinds+j];
                }
            }
        }else{
            info->m_value_max[size+j] = info->m_values[j];
            info->m_value_min[size+j] = info->m_values[j];
            for(i=1;i<info->m_nnodes;i++){
                if(info->m_value_max[size+j]<info->m_values[i*info->m_nkinds+j]) info->m_value_max[size+j] = info->m_values[i*info->m_nkinds+j];
                if(info->m_value_min[size+j]>info->m_values[i*info->m_nkinds+j]) info->m_value_min[size+j] = info->m_values[i*info->m_nkinds+j];
            }
        }
    }
    for(j=0;j<info->m_nkinds;j++){
        // modify null check by @hira at 2017/04/10
        if (info->mt_nullflag != NULL) info->mt_nullflag[size+j] = info->m_nullflags[j];
        if (info->mt_nulldata != NULL) info->mt_nulldata[size+j] = info->m_nulldatas[j];
    }
//  free(value);
    state = RTC_OK;

garbage:
    if(NULL!=ifs) fclose(ifs);
    return(state);
}
/*****************************************************************************/
/*
 * サブボリューム単位の出力
 *
 */
int FilterIoAvs::write_ucdbin( int      step_loop     ,
                  int      volm_loop     ,
                  int      subvol_no     ,
                  Int64 subvol_nelems ,
                  Int64 subvol_nnodes )
{

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char* filename = info->filename;
    float *m_coords = info->m_coords;

    int           state = RTC_NG;
    FILE         *ifs = NULL;
    int           size;
    int           header;
    int           footer;
    float              version;
    int           stepn;
    float         stept;
    int           desc_type;
    int           nnodes;
    int           id;
    float         coord[3];
    int           element_id;
    int           material_id;
    int           nelements;
    char          element_type;
    int           ncomponents;
    int           data_type;
    char          component_name[COMPONENT_LEN];
    char          unit[COMPONENT_LEN];
    int           null_flag;
    float         null_data;
    int           nkind;
    Int64         i;
    Int64         j;
    Int64         k;
    unsigned int  inum;
    unsigned int  jnum;
    unsigned int  knum;
    unsigned int  nid;
    unsigned int  connect[8];
    unsigned int  org_id;
    unsigned int  new_id;
    int           veclen;
    float        *value = NULL;
    int          *ivalue = NULL;
    char          headfoot = 0;
    char          keyword[7+1] = "AVS UCD";

    sprintf(filename,"%s/%s.%05d.%07d.%07d.%s",param->out_dir,param->out_prefix,step_loop,volm_loop+1,info->m_nvolumes,param->out_suffix);

    if(param->headfoot){
        headfoot = (char) 1;
    }

    /* オープン */
    ifs = fopen(filename, "wb");
    if(NULL==ifs){
        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
        goto garbage;
    }
    //Head information

    header = 7;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;

    // keyword
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( keyword, 7, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // version
    version = 1.0f;
    header = 4;
    if(info->ENDIAN) version = ConvertEndianF(version);
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &version, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // title
    header = 70;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( info->m_title, 70, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // step num.
    stepn = info->m_stepn;
    header = 4;
    if(info->ENDIAN) header = ConvertEndianI(header);
    if(info->ENDIAN) stepn  = ConvertEndianI(stepn);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &stepn, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // step time
    stept = info->m_stept;
    header = 4;
    if(info->ENDIAN) header = ConvertEndianI(header);
    if(info->ENDIAN) stept  = ConvertEndianF(stept);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &stept, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    //Node information

    // num. nodes
    nnodes = subvol_nnodes;
    header = 4;
    if(info->ENDIAN) header = ConvertEndianI(header);
    if(info->ENDIAN) nnodes = ConvertEndianI(nnodes);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &nnodes, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // description type should be 1.
    desc_type = info->m_desctype;
    header = 4;
    if(info->ENDIAN) header    = ConvertEndianI(header);
    if(info->ENDIAN) desc_type = ConvertEndianI(desc_type);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &desc_type, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // coordinates
    id = 0;
    header = 16;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    for(i=0;i<info->m_nnodes;i++){
        if((char) 0==info->m_nodeflag[i]){
            continue;
        }
//      id = info->m_ids[i];
        id++;
        coord[0] = m_coords[ i*3     ];
        coord[1] = m_coords[ i*3 + 1 ];
        coord[2] = m_coords[ i*3 + 2 ];
        if(info->ENDIAN){
            id = ConvertEndianI(id);
            coord[0] = ConvertEndianF(coord[0]);
            coord[1] = ConvertEndianF(coord[1]);
            coord[2] = ConvertEndianF(coord[2]);
        }
        if(headfoot) fwrite( &header, 4, 1, ifs );
        fwrite( &id, 4, 1, ifs );
        fwrite( coord, 4, 3, ifs );
        if(headfoot) fwrite( &footer, 4, 1, ifs );
    }

    //Cell information

    // num. elemen
    nelements = subvol_nelems;
    header = 4;
    if(info->ENDIAN) nelements = ConvertEndianI(nelements);
    if(info->ENDIAN) header    = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &nelements, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // element num.
    header = 4*subvol_nelems;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<info->m_nelements;i++){
       if(subvol_no == info->m_subvolume_num[i]){
           element_id = info->m_elementids[i] ;
           if(info->ENDIAN) element_id = ConvertEndianI(element_id);
           fwrite( &element_id, 4, 1, ifs );
       }
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // material num.
    header = 4*subvol_nelems;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<info->m_nelements;i++){
        if(subvol_no == info->m_subvolume_num[i]) {
            material_id = info->m_materials[i] ;
            if(info->ENDIAN) material_id = ConvertEndianI(material_id);
            fwrite( &material_id, 4, 1, ifs );
        }
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // element type
    header = 1*subvol_nelems;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<info->m_nelements;i++){
        if(subvol_no==info->m_subvolume_num[i]) {
            element_type = info->m_elemtypes[i] ;
            fwrite( &element_type, 1, 1, ifs );
        }
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // connect
    ivalue = (int *)malloc(sizeof(int) * 20 );
    header = 4*subvol_nelems*info->m_elemcoms;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    size = 0;
    j = 0;
    if(param->struct_grid){
        inum = param->dim1;
        jnum = param->dim2;
        knum = param->dim3;
        nelements = (inum-1)*(jnum-1)*(knum-1);
        size = 8;
        for(i=0;i<info->m_nelements;i++){
            if(subvol_no==info->m_subvolume_num[i]) {
                nid = info->m_conn_top_node[i];
                connect[0] = nid;
                connect[1] = connect[0] + 1;
                connect[2] = connect[1] + inum;
                connect[3] = connect[0] + inum;
                connect[4] = connect[0] + inum*jnum;
                connect[5] = connect[4] + 1;
                connect[6] = connect[5] + inum;
                connect[7] = connect[4] + inum;
                k = 0;
                while(k<size){
                    org_id = connect[k];
                    new_id = info->m_nodes_trans[org_id-1];
                    ivalue[k] = new_id - 1 + 1;
                    if(info->ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
                    k++;
                }
                fwrite( ivalue, 4, size, ifs );
            }
        }
    }else{
        for(i=0;i<info->m_nelements;i++){
            size = info->m_elementsize[(int)info->m_elemtypes[i]];
            if(subvol_no==info->m_subvolume_num[i]) {
                k = 0;
                while(k<size){
                    org_id = info->m_connections[j+k];
                    new_id = info->m_nodes_trans[org_id-1];
                    ivalue[k] = new_id - 1 + 1;
                    if(info->ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
                    k++;
                }
                fwrite( ivalue, 4, size, ifs );
            }
            j+=size;
        }
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );
    free(ivalue);
    ivalue = NULL;

    //Data information

    // num. component
    ncomponents = info->m_ncomponents;
    header = 4;
    if(info->ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    if(info->ENDIAN) header      = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &ncomponents, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // data type
    data_type = info->m_datatype;
    header = 4;
    if(info->ENDIAN) data_type = ConvertEndianI(data_type);
    if(info->ENDIAN) header    = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &data_type, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // data header
    null_flag = 0;
    null_data = 0.0f;
    for(i=0;i<info->m_ncomponents;i++){

        header = 44;
        for(j=0;j<COMPONENT_LEN;j++){
           component_name[j] = info->pm_names[i][j];
           unit[j]           = info->pm_units[i][j];
        }
        veclen = info->m_veclens[i];
        null_flag = info->m_nullflags[i];
        null_data = info->m_nulldatas[i];
        if(info->ENDIAN){
            header         = ConvertEndianI(header);
            veclen         = ConvertEndianI(veclen);
            null_flag      = ConvertEndianI(null_flag);
            null_data      = ConvertEndianF(null_data);
        }
        footer = header;
        if(headfoot) fwrite( &header, 4, 1, ifs );
        fwrite( component_name, COMPONENT_LEN, 1, ifs );
        fwrite( unit, COMPONENT_LEN, 1, ifs );
        fwrite( &veclen, 4, 1, ifs );
        fwrite( &null_flag, 4, 1, ifs );
        fwrite( &null_data, 4, 1, ifs );
        if(headfoot) fwrite( &footer, 4, 1, ifs );

    }

    // value
    nkind = info->m_nkinds;
    value = (float *)malloc(sizeof(float) * nkind );
    header = 4*nkind;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    for(i=0;i<info->m_nnodes;i++){
        if((char) 1==info->m_nodeflag[i]){
            j = i*nkind;
            k = 0;
            while(k<nkind){
                value[k] = info->m_values[j+k];
                if(info->ENDIAN) value[k] = ConvertEndianF(value[k]);
                k++;
            }
            if(headfoot) fwrite( &header, 4, 1, ifs );
            fwrite( value, 4, nkind, ifs );
            if(headfoot) fwrite( &footer, 4, 1, ifs );
        }
    }

    header = 4;
    if(info->ENDIAN) header = ConvertEndianI(header);
    if(headfoot) fwrite( &header, 4, 1, ifs );
    header = 0;
    if(info->ENDIAN) header = ConvertEndianI(header);
    fwrite( &header, 4, 1, ifs );
    header = 4;
    if(info->ENDIAN) header = ConvertEndianI(header);
    if(headfoot) fwrite( &header, 4, 1, ifs );

    free(value);
    value = NULL;
    state = RTC_OK;
garbage:
    if(NULL!=ifs) fclose(ifs);
    return(state);
}

/*****************************************************************************/
/*
 * サブボリューム単位の出力
 *
 */
int FilterIoAvs::write_ucdbin_elem( int      step_loop     ,
                  int      elem_id       ,
                  Int64 subvol_nelems ,
                  Int64 subvol_nnodes ,
                  Int64 rank_nodes    ,
                  Int64 rank_elems    )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char* buff = info->buff;
    char* filename = info->filename;
    float *m_coords = info->m_coords;

    int           state = RTC_NG;
    FILE         *ifs = NULL;
    int           size;
    int           header;
    int           footer;
    float         version;
    int           stepn;
    float         stept;
    int           desc_type;
    int           nnodes;
    int           id;
    float         coord[3];
    int           element_id;
    int           material_id;
    int           nelements;
    char          element_type;
    int           ncomponents;
    int           data_type;
    char          component_name[COMPONENT_LEN];
    char          unit[COMPONENT_LEN];
    int           null_flag;
    float         null_data;
    Int64         i;
    Int64         j;
    Int64         k;
    unsigned int  org_id;
    unsigned int  new_id;
    int           veclen;
    float        *value = NULL;
    int          *ivalue = NULL;
    char          headfoot = 0;
    char          keyword[7+1] = "AVS UCD";
    int           elem_type = 0;
    int           nkind;

    elem_type = elem_id;

    sprintf(buff,param->format,step_loop);
    sprintf(filename,"%s/%02d-%s%s%s",param->out_dir,elem_type,param->out_prefix,buff,param->in_suffix);
    LOGOUT(FILTER_LOG, (char*)"|||| filename          : %s\n",    filename         );

//  if(param->headfoot){
//      headfoot = (char) 1;
//  }
    headfoot = (char) 0;

    /* オープン */
    ifs = fopen(filename, "wb");
    if(NULL==ifs){
        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", filename);
        goto garbage;
    }
    //Head information

    header = 7;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;

    // keyword
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( keyword, 7, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // version
    version = 1.0f;
    header = 4;
    if(info->ENDIAN) version = ConvertEndianF(version);
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &version, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // title
    header = 70;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( info->m_title, 70, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // step num.
    stepn = step_loop;
    header = 4;
    if(info->ENDIAN) header = ConvertEndianI(header);
    if(info->ENDIAN) stepn  = ConvertEndianI(stepn);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &stepn, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // step time
    stept = info->m_stept;
    header = 4;
    if(info->ENDIAN) header = ConvertEndianI(header);
    if(info->ENDIAN) stept  = ConvertEndianF(stept);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &stept, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    //Node information

    // num. nodes
    nnodes = subvol_nnodes;
    header = 4;
    if(info->ENDIAN) header = ConvertEndianI(header);
    if(info->ENDIAN) nnodes = ConvertEndianI(nnodes);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &nnodes, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // description type should be 1.
    desc_type = 1;
    header = 4;
    if(info->ENDIAN) header    = ConvertEndianI(header);
    if(info->ENDIAN) desc_type = ConvertEndianI(desc_type);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &desc_type, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // coordinates
    id = 0;
    header = 16;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;

    for(i=0;i<subvol_nnodes;i++){
        id = info->m_subvolume_nodes[rank_nodes+i];
        j = id * 3;
        coord[0] = m_coords[j  ];
        coord[1] = m_coords[j+1];
        coord[2] = m_coords[j+2];
        id++;
///////////////////
        id = i+1;
///////////////////
        if(info->ENDIAN){
            id = ConvertEndianI(id);
            coord[0] = ConvertEndianF(coord[0]);
            coord[1] = ConvertEndianF(coord[1]);
            coord[2] = ConvertEndianF(coord[2]);
        }
        if(headfoot) fwrite( &header, 4, 1, ifs );
        fwrite( &id, 4, 1, ifs );
        fwrite( coord, 4, 3, ifs );
        if(headfoot) fwrite( &footer, 4, 1, ifs );
    }

    //Cell information

    // num. elemen
    nelements = subvol_nelems;
    header = 4;
    if(info->ENDIAN) nelements = ConvertEndianI(nelements);
    if(info->ENDIAN) header    = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &nelements, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // element num.
    header = 4*subvol_nelems;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<subvol_nelems;i++){
         element_id = i+1 ;
         if(info->ENDIAN) element_id = ConvertEndianI(element_id);
         fwrite( &element_id, 4, 1, ifs );
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // material num.
    header = 4*subvol_nelems;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<info->m_nelements;i++){
        if(elem_type==info->m_elemtypes[i]) {
            material_id = info->m_materials[i];
            if(info->ENDIAN) material_id = ConvertEndianI(material_id);
            fwrite( &material_id, 4, 1, ifs );
        }
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // element type
    header = 1*subvol_nelems;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<subvol_nelems;i++){
            element_type = elem_type ;
            fwrite( &element_type, 1, 1, ifs );
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // connect
    ivalue = (int *)malloc(sizeof(int) * 20 );
    header = 4*subvol_nelems*info->m_elemcoms;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    size = 0;
    j = 0;
    for(i=0;i<info->m_nelements;i++){
        size = info->m_elementsize[(int)info->m_elemtypes[i]];
        if(elem_type==info->m_elemtypes[i]) {
            k = 0;
            while(k<size){
                org_id = info->m_connections[j+k];
                new_id = info->m_nodes_trans[org_id-1];
                ivalue[k] = new_id - 1 + 1;
                if(info->ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
                k++;
            }
            fwrite( ivalue, 4, size, ifs );
        }
        j+=size;
    }

    if(headfoot) fwrite( &footer, 4, 1, ifs );
    free(ivalue);
    ivalue = NULL;

    //Data information

    // num. component
    ncomponents = info->m_ncomponents;
    header = 4;
    if(info->ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    if(info->ENDIAN) header      = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &ncomponents, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // data type
    data_type = 1;
    header = 4;
    if(info->ENDIAN) data_type = ConvertEndianI(data_type);
    if(info->ENDIAN) header    = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &data_type, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // data header
    null_flag = 0;
    null_data = 0.0f;
    nkind = 0;
    for(i=0;i<info->m_ncomponents;i++){

        header = 44;
        for(j=0;j<COMPONENT_LEN;j++){
           component_name[j] = info->pm_names[i][j];
           unit[j]           = info->pm_units[i][j];
        }
//      strcpy(component_name,pm_names[i]);
//      strcpy(unit          ,pm_units[i]);
        veclen    = info->m_veclens[i];
        null_flag = info->m_nullflags[nkind];
        null_data = info->m_nulldatas[nkind];
        if(info->ENDIAN){
            header         = ConvertEndianI(header);
            veclen         = ConvertEndianI(veclen);
            null_flag      = ConvertEndianI(null_flag);
            null_data      = ConvertEndianF(null_data);
        }
        footer = header;
        if(headfoot) fwrite( &header, 4, 1, ifs );
        fwrite( component_name, COMPONENT_LEN, 1, ifs );
        fwrite( unit, COMPONENT_LEN, 1, ifs );
        fwrite( &veclen, 4, 1, ifs );
        fwrite( &null_flag, 4, 1, ifs );
        fwrite( &null_data, 4, 1, ifs );
        if(headfoot) fwrite( &footer, 4, 1, ifs );
        nkind += info->m_veclens[i];
    }

    value = (float *)malloc(sizeof(float) * info->m_nkinds );
    header = 4*info->m_nkinds;
    if(info->ENDIAN) header = ConvertEndianI(header);
    footer = header;
    for(i=0;i<subvol_nnodes;i++){
        id = info->m_subvolume_nodes[rank_nodes+i];
        j = id*info->m_nkinds;
        k = 0;
        while(k<info->m_nkinds){
            value[k] = info->m_values[j+k];
            if(info->ENDIAN) value[k] = ConvertEndianF(value[k]);
            k++;
        }
        if(headfoot) fwrite( &header, 4, 1, ifs );
        fwrite( value, 4, info->m_nkinds, ifs );
        if(headfoot) fwrite( &footer, 4, 1, ifs );
    }
    free(value);
    value = NULL;

    header = 4;
    if(info->ENDIAN) header = ConvertEndianI(header);
    if(headfoot) fwrite( &header, 4, 1, ifs );
    header = 0;
    if(info->ENDIAN) header = ConvertEndianI(header);
    fwrite( &header, 4, 1, ifs );
    header = 4;
    if(info->ENDIAN) header = ConvertEndianI(header);
    if(headfoot) fwrite( &header, 4, 1, ifs );

    state = RTC_OK;
garbage:
    if(NULL!=ifs) fclose(ifs);
    return(state);
}


/*
 * サブボリューム単位の出力
 *
 */
int FilterIoAvs::write_ucdbin_data(
                    int      step_loop     ,
                    int      elem_id       ,
                    Int64 subvol_nelems ,
                    Int64 subvol_nnodes ,
                    Int64 rank_nodes    ,
                    Int64 rank_elems    )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    float *m_coords = info->m_coords;
    NodeInf *min = &info->node_min;
    NodeInf *max = &info->node_max;

    int           state = RTC_NG;
    FILE         *ifs = NULL;
    int           size;
    int           stepn;
    float         stept;
    int           desc_type;
    int           nnodes;
    int           id;
    float         coord[3];
    int           element_id;
    int           material_id;
    int           nelements;
    char          element_type;
    int           ncomponents;
    int           data_type;
    char          component_name[COMPONENT_LEN];
    char          unit[COMPONENT_LEN];
    int           null_flag;
    float         null_data;
    Int64         i;
    Int64         j;
    Int64         k;
    Int64         n;
    unsigned int  org_id;
    unsigned int  new_id;
    int           veclen;
    float        *value = NULL;
    int          *ivalue = NULL;
    char          headfoot = 0;
    char          keyword[7+1] = "AVS UCD";
    int           elem_type = 0;
    int           nkind;
    int total_nelements = 0;

    elem_type = elem_id;

    // keyword
    // version
    // title
    // step num.
    info->m_stepn = step_loop;
    // step time
    //Node information
    // num. nodes
    nnodes = subvol_nnodes;
    info->m_nnodes = nnodes;

    // description type should be 1.
    // coordinates
    float *coords = (float*)malloc(sizeof(float)*subvol_nnodes*3);
    memset(coords, 0x00, sizeof(float)*subvol_nnodes*3);
    id = 0;
    int count = 0;
    for(i=0;i<subvol_nnodes;i++){
        id = info->m_subvolume_nodes[rank_nodes+i];
        j = id * 3;
        info->m_ids[i] = i+1;
        coords[count++] = m_coords[j  ];
        coords[count++] = m_coords[j+1];
        coords[count++] = m_coords[j+2];
    }
    memcpy(m_coords, coords, sizeof(float)*subvol_nnodes*3);
    free(coords);

    for(i=0;i<subvol_nnodes;i++){
        if(info->m_ids[i] != (i+1)){
            param->nodesearch = (char)1;
            LOGOUT(FILTER_LOG, (char*)"  param->nodesearch : %d\n",param->nodesearch);
            break;
        }
    }

    /* 最大・最小を取得 */
    // add null check by @hira at 2017/04/15
    if (m_coords != NULL) {
        min->x = max->x = m_coords[0];
        min->y = max->y = m_coords[1];
        min->z = max->z = m_coords[2];
        for(i=1; i<info->m_nnodes; i++){
            if (min->x > m_coords[i*3  ]) min->x = m_coords[i*3  ];
            if (min->y > m_coords[i*3+1]) min->y = m_coords[i*3+1];
            if (min->z > m_coords[i*3+2]) min->z = m_coords[i*3+2];

            if (max->x < m_coords[i*3  ]) max->x = m_coords[i*3  ];
            if (max->y < m_coords[i*3+1]) max->y = m_coords[i*3+1];
            if (max->z < m_coords[i*3+2]) max->z = m_coords[i*3+2];
        }
        LOGOUT(FILTER_LOG, (char*)"|||+  max -> %8.2f, %8.2f, %8.2f\n",max->x,max->y,max->z);
        LOGOUT(FILTER_LOG, (char*)"|||+  min -> %8.2f, %8.2f, %8.2f\n",min->x,min->y,min->z);
    }
    //Cell information
    // num. elemen
    total_nelements = info->m_nelements;
    nelements = subvol_nelems;
    info->m_nelements = nelements;

    // element num.
    for(i=0;i<subvol_nelems;i++){
        info->m_elementids[i] = i+1 ;
    }

    // material num.
    count = 0;
    int *materials = (int*)malloc(sizeof(int)*subvol_nelems);
    memset(materials, 0x00, sizeof(int)*subvol_nelems);
    for(i=0;i<total_nelements;i++){
        if(elem_type==info->m_elemtypes[i]) {
            material_id = info->m_materials[i];
            materials[count++] = material_id;
        }
    }
    memcpy(info->m_materials, materials, sizeof(int)*subvol_nelems);
    free(materials);

    info->m_element_type = elem_type ;
    int nelem_com = 0;
    if( elem_type == 2 ) nelem_com = 3;
    else if( elem_type == 3 )        nelem_com = 4;
    else if( elem_type == 4 )        nelem_com = 4;
    else if( elem_type == 5 )        nelem_com = 5;
    else if( elem_type == 6 )        nelem_com = 6;
    else if( elem_type == 7 )        nelem_com = 8;
    else if( elem_type == 9 )        nelem_com = 6;
    else if( elem_type == 10)        nelem_com = 8;
    else if( elem_type == 11)        nelem_com = 10;
    else if( elem_type == 14)        nelem_com = 20;
    else {
        LOGOUT(FILTER_ERR, (char*)"Unkown element type : %d\n",info->m_element_type);
        return RTC_NG;
    }
    info->m_elemcoms = nelem_com;
    info->m_connectsize = 0;
    for(i=0;i<subvol_nelems;i++) {
        info->m_connectsize+=info->m_elementsize[elem_type];
    }

    // connect
    ivalue = (int *)malloc(sizeof(int) * 20 );
    size = 0;
    j = 0;
    count = 0;
    int *connections = (int *)malloc(sizeof(int) * info->m_connectsize );
    memset(connections, 0x00, sizeof(int) * info->m_connectsize);
    for(i=0;i<total_nelements;i++){
        size = info->m_elementsize[(int)info->m_elemtypes[i]];
        if(elem_type==info->m_elemtypes[i]) {
            k = 0;
            while(k<size){
                org_id = info->m_connections[j+k];
                new_id = info->m_nodes_trans[org_id-1];
                ivalue[k] = new_id - 1 + 1;
                k++;
            }
            for (n=0; n<size; n++) {
                connections[count++] = ivalue[n];
            }
        }
        j+=size;
    }
    memcpy(info->m_connections, connections, sizeof(int) * info->m_connectsize);
    free(connections);
    free(ivalue);
    ivalue = NULL;


    //Data information

    // num. component
    ncomponents = info->m_ncomponents;
    info->m_nnodecomponents = ncomponents;
/*************
    // value
    value = (float *)malloc(sizeof(float) * info->m_nkinds*subvol_nnodes);
    memset(value, 0x00, sizeof(float) * info->m_nkinds*subvol_nnodes);
    count = 0;
    for(i=0;i<subvol_nnodes;i++){
        id = info->m_subvolume_nodes[rank_nodes+i];
        j = id*info->m_nkinds;
        k = 0;
        while(k<info->m_nkinds){
            value[count++] = info->m_values[j+k];
            k++;
        }
    }
    memcpy(info->m_values, value, sizeof(float) * info->m_nkinds*subvol_nnodes);
    free(value);
    value = NULL;
************************/
    // element type
    for(i=0;i<subvol_nelems;i++){
        info->m_elemtypes[i] = elem_type ;
    }

    state = RTC_OK;
    if(NULL!=ifs) fclose(ifs);
    return(state);
}

} /* namespace filter */
} /* namespace pbvr */
