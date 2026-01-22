#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <sys/stat.h>
#include "../filter_log.h"
#include "../filter_convert.h"
#ifdef _OPENMP
#include "omp.h"
#endif

/*****************************************************************************/
/*
 * ＵＣＤバイナリファイル全読込み
 *
 */
int read_ucdbin_geom( void )
{
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

    bsize = Fparam.blocksize;
    if(strlen(Fparam.ucd_inp)!=0&&m_multi_element==0){
        sprintf(filename,"%s",m_ucdbinfile[0]);
    }else{
        sprintf(buff,Fparam.format,Fparam.start_step);
        sprintf(filename,"%s/%s%s%s",Fparam.in_dir,Fparam.in_prefix,buff,Fparam.in_suffix);
    }
    /* オープン */
    ifs = fopen(filename, "rb");
    if(NULL==ifs){
        filter_log(FILTER_ERR, "can not open %s .\n", filename);
        goto garbage;
    }
    // check first byte size.
    fread(&header, 4, 1, ifs);
    if(ENDIAN) header = ConvertEndianI(header);
    filter_log(FILTER_LOG,"|||| headder           : %10d\n",  header           );
    if(header==7){
        headfoot = (char) 1;
    }else{
        headfoot = (char) 0;
    }
    m_headfoot = headfoot;
    rewind(ifs);
    header = footer = 0;
    filter_log(FILTER_LOG,"|||| filename          : %s\n",    filename         );
    filter_log(FILTER_LOG,"|||| m_headfoot        : %10d\n",  m_headfoot       );

    // keyword
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( keyword, 7, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );

    // version
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &version, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(ENDIAN) version = ConvertEndianF(version);

    // title
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( m_title, 70, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );

    // step num.
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &stepn, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(ENDIAN) stepn = ConvertEndianI(stepn);
    m_stepn = stepn;

    // step time 
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &stept, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(ENDIAN) stept = ConvertEndianF(stept);
    m_stept = stept;
    filter_log(FILTER_LOG,"|||| keyword           : %s\n",    keyword          );
    filter_log(FILTER_LOG,"|||| version           : %10.3f\n",version          );
    /*filter_log(FILTER_LOG,"|||| m_title           : %s\n",    m_title           ); */
    filter_log(FILTER_LOG,"|||| step_no           : %10d\n",  stepn            );
    filter_log(FILTER_LOG,"|||| step_time         : %10.3f\n",stept            );

    //Node information

    // num. nodes
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &nnodes, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(ENDIAN) nnodes = ConvertEndianI(nnodes);
    m_nnodes = nnodes;

    // description type should be 1.
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &desc_type, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(ENDIAN) desc_type = ConvertEndianI(desc_type);
    m_desctype = desc_type;
    filter_log(FILTER_LOG,"|||| m_nnodes          : %10d\n",   m_nnodes         );
    filter_log(FILTER_LOG,"|||| m_desctype        : %10d\n",   m_desctype       );

    // coordinates
    m_coords = (float *)malloc(sizeof(float)*(m_nnodes)*3) ;
    if(NULL==m_coords){
        filter_log(FILTER_ERR, "can not allocate memory(coords).\n");
        goto garbage;
    }
    m_ids = (int *)malloc(sizeof(int)*(m_nnodes)) ;
    if(NULL==m_ids){
        filter_log(FILTER_ERR, "can not allocate memory(ids).\n");
        goto garbage;
    }

    if(m_desctype==1){
        i3 = 0;
        for(i=0;i<m_nnodes;i++,i3+= 3){
            if(headfoot) fread( &header, 4, 1, ifs );
            fread( &id, 4, 1, ifs );
            fread( coord, 4, 3, ifs );
            if(headfoot) fread( &footer, 4, 1, ifs );
            if(ENDIAN){
                id       = ConvertEndianI(id);
                coord[0] = ConvertEndianF(coord[0]);
                coord[1] = ConvertEndianF(coord[1]);
                coord[2] = ConvertEndianF(coord[2]);
            }
            m_ids[i] = id;
            m_coords[ i3     ] = coord[0];
            m_coords[ i3 + 1 ] = coord[1];
            m_coords[ i3 + 2 ] = coord[2];
        }
    }else{
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( m_ids, 4, m_nnodes, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        /* X corrdinate */
        if(headfoot) fread( &header, 4, 1, ifs );
        i3 = 0;
        for(i=0;i<m_nnodes;i++,i3+= 3){
            fread( &m_coords[i3], 4, 1, ifs );
        }
        if(headfoot) fread( &footer, 4, 1, ifs );
        /* Y corrdinate */
        if(headfoot) fread( &header, 4, 1, ifs );
        i3 = 1;
        for(i=0;i<m_nnodes;i++,i3+= 3){
            fread( &m_coords[i3], 4, 1, ifs );
        }
        if(headfoot) fread( &footer, 4, 1, ifs );
        /* Z corrdinate */
        if(headfoot) fread( &header, 4, 1, ifs );
        i3 = 2;
        for(i=0;i<m_nnodes;i++,i3+= 3){
            fread( &m_coords[i3], 4, 1, ifs );
        }
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(ENDIAN){
            for(i=0;i<m_nnodes;i++){
                m_ids[i] = ConvertEndianF(m_ids[i]);
                m_coords[i*3  ] = ConvertEndianF(m_coords[i*3  ]);
                m_coords[i*3+1] = ConvertEndianF(m_coords[i*3+1]);
                m_coords[i*3+2] = ConvertEndianF(m_coords[i*3+2]);
            }
        }
    }

    for(i=0;i<m_nnodes;i++){
        if(m_ids[i] != (i+1)){
            Fparam.nodesearch = (char)1;
            filter_log(FILTER_LOG,"     mids[%10d]        : %d\n",i,m_ids[i]       );
            filter_log(FILTER_LOG,"     Fparam.nodesearch : %d\n",Fparam.nodesearch);
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
    for(i=0;i<m_nnodes;i++){
        if (min_x[myth] > m_coords[i*3  ]) min_x[myth] = m_coords[i*3  ];
        if (min_y[myth] > m_coords[i*3+1]) min_y[myth] = m_coords[i*3+1];
        if (min_z[myth] > m_coords[i*3+2]) min_z[myth] = m_coords[i*3+2];
  
        if (max_x[myth] < m_coords[i*3  ]) max_x[myth] = m_coords[i*3  ];
        if (max_y[myth] < m_coords[i*3+1]) max_y[myth] = m_coords[i*3+1];
        if (max_z[myth] < m_coords[i*3+2]) max_z[myth] = m_coords[i*3+2];
    }
#pragma omp master
   {
    min.x = min_x[0];
    min.y = min_y[0];
    min.z = min_z[0];
    max.x = max_x[0];
    max.y = max_y[0];
    max.z = max_z[0];
    for(i=1;i<numth;i++){
        if (min.x > min_x[i]) min.x = min_x[i];
        if (min.y > min_y[i]) min.y = min_y[i];
        if (min.z > min_z[i]) min.z = min_z[i];
  
        if (max.x < max_x[i]) max.x = max_x[i];
        if (max.y < max_y[i]) max.y = max_y[i];
        if (max.z < max_z[i]) max.z = max_z[i];
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
    min.x = max.x = m_coords[0];
    min.y = max.y = m_coords[1];
    min.z = max.z = m_coords[2];
    for(i=1; i<m_nnodes; i++){
        if (min.x > m_coords[i*3  ]) min.x = m_coords[i*3  ];
        if (min.y > m_coords[i*3+1]) min.y = m_coords[i*3+1];
        if (min.z > m_coords[i*3+2]) min.z = m_coords[i*3+2];
  
        if (max.x < m_coords[i*3  ]) max.x = m_coords[i*3  ];
        if (max.y < m_coords[i*3+1]) max.y = m_coords[i*3+1];
        if (max.z < m_coords[i*3+2]) max.z = m_coords[i*3+2];
    }
#endif
    filter_log(FILTER_LOG,"|||| max -> %8.2f, %8.2f, %8.2f\n",max.x,max.y,max.z);
    filter_log(FILTER_LOG,"|||| min -> %8.2f, %8.2f, %8.2f\n",min.x,min.y,min.z);

    //Cell information

    // num. element
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &nelements, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(ENDIAN) nelements = ConvertEndianI(nelements);
    m_nelements = nelements;
    filter_log(FILTER_LOG,"|||| m_nelements       : %10d\n",   m_nelements      );

    // element num.
    if(headfoot) fread( &header, 4, 1, ifs );
    m_elementids = (int *)malloc(sizeof(int) * nelements );
#ifdef BLOCK_READ
    div = nelements / bsize;
    mod = nelements % bsize;
    for(i=0;i<div;i++){
        fread( &m_elementids[bsize*i], 4, bsize , ifs );
    }
    if(mod>0){
        fread( &m_elementids[bsize*div], 4, mod , ifs );
    }
#else
    fread( m_elementids, 4, nelements , ifs );
#endif
    if(ENDIAN){
        for(i=0;i<nelements;i++){
            m_elementids[i] = ConvertEndianI(m_elementids[i]);
        }
    }
    filter_log(FILTER_LOG,"|||| element id read         \n"                     );
    if(headfoot) fread( &footer, 4, 1, ifs );

    // material num.
    if(headfoot) fread( &header, 4, 1, ifs );
    m_materials = (int *)malloc(sizeof(int) * nelements );
#ifdef BLOCK_READ
    div = nelements / bsize;
    mod = nelements % bsize;
    for(i=0;i<div;i++){
        fread( &m_materials[bsize*i], 4, bsize , ifs );
    }
    if(mod>0){
        fread( &m_materials[bsize*div], 4, mod , ifs );
    }
#else
    fread( m_materials, 4, nelements , ifs );
#endif
    if(ENDIAN){
        for(i=0;i<nelements;i++){
            m_materials[i] = ConvertEndianI(m_materials[i]);
        }
    }
    filter_log(FILTER_LOG,"|||| element material read   \n"                     );
    if(headfoot) fread( &footer, 4, 1, ifs );

    // element type
    m_elemtypes = (char *)malloc(sizeof(char) * nelements );
    if(NULL==m_elemtypes){
        filter_log(FILTER_ERR, "can not allocate memory(elemtypes).\n");
        goto garbage;
    }
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( m_elemtypes, 1, nelements, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    element_type = m_elemtypes[0];

    if( m_elemtypes[0] == 2 ){
        nelem_com = 3;
    }else if( m_elemtypes[0] == 3 ){
        nelem_com = 4;
    }else if( m_elemtypes[0] == 4 ){
        nelem_com = 4;
    }else if( m_elemtypes[0] == 5 ){
        nelem_com = 5;
    }else if( m_elemtypes[0] == 6 ){
        nelem_com = 6;
    }else if( m_elemtypes[0] == 7 ){
        nelem_com = 8;
    }else if( m_elemtypes[0] == 9 ){
        nelem_com = 6;
    }else if( m_elemtypes[0] == 10){
        nelem_com = 8;
    }else if( m_elemtypes[0] == 11){
        nelem_com = 10;
    }else if( m_elemtypes[0] == 14){
        nelem_com = 20;
    }else{
        filter_log(FILTER_ERR, "Unkown element type : %d\n",m_element_type);
        goto garbage;
    }
    m_elemcoms = nelem_com;
    m_element_type = m_elemtypes[0];

    filter_log(FILTER_LOG,"|||| element_type      : %10d\n",   element_type     );
    filter_log(FILTER_LOG,"|||| m_element_type    : %10d\n",   m_element_type   );
    filter_log(FILTER_LOG,"|||| m_elemcoms        : %10d\n",   m_elemcoms       );

    Fparam.mult_element_type = (char)0;
    m_connectsize = 0;
    type = m_elemtypes[0];
    for(i=0;i<m_nelements;i++){
        if(type!=m_elemtypes[i]){
            Fparam.mult_element_type = (char)1;
        }
        if(m_elemtypes[i]<=1  ||
           m_elemtypes[i]==8  ||
           m_elemtypes[i]==12 ||
           m_elemtypes[i]==13 ||
           m_elemtypes[i]>=15){
            filter_log(FILTER_ERR, "Unkown element type : %d\n",m_elemtypes[i]);
            goto garbage;
        }
        m_connectsize+=m_elementsize[(int)m_elemtypes[i]];
        m_types[(int)m_elemtypes[i]]++;
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

//  m_connections = (int *)malloc(sizeof(int)*nelem_com*m_nelements );
    m_connections = (int *)malloc(sizeof(int)*m_connectsize );
    if(NULL==m_connections){
        filter_log(FILTER_ERR, "can not allocate memory(connections).\n");
        goto garbage;
    }

    if(headfoot) fread( &header, 4, 1, ifs );
//  fread( m_connections, 4, nelem_com * m_nelements , ifs );
#ifdef BLOCK_READ
    div = m_connectsize / bsize;
    mod = m_connectsize % bsize;
    for(i = 0; i < div; i++ ) {
        fread( &m_connections[bsize*i], 4, bsize , ifs );
    }
    if(mod>0){
        fread( &m_connections[bsize*div], 4, mod , ifs );
    }
#else
    fread( m_connections, 4, m_connectsize , ifs );
#endif
    if(headfoot) fread( &footer, 4, 1, ifs );

    /// adjust connection by minus 1 ///
    if(ENDIAN){
#ifdef _OPENMP
        #pragma omp parallel for default(shared) private(i)
#endif
        for(i = 0; i < m_connectsize; i++ ) {
            m_connections[i] = ConvertEndianI(m_connections[i]);
        }
    }
    if (Fparam.nodesearch ==1) {
        int max_id = 0;
        int *conv_ids = NULL;

        for(i=0;i<m_nnodes;i++){
            if(max_id<m_ids[i]){
                max_id = m_ids[i];
            }
        }

        conv_ids = (int *)malloc(sizeof(float)*(max_id+1));

        for(i=0;i<m_nnodes;i++){
            conv_ids[m_ids[i]] = i+1;
        }
   
        for(i = 0; i < m_connectsize; i++ ) {
            m_connections[i] = conv_ids[m_connections[i]];
        }

        free(conv_ids);
   
        Fparam.nodesearch = 0;
    }

    //Data information

    // num. component of nodes
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &ncomponents, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    m_nnodecomponents = ncomponents;
    filter_log(FILTER_LOG,"|||| m_nnodecomponents : %10d\n",   m_nnodecomponents);

    nkind = 0;
    if(m_nnodecomponents>0){
        // data type
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &data_type, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(ENDIAN) data_type = ConvertEndianI(data_type);
        m_datatype = data_type;
        filter_log(FILTER_LOG,"||||  m_datatype        : %10d\n",   m_datatype      );
        mn_name = (char *)malloc(sizeof(char)*m_nnodecomponents*COMPONENT_LEN );
        if(NULL==mn_name){
            filter_log(FILTER_ERR, "can not allocate memory(mn_name).\n");
            goto garbage;
        }
        pmn_name = (char**)malloc(sizeof(char*)*m_nnodecomponents);
        for(i=0;i<m_nnodecomponents;i++){
            pmn_name[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }

        mn_unit = (char *)malloc(sizeof(char)*m_nnodecomponents*COMPONENT_LEN );
        if(NULL==mn_unit){
            filter_log(FILTER_ERR, "can not allocate memory(mn_unit).\n");
            goto garbage;
        }
        pmn_unit = (char**)malloc(sizeof(char*)*m_nnodecomponents);
        for(i=0;i<m_nnodecomponents;i++){
            pmn_unit[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }

        mn_veclen = (int *)malloc(sizeof(int)*m_nnodecomponents );
        if(NULL==mn_veclen){
            filter_log(FILTER_ERR, "can not allocate memory(mn_veclen).\n");
            goto garbage;
        }
        mn_nullflag = (int *)malloc(sizeof(int)*m_nnodecomponents );
        if(NULL==mn_nullflag){
            filter_log(FILTER_ERR, "can not allocate memory(mn_nullflag).\n");
            goto garbage;
        }
        mn_nulldata = (float *)malloc(sizeof(float)*m_nnodecomponents );
        if(NULL==mn_nulldata){
            filter_log(FILTER_ERR, "can not allocate memory(mn_nulldata).\n");
            goto garbage;
        }

        if(m_datatype==1){
            // data header
            for(i=0;i<m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, 1, COMPONENT_LEN, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
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
                filter_log(FILTER_LOG,"||||  i  : %2d\n",i);
                filter_log(FILTER_LOG,"||||  component_name    : %s\n",     component_name  );
                filter_log(FILTER_LOG,"||||  unit              : %s\n",     unit            );
                filter_log(FILTER_LOG,"||||  veclen            : %10d\n",   veclen          );
                filter_log(FILTER_LOG,"||||  null_flag         : %10d\n",   null_flag       );
                filter_log(FILTER_LOG,"||||  null_data         : %10.5f\n", null_data       );
                nkind += veclen;
            }
            size = 4*nkind*m_nnodes;
            if(headfoot) size += (4+4)*m_nnodes;
            fseek( ifs, size, SEEK_CUR );
        }else if(m_datatype==2){
            // data header
            for(i=0;i<m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
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
            size = sizeof(float)*m_nnodes*nkind;
            if(headfoot) size += (4+4)*nkind;
            fseek( ifs, size, SEEK_CUR );
        } else if(m_datatype==3){
            for(i=0;i<m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                mn_veclen[i] = veclen;
                mn_nullflag[i] = 1;
                mn_nulldata[i] = -99999999.f;
//              strcpy(pmn_name[i],component_name);
//              strcpy(pmn_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pmn_name[i][m] = component_name[m];
                    pmn_unit[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            for(i=0;i<m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nnodes, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    nnodes = ConvertEndianI(nnodes);
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nnodes*(m_veclens[i]+1), SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
            }
        } else if(m_datatype==4){
            for(i=0;i<m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                mn_veclen[i] = veclen;
                mn_nullflag[i] = 1;
                mn_nulldata[i] = -99999999.f;
//              strcpy(pmn_name[i],component_name);
//              strcpy(pmn_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pmn_name[i][m] = component_name[m];
                    pmn_unit[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            for(i=0;i<m_nnodecomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nnodes, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    nnodes = ConvertEndianI(nnodes);
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nnodes, SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nnodes*m_veclens[i], SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
            }
        }
    }
    m_nnodekinds = nkind;
    filter_log(FILTER_LOG,"|||| m_nnodekinds      : %10d\n",   m_nnodekinds     );

    // num. component of elements
    if(headfoot) fread( &header, 4, 1, ifs );
    size = fread( &ncomponents, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    m_nelemcomponents = ncomponents;

    filter_log(FILTER_LOG,"|||| m_nelemcomponents : %10d\n",   m_nelemcomponents);
    nkind = 0;

    if(m_nelemcomponents>0){
        // data type
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &data_type, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(ENDIAN) data_type = ConvertEndianI(data_type);
        m_datatype = data_type;
        filter_log(FILTER_LOG,"||||  m_datatype        : %10d\n",   m_datatype       );

        me_name = (char *)malloc(sizeof(char)*m_nelemcomponents*COMPONENT_LEN );
        if(NULL==me_name){
            filter_log(FILTER_ERR, "can not allocate memory(me_name).\n");
            goto garbage;
        }
        pme_name = (char**)malloc(sizeof(char*)*m_nelemcomponents);
        for(i=0;i<m_nelemcomponents;i++){
            pme_name[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }

        me_unit = (char *)malloc(sizeof(char)*m_nelemcomponents*COMPONENT_LEN );
        if(NULL==me_unit){
            filter_log(FILTER_ERR, "can not allocate memory(me_unit).\n");
            goto garbage;
        }
        pme_unit = (char**)malloc(sizeof(char*)*m_nelemcomponents);
        for(i=0;i<m_nelemcomponents;i++){
            pme_unit[i] = (char*)malloc(sizeof(char)*COMPONENT_LEN);
        }

        me_veclen = (int *)malloc(sizeof(int)*m_nelemcomponents );
        if(NULL==me_veclen){
            filter_log(FILTER_ERR, "can not allocate memory(me_veclen).\n");
            goto garbage;
        }
        me_nullflag = (int *)malloc(sizeof(int)*m_nelemcomponents );
        if(NULL==me_nullflag){
            filter_log(FILTER_ERR, "can not allocate memory(me_nullflag).\n");
            goto garbage;
        }
        me_nulldata = (float *)malloc(sizeof(float)*m_nelemcomponents );
        if(NULL==me_nulldata){
            filter_log(FILTER_ERR, "can not allocate memory(me_nulldata).\n");
            goto garbage;
        }

        if(m_datatype==1){
            // data header
            for(i=0;i<m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
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
                filter_log(FILTER_LOG,"||||  i  : %2d\n",i);
                filter_log(FILTER_LOG,"||||  component_name    : %s\n",     component_name  );
                filter_log(FILTER_LOG,"||||  unit              : %s\n",     unit            );
                filter_log(FILTER_LOG,"||||  veclen            : %10d\n",   veclen          );
                filter_log(FILTER_LOG,"||||  null_flag         : %10d\n",   null_flag       );
                filter_log(FILTER_LOG,"||||  null_data         : %10.5f\n", null_data       );
                nkind += veclen;
            }
            size = sizeof(float)*m_nelements*nkind;
            if(headfoot) size += (4+4)*m_nelements;
            fseek( ifs, size, SEEK_CUR );
        }else if(m_datatype==2){
            // data header
            for(i=0;i<m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                fread( &null_flag, 4, 1, ifs );
                fread( &null_data, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
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
            size = sizeof(float)*m_nelements*nkind;
            if(headfoot) size += (4+4)*nkind;
            fseek( ifs, size, SEEK_CUR );
        } else if(m_datatype==3){
            for(i=0;i<m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                me_veclen[i] = veclen;
                me_nullflag[i] = 1;
                me_nulldata[i] = -99999999.f;
//              strcpy(pme_name[i],component_name);
//              strcpy(pme_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pme_name[i][m] = component_name[m];
                    pme_unit[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            for(i=0;i<m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nelements, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    nelements = ConvertEndianI(nelements);
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nelements*(m_veclens[i]+1), SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
            }
        } else if(m_datatype==4){
            for(i=0;i<m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                me_veclen[i] = veclen;
                me_nullflag[i] = 1;
                me_nulldata[i] = -99999999.f;
//              strcpy(pme_name[i],component_name);
//              strcpy(pme_unit[i],unit          );
                for(m=0;m<COMPONENT_LEN;m++){
                    pme_name[i][m] = component_name[m];
                    pme_unit[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            for(i=0;i<m_nelemcomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nelements, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    nelements = ConvertEndianI(nelements);
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nelements, SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(headfoot) fread( &header, 4, 1, ifs );
                fseek( ifs, nelements*m_veclens[i], SEEK_CUR );
                if(headfoot) fread( &footer, 4, 1, ifs );
            }
        }
    }
    m_nelemkinds = nkind;
    m_nkinds = m_nnodekinds + m_nelemkinds;
    m_ncomponents = m_nnodecomponents + m_nelemcomponents;
    filter_log(FILTER_LOG,"|||| m_nelemkinds      : %10d\n",   m_nelemkinds     );
    filter_log(FILTER_LOG,"|||| m_ncomponents     : %10d\n",   m_ncomponents    );
    filter_log(FILTER_LOG,"|||| m_nkinds          : %10d\n",   m_nkinds         );

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
//              strcpy(pm_names[i],pmn_name[i]);
//              strcpy(pm_units[i],pmn_unit[i]);
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[i][m] = pmn_name[i][m];
                    pm_units[i][m] = pmn_unit[i][m];
                }
                m_veclens[i] = mn_veclen[i];
                for(j=0;j<m_veclens[i];j++){
                    m_nullflags[nkind] = mn_nullflag[i];
                    m_nulldatas[nkind] = mn_nulldata[i];
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
            if(mn_nullflag!=NULL) free(mn_nullflag);
            if(mn_nulldata!=NULL) free(mn_nulldata);
               mn_name=NULL;
               mn_unit=NULL;
               mn_veclen=NULL;
               mn_nullflag=NULL;
               mn_nulldata=NULL;
        }
        if(m_nelemcomponents>0){
            nkind = 0;
            for(i=0;i<m_nelemcomponents;i++ ){
//              strcpy(pm_names[m_nnodecomponents+i],pme_name[i]);
//              strcpy(pm_units[m_nnodecomponents+i],pme_unit[i]);
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[m_nnodecomponents+i][m] = pme_name[i][m];
                    pm_units[m_nnodecomponents+i][m] = pme_unit[i][m];
                }
                m_veclens[m_nnodecomponents+i] = me_veclen[i];
                for(j=0;j<me_veclen[i];j++){
                    m_nullflags[m_nnodecomponents+nkind] = me_nullflag[i];
                    m_nulldatas[m_nnodecomponents+nkind] = me_nulldata[i];
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
    m_filesize = ftell(ifs);
  /*fpos_t  pos;
    fgetpos(ifs,&pos);
    m_filesize = pos.__pos;*/
    filter_log(FILTER_LOG,"|||| m_filesize        : %10ld\n",   m_filesize       );

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
int read_ucdbin_data( int step_loop )
{
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

    if(strlen(Fparam.ucd_inp)!=0&&m_multi_element==0){
        sprintf(filename,"%s",m_ucdbinfile[step_loop-1]);
    }else{
        sprintf(buff,Fparam.format,step_loop);
        sprintf(filename,"%s/%s%s%s",Fparam.in_dir,Fparam.in_prefix,buff,Fparam.in_suffix);
    }
    /* オープン */
    ifs = fopen(filename, "rb");
    if(NULL==ifs){
        filter_log(FILTER_ERR, "can not open %s.\n", filename);
        goto garbage;
    }
    //Head information

    // check first byte size.
    fread(&header, 4, 1, ifs);
    if(ENDIAN) header = ConvertEndianI(header);
    filter_log(FILTER_LOG,"|||+ header            : %10d\n",  header           );
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
    if(f_size!=m_filesize){
        // num. nodes
        geom_size += 4;
        if(headfoot) geom_size+=8;
        // description type
        geom_size += 4;
        if(headfoot) geom_size+=8;
        // id & coordinates
        if(m_desctype==1){
            geom_size += (4+4+4+4)*m_nnodes;
            if(headfoot) geom_size+=8*m_nnodes;
        }else{
            geom_size += (4+4+4+4)*m_nnodes;
            if(headfoot) geom_size+=8*4;
        }
        //Cell information
        // num. element
        geom_size += 4;
        if(headfoot) geom_size+=8;
        // element num.
        geom_size += (4*m_nelements);
        if(headfoot) geom_size+=8;
        // material num.
        geom_size += (4*m_nelements);
        if(headfoot) geom_size+=8;
        // element type
        geom_size += 1*m_nelements;
        if(headfoot) geom_size+=8;
        // connection
        geom_size += 4*m_connectsize;
        if(headfoot) geom_size+=8;
    }

    filter_log(FILTER_LOG,"|||+ filename          : %s\n",    filename         );
    filter_log(FILTER_LOG,"|||+ m_headfoot        : %10d\n",  m_headfoot       );
    filter_log(FILTER_LOG,"|||+ headfoot          : %10d\n",  headfoot         );
    filter_log(FILTER_LOG,"|||+ f_size            : %10ld\n",  f_size           );
    filter_log(FILTER_LOG,"|||+ m_filesize        : %10ld\n",  m_filesize       );
    filter_log(FILTER_LOG,"|||+ geom_size         : %10ld\n",  geom_size        );

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
    if(ENDIAN) version = ConvertEndianF(version);

    // title
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( m_title, 70, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );

    // step num.
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &stepn, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(ENDIAN) stepn = ConvertEndianI(stepn);
    m_stepn = stepn;

    // step time
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &stept, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(ENDIAN) stept = ConvertEndianF(stept);
    m_stept = stept;
    filter_log(FILTER_LOG,"|||+ keyword           : %s\n",    keyword          );
    filter_log(FILTER_LOG,"|||+ version           : %10.3f\n",version          );
    /*filter_log(FILTER_LOG,"|||+ m_title           : %s\n",    m_title          );*/
    filter_log(FILTER_LOG,"|||+ step_no           : %10d\n",  stepn            );
    filter_log(FILTER_LOG,"|||+ step_time         : %10.3f\n",stept            );

    if(Fparam.start_step==step_loop||m_filesize==f_size||m_headfoot!=headfoot){
#if 1
        // num. nodes
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &nnodes, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(ENDIAN) nnodes = ConvertEndianI(nnodes);
        // description type should be 1.
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &desc_type, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(ENDIAN) desc_type = ConvertEndianI(desc_type);
        filter_log(FILTER_LOG,"|||+ nnodes            : %10d\n",   nnodes           );
        filter_log(FILTER_LOG,"|||+ desc_type         : %10d\n",   desc_type        );
        if(m_desctype==1){
            i3 = 0;
            for(i=0;i<m_nnodes;i++,i3+= 3){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &id, 4, 1, ifs );
                fread( coord, 4, 3, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    id       = ConvertEndianI(id);
                    coord[0] = ConvertEndianF(coord[0]);
                    coord[1] = ConvertEndianF(coord[1]);
                    coord[2] = ConvertEndianF(coord[2]);
                }
                m_ids[i] = id;
                m_coords[ i3     ] = coord[0];
                m_coords[ i3 + 1 ] = coord[1];
                m_coords[ i3 + 2 ] = coord[2];
            }
        }else{
            if(headfoot) fread( &header, 4, 1, ifs );
            fread( m_ids, 4, m_nnodes, ifs );
            if(headfoot) fread( &footer, 4, 1, ifs );
            /* X corrdinate */
            if(headfoot) fread( &header, 4, 1, ifs );
            i3 = 0;
            for(i=0;i<m_nnodes;i++,i3+= 3){
                fread( &m_coords[i3], 4, 1, ifs );
            }
            if(headfoot) fread( &footer, 4, 1, ifs );
            /* Y corrdinate */
            if(headfoot) fread( &header, 4, 1, ifs );
            i3 = 1;
            for(i=0;i<m_nnodes;i++,i3+= 3){
                fread( &m_coords[i3], 4, 1, ifs );
            }
            if(headfoot) fread( &footer, 4, 1, ifs );
            /* Z corrdinate */
            if(headfoot) fread( &header, 4, 1, ifs );
            i3 = 2;
            for(i=0;i<m_nnodes;i++,i3+= 3){
                fread( &m_coords[i3], 4, 1, ifs );
            }
            if(headfoot) fread( &footer, 4, 1, ifs );
            if(ENDIAN){
                    for(i=0;i<m_nnodes;i++){
                    m_ids[i] = ConvertEndianF(m_ids[i]);
                    m_coords[i*3  ] = ConvertEndianF(m_coords[i*3  ]);
                    m_coords[i*3+1] = ConvertEndianF(m_coords[i*3+1]);
                    m_coords[i*3+2] = ConvertEndianF(m_coords[i*3+2]);
                }
            }
        }
        for(i=0;i<m_nnodes;i++){
            if(m_ids[i] != (i+1)){
                Fparam.nodesearch = (char)1;
                filter_log(FILTER_LOG,"  Fparam.nodesearch : %d\n",Fparam.nodesearch);
                break;
            }
        }
        /* 最大・最小を取得 */
        min.x = max.x = m_coords[0];
        min.y = max.y = m_coords[1];
        min.z = max.z = m_coords[2];
        for(i=1; i<m_nnodes; i++){
            if (min.x > m_coords[i*3  ]) min.x = m_coords[i*3  ];
            if (min.y > m_coords[i*3+1]) min.y = m_coords[i*3+1];
            if (min.z > m_coords[i*3+2]) min.z = m_coords[i*3+2];
    
            if (max.x < m_coords[i*3  ]) max.x = m_coords[i*3  ];
            if (max.y < m_coords[i*3+1]) max.y = m_coords[i*3+1];
            if (max.z < m_coords[i*3+2]) max.z = m_coords[i*3+2];
        }
        filter_log(FILTER_LOG,"|||+  max -> %8.2f, %8.2f, %8.2f\n",max.x,max.y,max.z);
        filter_log(FILTER_LOG,"|||+  min -> %8.2f, %8.2f, %8.2f\n",min.x,min.y,min.z);

#else
        size = 0;
        // num. nodes
        size += 4;
        if(headfoot) size+=8;
        // description type should be 1.
        size += 4;
        if(headfoot) size+=8;
        // id & coordinates
        size += (4+4*3)*m_nnodes;
        if(headfoot) size+=8*m_nnodes;
        fseek( ifs, size, SEEK_CUR );
#endif

        //Cell information
        size = 0;
        // num. element
        size += 4;
        if(headfoot) size+=8;
        // element num.
        size += (4*m_nelements);
        if(headfoot) size+=8;
        // material num.
        size += (4*m_nelements);
        if(headfoot) size+=8;
        // element type
        size += 1*m_nelements;
        if(headfoot) size+=8;
        // connection
        size += 4*m_connectsize;
        if(headfoot) size+=8;
        fseek( ifs, size, SEEK_CUR );
    }

    //Data information

    // num. component of nodes
    if(headfoot) fread( &header, 4, 1, ifs );
    fread( &ncomponents, 4, 1, ifs );
    if(headfoot) fread( &footer, 4, 1, ifs );
    if(ENDIAN) header      = ConvertEndianI(header     );
    if(ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    if(ENDIAN) footer      = ConvertEndianI(footer     );
    filter_log(FILTER_LOG,"|||+ ncomponents       : %10d\n",   ncomponents      );
    filter_log(FILTER_LOG,"|||+ m_nnodes          : %10d\n",   m_nnodes         );
    filter_log(FILTER_LOG,"|||+ m_nnodekinds      : %10d\n",   m_nnodekinds     );

    if(ncomponents>0){
        // data type
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &datatype, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(ENDIAN) header   = ConvertEndianI(header     );
        if(ENDIAN) datatype = ConvertEndianI(datatype);
        if(ENDIAN) footer   = ConvertEndianI(footer     );
        filter_log(FILTER_LOG,"|||+ datatype          : %10d\n",   datatype         );
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
                if(ENDIAN){
                    header    = ConvertEndianI(header);
                    footer    = ConvertEndianI(footer);
                    veclen    = ConvertEndianI(veclen);
                    null_flag = ConvertEndianI(null_flag);
                    null_data = ConvertEndianF(null_data);
                }
                m_veclens[i] = veclen;
                for(j=0;j<veclen;j++){
                    m_nullflags[nkind+j] = null_flag;
                    m_nulldatas[nkind+j] = null_data;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[i][m] = component_name[m];
                    pm_units[i][m] = unit[m]          ;
                }
                nkind += veclen;
                filter_log(FILTER_LOG,"||||  component_name    : %s\n",     component_name  );
            }
            value = (float *)malloc(sizeof(float) * nkind );
            if(NULL==value){
                filter_log(FILTER_ERR, "can not allocate memory(values).\n");
                goto garbage;
            }
    
            for(i=0;i<m_nnodes;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( value, 4, nkind, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    for(j=0;j<nkind;j++){
                        value[j] = ConvertEndianF(value[j]);
                    }
                }
                for(j=0;j<nkind;j++){
                    m_values[i*m_nkinds+j]=value[j];
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
                if(ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                m_veclens[i] = veclen;
                for(j=0;j<veclen;j++){
                    m_nullflags[nkind+j] = null_flag;
                    m_nulldatas[nkind+j] = null_data;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[i][m] = component_name[m];
                    pm_units[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }

            value = (float *)malloc(sizeof(float) * m_nnodes );
            if(NULL==value){
                filter_log(FILTER_ERR, "can not allocate memory(values).\n");
                goto garbage;
            }

            for(i=0;i<m_nkinds;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( value, 4, m_nnodes, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    for(j=0;j<m_nnodes;j++){
                        value[j] = ConvertEndianF(value[j]);
                    }
                }
                for(j=0;j<m_nnodes;j++){
                    m_values[j*m_nkinds+i]=value[j];
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
                if(ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                m_veclens[i] = veclen;
                for(j=0;j<veclen;j++){
                    m_nullflags[nkind+j] = 1;
                    m_nulldatas[nkind+j] = -99999999.f;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[i][m] = component_name[m];
                    pm_units[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            nkind = 0;
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nnodes, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    nnodes = ConvertEndianI(nnodes);
                }
                ids   = (int   *)malloc(sizeof(float) * nnodes );
                value = (float *)malloc(sizeof(float) * m_veclens[i] );
                if(NULL==value||NULL==ids){
                    filter_log(FILTER_ERR, "can not allocate memory(values).\n");
                    goto garbage;
                }
                for(j=0;j<m_nnodes;j++){
                    for(k=0;k<m_veclens[i];k++){
                        m_values[j*m_nkinds+nkind+k] = m_nulldatas[nkind+k];
                    }
                }
                for(j=0;j<nnodes;j++){
                    if(headfoot) fread( &header, 4, 1, ifs );
                    fread( &ids[j], 4, 1, ifs );
                    fread( value  , 4, m_veclens[i], ifs );
                    if(headfoot) fread( &footer, 4, 1, ifs );
                    if(ENDIAN){
                        ids[j]   = ConvertEndianI(ids[j]);
                        for(k=0;k<m_veclens[i];k++){
                            value[k] = ConvertEndianF(value[k]);
                        }
                    }
                    for(k=0;k<m_veclens[i];k++){
                        m_values[(ids[j]-1)*m_nkinds+nkind+k] = value[k];
                    }
                }
                nkind += m_veclens[i];
                free(value);
                value = NULL;
                free(ids);
                ids = NULL;
            }
        }else if(m_datatype==4){
            nkind = 0;
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                m_veclens[i] = veclen;
                for(j=0;j<veclen;j++){
                    m_nullflags[nkind+j] = 1;
                    m_nulldatas[nkind+j] = -99999999.f;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[i][m] = component_name[m];
                    pm_units[i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            nkind = 0;
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nnodes, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    nnodes = ConvertEndianI(nnodes);
                }

                ids   = (int   *)malloc(sizeof(float) * nnodes );
                value = (float *)malloc(sizeof(float) * nnodes );
                if(NULL==value||NULL==ids){
                    filter_log(FILTER_ERR, "can not allocate memory(values).\n");
                    goto garbage;
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( ids, 4, nnodes, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    for(j=0;j<nnodes;j++){
                        ids[j] = ConvertEndianI(ids[j]);
                    }
                }
                for(j=0;j<m_veclens[i];j++){
                    if(headfoot) fread( &header, 4, 1, ifs );
                    fread( value , 4, nnodes, ifs );
                    if(headfoot) fread( &footer, 4, 1, ifs );
                    if(ENDIAN) {
                        for(k=0;k<nnodes;k++){
                            value[k] = ConvertEndianF(value[k]);
                        }
                    }
                    for(k=0;k<m_nnodes;k++){
                        m_values[k*m_nkinds+nkind+j] = m_nulldatas[nkind+j];
                    }
                    for(k=0;k<nnodes;k++){
                        m_values[(ids[k]-1)*m_nkinds+nkind+j] = value[k];
                    }
                }
                nkind += m_veclens[i];
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
    if(ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    filter_log(FILTER_LOG,"|||+ ncomponents       : %10d\n",   ncomponents      );
    filter_log(FILTER_LOG,"|||+ m_nelements       : %10d\n",   m_nelements      );
    filter_log(FILTER_LOG,"|||+ m_nelemkinds      : %10d\n",   m_nelemkinds     );

    if(ncomponents>0){
        // data type
        float    *elem_value = NULL;
        int      *effe_num   = NULL;
        if(headfoot) fread( &header, 4, 1, ifs );
        fread( &datatype, 4, 1, ifs );
        if(headfoot) fread( &footer, 4, 1, ifs );
        if(ENDIAN) datatype = ConvertEndianI(datatype);
        filter_log(FILTER_LOG,"|||+ datatype          : %10d\n",   datatype         );

        elem_value = (float*)malloc(sizeof(float)*m_nelements*m_nelemkinds);
        if(NULL==elem_value){
            filter_log(FILTER_ERR, "can not allocate memory(values).\n");
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
                if(ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                m_veclens[m_nnodecomponents+i] = veclen;
                for(j=0;j<veclen;j++){
                    m_nullflags[m_nnodekinds+nkind+j] = null_flag;
                    m_nulldatas[m_nnodekinds+nkind+j] = null_data;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[m_nnodecomponents+i][m] = component_name[m];
                    pm_units[m_nnodecomponents+i][m] = unit[m]          ;
                }
                nkind += veclen;
            }

            value    = (float *)malloc(sizeof(float)*nkind );
            if(NULL==value){
                filter_log(FILTER_ERR, "can not allocate memory(values).\n");
                goto garbage;
            }

            for(i=0;i<m_nelements;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( value, 4, nkind, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
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
                if(ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                m_veclens[m_nnodecomponents+i] = veclen;
                for(j=0;j<veclen;j++){
                    m_nullflags[m_nnodekinds+nkind+j] = null_flag;
                    m_nulldatas[m_nnodekinds+nkind+j] = null_data;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[m_nnodecomponents+i][m] = component_name[m];
                    pm_units[m_nnodecomponents+i][m] = unit[m]          ;
                }
                nkind += veclen;
            }

            value = (float *)malloc(sizeof(float) * m_nelements );
            if(NULL==value){
                filter_log(FILTER_ERR, "can not allocate memory(values).\n");
                goto garbage;
            }
            for(i=0;i<nkind;i++){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( value, 4, m_nelements, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    for(j=0;j<m_nelements;j++){
                        value[j] = ConvertEndianF(value[j]);
                    }
                }
                for(j=0;j<m_nelements;j++){
                    elem_value[j*m_nelemkinds+i] = value[j];
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
                if(ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                m_veclens[m_nnodecomponents+i] = veclen;
                for(j=0;j<veclen;j++){
                    m_nullflags[m_nnodekinds+nkind+j] = 1;
                    m_nulldatas[m_nnodekinds+nkind+j] = -99999999.f;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[m_nnodecomponents+i][m] = component_name[m];
                    pm_units[m_nnodecomponents+i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            nkind = 0;
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nelements, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    nelements = ConvertEndianI(nelements);
                }
                ids   = (int   *)malloc(sizeof(float) * nelements );
                value = (float *)malloc(sizeof(float) * m_veclens[m_nnodecomponents-1+i] );
                if(NULL==value||NULL==ids){
                    filter_log(FILTER_ERR, "can not allocate memory(values).\n");
                    goto garbage;
                }
                for(j=0;j<m_nelements;j++){
                    for(k=0;k<m_veclens[m_nnodecomponents+i];k++){
                        elem_value[j*m_nelemkinds+nkind+k] = m_nulldatas[m_nnodekinds+nkind+k];
                    }
                }
                for(j=0;j<nelements;j++){
                    if(headfoot) fread( &header, 4, 1, ifs );
                    fread( &ids[j], 4, 1, ifs );
                    fread( value  , 4, m_veclens[m_nnodecomponents+i], ifs );
                    if(headfoot) fread( &footer, 4, 1, ifs );
                    if(ENDIAN){
                        ids[j]   = ConvertEndianI(ids[j]);
                        for(k=0;k<m_veclens[m_nnodecomponents+i];k++){
                            value[k] = ConvertEndianF(value[k]);
                        }
                    }
                    eid=0;
                    for(k=0;k<m_nelements;k++){
                        if(ids[j]==m_elementids[k]){
                            eid = k;
                            break;
                        }
                    }
                    for(k=0;k<m_veclens[m_nnodecomponents+i];k++){
                        elem_value[eid*m_nelemkinds+nkind+k] = value[k];
                    }
                }
                nkind += m_veclens[m_nnodecomponents+i];
                free(value);
                value = NULL;
                free(ids);
                ids = NULL;
            }
        }else if(m_datatype==4){
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( component_name, COMPONENT_LEN, 1, ifs );
                fread( unit, COMPONENT_LEN, 1, ifs );
                fread( &veclen, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    veclen = ConvertEndianI(veclen);
                }
                m_veclens[m_nnodecomponents+i] = veclen;
                for(j=0;j<veclen;j++){
                    m_nullflags[m_nnodekinds+nkind+j] = 1;
                    m_nulldatas[m_nnodekinds+nkind+j] = -99999999.f;
                }
                for(m=0;m<COMPONENT_LEN;m++){
                    pm_names[m_nnodecomponents+i][m] = component_name[m];
                    pm_units[m_nnodecomponents+i][m] = unit[m]          ;
                }
                nkind += veclen;
            }
            nkind = 0;
            for(i=0;i<ncomponents;i++ ){
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( &nelements, 4, 1, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    nelements = ConvertEndianI(nelements);
                }

                ids   = (int   *)malloc(sizeof(float) * nelements );
                value = (float *)malloc(sizeof(float) * nelements );
                if(NULL==value||NULL==ids){
                    filter_log(FILTER_ERR, "can not allocate memory(values).\n");
                    goto garbage;
                }
                if(headfoot) fread( &header, 4, 1, ifs );
                fread( ids, 4, nelements, ifs );
                if(headfoot) fread( &footer, 4, 1, ifs );
                if(ENDIAN){
                    for(j=0;j<nelements;j++){
                        ids[j] = ConvertEndianI(ids[j]);
                    }
                }
                for(j=0;j<m_veclens[m_nnodecomponents+i];j++){
                    if(headfoot) fread( &header, 4, 1, ifs );
                    fread( value , 4, nelements, ifs );
                    if(headfoot) fread( &footer, 4, 1, ifs );
                    if(ENDIAN) {
                        for(k=0;k<nelements;k++){
                            value[k] = ConvertEndianF(value[k]);
                        }
                    }
                    for(k=0;k<m_nelements;k++){
                        elem_value[k*m_nelemkinds+nkind+j] = m_nulldatas[m_nnodekinds+nkind+j];
                    }
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(k,m)
#endif
                    for(k=0;k<nelements;k++){
                        for(m=0;m<m_nelements;m++){
                            if(ids[k]==m_elementids[m]){
                                ids[k] = m+1;
                                break;
                            }
                        }
                    }
                    for(k=0;k<nelements;k++){
                        elem_value[(ids[k]-1)*m_nelemkinds+nkind+j] = value[k];
                    }
                }
                nkind += m_veclens[m_nnodecomponents+i];
                free(value);
                value = NULL;
                free(ids);
                ids = NULL;
            }
        }

        value = (float *)malloc(sizeof(float) * m_nelemkinds);
        effe_num = (int *)malloc(sizeof(int) * m_nelemkinds);
        if(NULL==value||NULL==effe_num){
            filter_log(FILTER_ERR, "can not allocate memory(values).\n");
            goto garbage;
        }
        for(i=0;i<m_nnodes;i++){
            for(k=0;k<m_nelemkinds;k++){
                value[k] = 0.0;
                effe_num[k] = 0;
            }
            for(j=0;j<m_elem_in_node[i];j++){
                eid = m_elem_in_node_array[m_elem_in_node_addr[i]+j] - 1;
                for(k=0;k<m_nelemkinds;k++){
                    if((m_nullflags[m_nnodekinds+k])&&
                       (m_nulldatas[m_nnodekinds+k]==elem_value[eid*m_nelemkinds+k])){
                        ;
                    }else{
                        value[k] = value[k] + elem_value[eid*m_nelemkinds+k];
                        effe_num[k]++ ;
                    }
                }
            }
            for(k=0;k<m_nelemkinds;k++){
                if(effe_num[k]!=0){
                    m_values[i*m_nkinds+m_nnodekinds+k] = value[k] / effe_num[k];
                }else{
                    m_values[i*m_nkinds+m_nnodekinds+k] = m_nulldatas[m_nnodekinds+k];
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

    size = m_nkinds*(step_loop-Fparam.start_step);
    for(j=0;j<m_nkinds;j++){
        if(m_nullflags[j]){
            m_value_max[size+j] = FLT_MIN;
            m_value_min[size+j] = FLT_MAX;
            for(i=0;i<m_nnodes;i++){
                if(m_nulldatas[j]!=m_values[i*m_nkinds+j]){
                    if(m_value_max[size+j]<m_values[i*m_nkinds+j]) m_value_max[size+j] = m_values[i*m_nkinds+j];
                    if(m_value_min[size+j]>m_values[i*m_nkinds+j]) m_value_min[size+j] = m_values[i*m_nkinds+j];
                }
            }
        }else{
            m_value_max[size+j] = m_values[j];
            m_value_min[size+j] = m_values[j];
            for(i=1;i<m_nnodes;i++){
                if(m_value_max[size+j]<m_values[i*m_nkinds+j]) m_value_max[size+j] = m_values[i*m_nkinds+j];
                if(m_value_min[size+j]>m_values[i*m_nkinds+j]) m_value_min[size+j] = m_values[i*m_nkinds+j];
            }
        }
    }
    for(j=0;j<m_nkinds;j++){
        mt_nullflag[size+j] = m_nullflags[j];
        mt_nulldata[size+j] = m_nulldatas[j];
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
int write_ucdbin( int      step_loop     ,
                  int      volm_loop     ,
                  int      subvol_no     ,
                  Int64 subvol_nelems ,
                  Int64 subvol_nnodes )
{
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
    char          keyword[7] = "AVS UCD";

    sprintf(filename,"%s/%s.%05d.%07d.%07d.%s",Fparam.out_dir,Fparam.out_prefix,step_loop,volm_loop+1,m_nvolumes,Fparam.out_suffix);

    if(Fparam.headfoot){
        headfoot = (char) 1;
    }

    /* オープン */
    ifs = fopen(filename, "wb");
    if(NULL==ifs){
        filter_log(FILTER_ERR, "can not open %s.\n", filename);
        goto garbage;
    }
    //Head information

    header = 7;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;

    // keyword
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( keyword, 7, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // version
    version = 1.0f;
    header = 4;
    if(ENDIAN) version = ConvertEndianF(version);
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &version, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // title
    header = 70;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( m_title, 70, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // step num.
    stepn = m_stepn;
    header = 4;
    if(ENDIAN) header = ConvertEndianI(header);
    if(ENDIAN) stepn  = ConvertEndianI(stepn);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &stepn, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // step time 
    stept = m_stept;
    header = 4;
    if(ENDIAN) header = ConvertEndianI(header);
    if(ENDIAN) stept  = ConvertEndianF(stept);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &stept, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    //Node information

    // num. nodes
    nnodes = subvol_nnodes;
    header = 4;
    if(ENDIAN) header = ConvertEndianI(header);
    if(ENDIAN) nnodes = ConvertEndianI(nnodes);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &nnodes, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // description type should be 1.
    desc_type = m_desctype;
    header = 4;
    if(ENDIAN) header    = ConvertEndianI(header);
    if(ENDIAN) desc_type = ConvertEndianI(desc_type);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &desc_type, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // coordinates
    id = 0;
    header = 16;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    for(i=0;i<m_nnodes;i++){
        if((char) 0==m_nodeflag[i]){
            continue;
        }
//      id = m_ids[i];
        id++;
        coord[0] = m_coords[ i*3     ];
        coord[1] = m_coords[ i*3 + 1 ];
        coord[2] = m_coords[ i*3 + 2 ];
        if(ENDIAN){
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
    if(ENDIAN) nelements = ConvertEndianI(nelements);
    if(ENDIAN) header    = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &nelements, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // element num.
    header = 4*subvol_nelems;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<m_nelements;i++){
       if(subvol_no == m_subvolume_num[i]){
           element_id = m_elementids[i] ;
           if(ENDIAN) element_id = ConvertEndianI(element_id);
           fwrite( &element_id, 4, 1, ifs );
       }
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // material num.
    header = 4*subvol_nelems;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<m_nelements;i++){
        if(subvol_no == m_subvolume_num[i]) {
            material_id = m_materials[i] ;
            if(ENDIAN) material_id = ConvertEndianI(material_id);
            fwrite( &material_id, 4, 1, ifs );
        }
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // element type
    header = 1*subvol_nelems;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<m_nelements;i++){
        if(subvol_no==m_subvolume_num[i]) {
            element_type = m_elemtypes[i] ;
            fwrite( &element_type, 1, 1, ifs );
        }
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // connect      
    ivalue = (int *)malloc(sizeof(int) * 20 );
    header = 4*subvol_nelems*m_elemcoms;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    size = 0;
    j = 0;
    if(Fparam.struct_grid){
        inum = Fparam.dim1;
        jnum = Fparam.dim2;
        knum = Fparam.dim3;
        nelements = (inum-1)*(jnum-1)*(knum-1);
        size = 8;
        for(i=0;i<m_nelements;i++){
            if(subvol_no==m_subvolume_num[i]) {
                nid = m_conn_top_node[i];
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
                    new_id = m_nodes_trans[org_id-1];
                    ivalue[k] = new_id - 1 + 1;
                    if(ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
                    k++;
                }
                fwrite( ivalue, 4, size, ifs );
            }
        }
    }else{
        for(i=0;i<m_nelements;i++){
            size = m_elementsize[(int)m_elemtypes[i]];
            if(subvol_no==m_subvolume_num[i]) {
                k = 0;
                while(k<size){
                    org_id = m_connections[j+k];
                    new_id = m_nodes_trans[org_id-1];
                    ivalue[k] = new_id - 1 + 1;
                    if(ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
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
    ncomponents = m_ncomponents;
    header = 4;
    if(ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    if(ENDIAN) header      = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &ncomponents, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // data type
    data_type = m_datatype;
    header = 4;
    if(ENDIAN) data_type = ConvertEndianI(data_type);
    if(ENDIAN) header    = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &data_type, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // data header
    null_flag = 0;
    null_data = 0.0f;
    for(i=0;i<m_ncomponents;i++){

        header = 44;
        for(j=0;j<COMPONENT_LEN;j++){
           component_name[j] = pm_names[i][j];
           unit[j]           = pm_units[i][j];
        }
        veclen = m_veclens[i];
        null_flag = m_nullflags[i];
        null_data = m_nulldatas[i];
        if(ENDIAN){
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
    nkind = m_nkinds;
    value = (float *)malloc(sizeof(float) * nkind );
    header = 4*nkind;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    for(i=0;i<m_nnodes;i++){
        if((char) 1==m_nodeflag[i]){
            j = i*nkind;
            k = 0;
            while(k<nkind){
                value[k] = m_values[j+k];
                if(ENDIAN) value[k] = ConvertEndianF(value[k]);
                k++;
            }
            if(headfoot) fwrite( &header, 4, 1, ifs );
            fwrite( value, 4, nkind, ifs );
            if(headfoot) fwrite( &footer, 4, 1, ifs );
        }
    }

    header = 4;
    if(ENDIAN) header = ConvertEndianI(header);
    if(headfoot) fwrite( &header, 4, 1, ifs );
    header = 0;
    if(ENDIAN) header = ConvertEndianI(header);
    fwrite( &header, 4, 1, ifs );
    header = 4;
    if(ENDIAN) header = ConvertEndianI(header);
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
int write_ucdbin_elem( int      step_loop     ,
                  int      elem_id       ,
                  Int64 subvol_nelems ,
                  Int64 subvol_nnodes ,
                  Int64 rank_nodes    ,
                  Int64 rank_elems    )
{
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
    char          keyword[7] = "AVS UCD";
    int           elem_type = 0;
    int           nkind;

    elem_type = elem_id;

    sprintf(buff,Fparam.format,step_loop);
    sprintf(filename,"%s/%02d-%s%s%s",Fparam.out_dir,elem_type,Fparam.out_prefix,buff,Fparam.in_suffix);
    filter_log(FILTER_LOG,"|||| filename          : %s\n",    filename         );

//  if(Fparam.headfoot){
//      headfoot = (char) 1;
//  }
    headfoot = (char) 0;

    /* オープン */
    ifs = fopen(filename, "wb");
    if(NULL==ifs){
        filter_log(FILTER_ERR, "can not open %s.\n", filename);
        goto garbage;
    }
    //Head information

    header = 7;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;

    // keyword
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( keyword, 7, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // version
    version = 1.0f;
    header = 4;
    if(ENDIAN) version = ConvertEndianF(version);
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &version, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // title
    header = 70;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( m_title, 70, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // step num.
    stepn = step_loop;
    header = 4;
    if(ENDIAN) header = ConvertEndianI(header);
    if(ENDIAN) stepn  = ConvertEndianI(stepn);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &stepn, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // step time 
    stept = m_stept;
    header = 4;
    if(ENDIAN) header = ConvertEndianI(header);
    if(ENDIAN) stept  = ConvertEndianF(stept);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &stept, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    //Node information

    // num. nodes
    nnodes = subvol_nnodes;
    header = 4;
    if(ENDIAN) header = ConvertEndianI(header);
    if(ENDIAN) nnodes = ConvertEndianI(nnodes);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &nnodes, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // description type should be 1.
    desc_type = 1;
    header = 4;
    if(ENDIAN) header    = ConvertEndianI(header);
    if(ENDIAN) desc_type = ConvertEndianI(desc_type);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &desc_type, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // coordinates
    id = 0;
    header = 16;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    
    for(i=0;i<subvol_nnodes;i++){
        id = m_subvolume_nodes[rank_nodes+i];
        j = id * 3;
        coord[0] = m_coords[j  ];
        coord[1] = m_coords[j+1];
        coord[2] = m_coords[j+2];
        id++;
///////////////////
        id = i+1;
///////////////////
        if(ENDIAN){
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
    if(ENDIAN) nelements = ConvertEndianI(nelements);
    if(ENDIAN) header    = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &nelements, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // element num.
    header = 4*subvol_nelems;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<subvol_nelems;i++){
         element_id = i+1 ;
         if(ENDIAN) element_id = ConvertEndianI(element_id);
         fwrite( &element_id, 4, 1, ifs );
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // material num.
    header = 4*subvol_nelems;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<m_nelements;i++){
        if(elem_type==m_elemtypes[i]) {
            material_id = m_materials[i];
            if(ENDIAN) material_id = ConvertEndianI(material_id);
            fwrite( &material_id, 4, 1, ifs );
        }
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // element type
    header = 1*subvol_nelems;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    for(i=0;i<subvol_nelems;i++){
            element_type = elem_type ;
            fwrite( &element_type, 1, 1, ifs );
    }
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // connect      
    ivalue = (int *)malloc(sizeof(int) * 20 );
    header = 4*subvol_nelems*m_elemcoms;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    size = 0;
    j = 0;
    for(i=0;i<m_nelements;i++){
        size = m_elementsize[(int)m_elemtypes[i]];
        if(elem_type==m_elemtypes[i]) {
            k = 0;
            while(k<size){
                org_id = m_connections[j+k];
                new_id = m_nodes_trans[org_id-1];
                ivalue[k] = new_id - 1 + 1;
                if(ENDIAN) ivalue[k] = ConvertEndianI(ivalue[k]);
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
    ncomponents = m_ncomponents;
    header = 4;
    if(ENDIAN) ncomponents = ConvertEndianI(ncomponents);
    if(ENDIAN) header      = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &ncomponents, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // data type
    data_type = 1;
    header = 4;
    if(ENDIAN) data_type = ConvertEndianI(data_type);
    if(ENDIAN) header    = ConvertEndianI(header);
    footer = header;
    if(headfoot) fwrite( &header, 4, 1, ifs );
    fwrite( &data_type, 4, 1, ifs );
    if(headfoot) fwrite( &footer, 4, 1, ifs );

    // data header
    null_flag = 0;
    null_data = 0.0f;
    nkind = 0;
    for(i=0;i<m_ncomponents;i++){

        header = 44;
        for(j=0;j<COMPONENT_LEN;j++){
           component_name[j] = pm_names[i][j];
           unit[j]           = pm_units[i][j];
        }
//      strcpy(component_name,pm_names[i]);
//      strcpy(unit          ,pm_units[i]);
        veclen    = m_veclens[i];
        null_flag = m_nullflags[nkind];
        null_data = m_nulldatas[nkind];
        if(ENDIAN){
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
        nkind += m_veclens[i];
    }

    value = (float *)malloc(sizeof(float) * m_nkinds );
    header = 4*m_nkinds;
    if(ENDIAN) header = ConvertEndianI(header);
    footer = header;
    for(i=0;i<subvol_nnodes;i++){
        id = m_subvolume_nodes[rank_nodes+i];
        j = id*m_nkinds;
        k = 0;
        while(k<m_nkinds){
            value[k] = m_values[j+k];
            if(ENDIAN) value[k] = ConvertEndianF(value[k]);
            k++;
        }
        if(headfoot) fwrite( &header, 4, 1, ifs );
        fwrite( value, 4, m_nkinds, ifs );
        if(headfoot) fwrite( &footer, 4, 1, ifs );
    }
    free(value);
    value = NULL;

    header = 4;
    if(ENDIAN) header = ConvertEndianI(header);
    if(headfoot) fwrite( &header, 4, 1, ifs );
    header = 0;
    if(ENDIAN) header = ConvertEndianI(header);
    fwrite( &header, 4, 1, ifs );
    header = 4;
    if(ENDIAN) header = ConvertEndianI(header);
    if(headfoot) fwrite( &header, 4, 1, ifs );

    state = RTC_OK;
garbage:
    if(NULL!=ifs) fclose(ifs);
    return(state);
}
