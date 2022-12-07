#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "filter_io_avs/FilterIoAvs.h"

namespace pbvr {
namespace filter {


int FilterIoAvs::ChangeChar(char Text[], char Chr1, char Chr2) {
    int i;
    int len;
    len = (int) strlen(Text);
    for (i = 0; i < len; i++) {
    if (Text[i] == Chr1) Text[i] = Chr2;
    }
    return 0;
}
/*****************************************************************************/

/*
 * fldファイル全読込み
 *
 */
int FilterIoAvs::read_fld_file(void) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    int state = RTC_NG;
    FILE *ifs = NULL;
    char dlm[] = " =\t\n";
    char line [LINELEN_MAX];
    char line1[LINELEN_MAX];
    char linet[LINELEN_MAX];
    char *ch;
    char *tag = NULL;
    char *value = NULL;
    int i, j, ndim, nn;
    char flag0 = 0;
    char flag1 = 0;
    char flag2 = 0;
    int count;
    int do_step;
    char alloc_flag;

    count = 0;
    do_step = -1;
    alloc_flag = 0;

    sprintf(info->filename, "%s/%s", param->in_dir, param->field_file);
    ifs = fopen(info->filename, "r");
    if (ifs == NULL) {
        printf("Can't open file : %s\n", info->filename);
        return state;
    }

    param->nstep = 1;

    while (fgets(line, LINELEN_MAX, ifs) != NULL) {
        ch = line;
        if (*ch == '\n') {
            continue;
        }
        if (*ch == '#') {
            continue;
        }

        if (!strstr(line, "=")) {
            if (strstr(line, "ENDDO")/*||strstr(tag,"enddo")||strstr(tag,"Enddo")*/) {
                break;
            } else if (strstr(line, "EOT")/*||strstr(tag,"Eot")||strstr(tag,"eot")*/) {
                count++;
                param->nstep = count;
                flag0 = 1;
            } else if (strstr(line, "DO")/*||strstr(tag,"Do")||strstr(tag,"do")*/) {
                break;
            }
            continue;
        }
        for (i = 0; i < LINELEN_MAX; i++) {
            line1[i] = line[i];
        }

        tag = strtok(line, dlm);
        value = strtok(NULL, dlm);
        if (strstr(tag, "nstep")) {
            param->nstep = atoi(value);
            flag0 = 1;
            break;
        }
    }

    fseek(ifs, 0, SEEK_SET);
    count = 0;

    param->ndim = 1;
    param->dim1 = 1;
    param->dim2 = 1;
    param->dim3 = 1;
    param->nspace = 1;
    param->veclen = 1;

    memset(param->data, 0x00, 16);
    memset(param->field, 0x00, 16);
    memset(param->coordfile[0], 0x00, 16);
    memset(param->coordfile[1], 0x00, 16);
    memset(param->coordfile[2], 0x00, 16);
    memset(param->coordtype[0], 0x00, 16);
    memset(param->coordtype[1], 0x00, 16);
    memset(param->coordtype[2], 0x00, 16);
    memset(param->valuefile, 0x00, LINELEN_MAX);
    memset(param->valuetype, 0x00, LINELEN_MAX);

    param->coordskip[0] = 0;
    param->coordskip[1] = 0;
    param->coordskip[2] = 0;
    param->coordstride[0] = 1;
    param->coordstride[1] = 1;
    param->coordstride[2] = 1;
    param->valueskip = 0;
    param->min_ext[0] = 0;
    param->min_ext[1] = 0;
    param->min_ext[2] = 0;
    param->max_ext[0] = 0;
    param->max_ext[1] = 0;
    param->max_ext[2] = 0;

    while (fgets(line, LINELEN_MAX, ifs) != NULL) {
        ch = line;
        if (*ch == '\n') {
            continue;
        }
        if (*ch == '#') {
            continue;
        }

        /* エラー */
        if (strstr(line, "filetype=ascii")) {
            LOGOUT(FILTER_ERR, (char*)"Not support file_type=ascii .\n");
            return state;
        }
        if (strstr(line, "filetype=unformatted")) {
            LOGOUT(FILTER_ERR, (char*)"Not support file_type=unformatted .\n");
            return state;
        }


        if (!strstr(line, "=")) {
            if (strstr(line, "ENDDO")/*||strstr(tag,"enddo")||strstr(tag,"Enddo")*/) {
                ;
            } else if (strstr(line, "EOT")/*||strstr(tag,"Eot")||strstr(tag,"eot")*/) {
                count++;
                if (param->nstep <= count) {
                    break;
                }
            } else if (strstr(line, "DO")/*||strstr(tag,"Do")||strstr(tag,"do")*/) {
                do_step = count;
            }
            continue;
        }

        for (i = 0; i < LINELEN_MAX; i++) {
            line1[i] = line[i];
        }

        tag = strtok(line, dlm);
        value = strtok(NULL, dlm);

        if (strstr(tag, "nstep")) {
            param->nstep = atoi(value);
        } else if (strstr(tag, "ndim")) {
            param->ndim = atoi(value);
            flag1 = 1;
        } else if (strstr(tag, "dim1")) {
            param->dim1 = atoi(value);
        } else if (strstr(tag, "dim2")) {
            param->dim2 = atoi(value);
        } else if (strstr(tag, "dim3")) {
            param->dim3 = atoi(value);
        } else if (strstr(tag, "dim3")) {
            param->dim3 = atoi(value);
        } else if (strstr(tag, "nspace")) {
            param->nspace = atoi(value);
        } else if (strstr(tag, "veclen")) {
            param->veclen = atoi(value);
            flag2 = 1;
        } else if (strstr(tag, "data")) {
            sprintf(param->data, "%s", value);
        } else if (strstr(tag, "field")) {
            sprintf(param->field, "%s", value);

        } else if (strstr(tag, "min_ext")) {
            param->min_ext[0] = atof(value);
            value = strtok(line1, " ");
            value = strtok(NULL, " ");
            param->min_ext[1] = atof(value);
            value = strtok(NULL, " ");
            param->min_ext[2] = atof(value);
        } else if (strstr(tag, "max_ext")) {
            param->max_ext[0] = atof(value);
            value = strtok(line1, " ");
            value = strtok(NULL, " ");
            value = strtok(NULL, " ");
            param->max_ext[1] = atof(value);
            value = strtok(NULL, " ");
            param->max_ext[2] = atof(value);
        } else if (strstr(tag, "coord")) {
            ndim = atoi(value);
            value = strtok(line1, " ");
            if (!alloc_flag) {
                alloc_stepfileinfo();
                alloc_flag = 1;
            }
            while (value != NULL) {
                if (strstr(value, "file=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    sprintf(param->coordfile[ndim - 1], "%s", ch + 1);
                    sprintf(param->stepf[count].coord[ndim - 1].fname, "%s", ch + 1);
                } else if (strstr(value, "filetype=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    sprintf(param->coordtype[ndim - 1], "%s", ch + 1);
                    sprintf(param->stepf[count].coord[ndim - 1].ftype, "%s", ch + 1);
                } else if (strstr(value, "skip=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    param->coordskip[ndim - 1] = atoi(ch + 1);
                    param->stepf[count].coord[ndim - 1].skip = atoi(ch + 1);
                } else if (strstr(value, "offset=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    param->stepf[count].coord[ndim - 1].offset = atoi(ch + 1);
                } else if (strstr(value, "stride=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    param->coordstride[ndim - 1] = atoi(ch + 1);
                    param->stepf[count].coord[ndim - 1].stride = atoi(ch + 1);
                } else if (strstr(value, "close=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    param->stepf[count].coord[ndim - 1].close = atoi(ch + 1);
                } else {
                    ;
                }
                value = strtok(NULL, " ");
            }
        } else if (strstr(tag, "variable")) {
            nn = atoi(value);
            value = strtok(line1, " ");
            if (!alloc_flag) {
                alloc_stepfileinfo();
                alloc_flag = 1;
            }
            while (value != NULL) {
                if (strstr(value, "file=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    sprintf(param->valuefile, "%s", ch + 1);
                    sprintf(param->stepf[count].value[nn - 1].fname, "%s", ch + 1);
                } else if (strstr(value, "filetype=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    sprintf(param->valuetype, "%s", ch + 1);
                    sprintf(param->stepf[count].value[nn - 1].ftype, "%s", ch + 1);
                } else if (strstr(value, "skip=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    param->valueskip = atoi(ch + 1);
                    param->stepf[count].value[nn - 1].skip = atoi(ch + 1);
                } else if (strstr(value, "offset=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    param->stepf[count].value[nn - 1].offset = atoi(ch + 1);
                } else if (strstr(value, "stride=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    param->stepf[count].value[nn - 1].stride = atoi(ch + 1);
                } else if (strstr(value, "close=")) {
                    sprintf(linet, "%s", value);
                    ch = strstr(linet, "=");
                    param->stepf[count].value[nn - 1].close = atoi(ch + 1);
                } else {
                    ;
                }
                value = strtok(NULL, " ");
            }
        } else {
            ;
        }

        if (flag0 && flag1 && flag2&&!alloc_flag) {
            alloc_stepfileinfo();
            alloc_flag = 1;
            flag0 = 0;
            flag1 = 0;
            flag2 = 0;
        }
    }

    fclose(ifs);

    LOGOUT(FILTER_LOG, (char*)"param->nstep    : %d\n", param->nstep);
    LOGOUT(FILTER_LOG, (char*)"param->ndim     : %d\n", param->ndim);
    LOGOUT(FILTER_LOG, (char*)"param->dim1     : %d\n", param->dim1);
    LOGOUT(FILTER_LOG, (char*)"param->dim2     : %d\n", param->dim2);
    LOGOUT(FILTER_LOG, (char*)"param->dim3     : %d\n", param->dim3);
    LOGOUT(FILTER_LOG, (char*)"param->nspace   : %d\n", param->nspace);
    LOGOUT(FILTER_LOG, (char*)"param->veclen   : %d\n", param->veclen);
    LOGOUT(FILTER_LOG, (char*)"param->data     : %s\n", param->data);
    LOGOUT(FILTER_LOG, (char*)"param->field    : %s\n", param->field);
    for (i = 0; i < 3; i++) {
        LOGOUT(FILTER_LOG, (char*)"param->coordfile   : %s\n", param->coordfile[i]);
        LOGOUT(FILTER_LOG, (char*)"param->coordtype   : %s\n", param->coordtype[i]);
        LOGOUT(FILTER_LOG, (char*)"param->coordskip   : %d\n", param->coordskip[i]);
        LOGOUT(FILTER_LOG, (char*)"param->coordstride : %d\n", param->coordstride[i]);
    }
    LOGOUT(FILTER_LOG, (char*)"param->valuefile   : %s\n", param->valuefile);
    LOGOUT(FILTER_LOG, (char*)"param->valuetype   : %s\n", param->valuetype);
    LOGOUT(FILTER_LOG, (char*)"param->valueskip   : %d\n", param->valueskip);
    LOGOUT(FILTER_LOG, (char*)"do_step            : %d\n", do_step);
    LOGOUT(FILTER_LOG, (char*)"param->min_ext[0]  : %f\n", param->min_ext[0]);
    LOGOUT(FILTER_LOG, (char*)"param->min_ext[1]  : %f\n", param->min_ext[1]);
    LOGOUT(FILTER_LOG, (char*)"param->min_ext[2]  : %f\n", param->min_ext[2]);
    LOGOUT(FILTER_LOG, (char*)"param->max_ext[0]  : %f\n", param->max_ext[0]);
    LOGOUT(FILTER_LOG, (char*)"param->max_ext[1]  : %f\n", param->max_ext[1]);
    LOGOUT(FILTER_LOG, (char*)"param->max_ext[2]  : %f\n", param->max_ext[2]);

    if (do_step > 0) {
        for (i = do_step + 1; i < param->nstep; i++) {
            for (j = 0; j < param->nspace; j++) {
                sprintf(param->stepf[i].coord[j].fname, "%s", param->stepf[do_step].coord[j].fname);
                sprintf(param->stepf[i].coord[j].ftype, "%s", param->stepf[do_step].coord[j].ftype);
                param->stepf[i].coord[j].skip = param->stepf[do_step].coord[j].skip;
                param->stepf[i].coord[j].offset = param->stepf[do_step].coord[j].offset;
                param->stepf[i].coord[j].stride = param->stepf[do_step].coord[j].stride;
                param->stepf[i].coord[j].close = param->stepf[do_step].coord[j].close;
            }
            for (j = 0; j < param->veclen; j++) {
                sprintf(param->stepf[i].value[j].fname, "%s", param->stepf[do_step].value[j].fname);
                sprintf(param->stepf[i].value[j].ftype, "%s", param->stepf[do_step].value[j].ftype);
                param->stepf[i].value[j].skip = param->stepf[do_step].value[j].skip;
                param->stepf[i].value[j].offset = param->stepf[do_step].value[j].offset;
                param->stepf[i].value[j].stride = param->stepf[do_step].value[j].stride;
                param->stepf[i].value[j].close = param->stepf[do_step].value[j].close;
            }
        }
    }
    for (i = 0; i < param->nstep; i++) {
        if (i > 3) break;
        for (j = 0; j < param->nspace; j++) {
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].coord[%2d].fname : %s\n", i, j, param->stepf[i].coord[j].fname);
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].coord[%2d].ftype : %s\n", i, j, param->stepf[i].coord[j].ftype);
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].coord[%2d].skip  : %d\n", i, j, param->stepf[i].coord[j].skip);
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].coord[%2d].offset: %d\n", i, j, param->stepf[i].coord[j].offset);
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].coord[%2d].stride: %d\n", i, j, param->stepf[i].coord[j].stride);
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].coord[%2d].close : %d\n", i, j, param->stepf[i].coord[j].close);
        }
        for (j = 0; j < param->veclen; j++) {
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].value[%2d].fname : %s\n", i, j, param->stepf[i].value[j].fname);
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].value[%2d].ftype : %s\n", i, j, param->stepf[i].value[j].ftype);
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].value[%2d].skip  : %d\n", i, j, param->stepf[i].value[j].skip);
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].value[%2d].offset: %d\n", i, j, param->stepf[i].value[j].offset);
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].value[%2d].stride: %d\n", i, j, param->stepf[i].value[j].stride);
            LOGOUT(FILTER_LOG, (char*)"param->stepf[%3d].value[%2d].close : %d\n", i, j, param->stepf[i].value[j].close);
        }
    }

    //  if((param->start_step==0)&&(param->end_step==0)){
    //      param->end_step = param->nstep - 1;
    //  }

    if (param->ndim == 2) {
        info->m_nnodes = param->dim1 * param->dim2;
        info->m_nelements = (param->dim1 - 1)*(param->dim2 - 1);
        info->m_elemcoms = 4;
        info->m_element_type = 3;
    } else if (param->ndim == 3) {
        info->m_nnodes = param->dim1 * param->dim2 * param->dim3;
        info->m_nelements = (param->dim1 - 1)*(param->dim2 - 1)*(param->dim3 - 1);
        info->m_elemcoms = 8;
        info->m_element_type = 7;
    } else {
        LOGOUT(FILTER_ERR, (char*)"Not support dimension : %d\n", param->ndim);
        return (RTC_NG);
    }
    info->m_connectsize = info->m_nelements*info->m_elemcoms;
    info->m_ncomponents = param->veclen;
    info->m_nkinds = param->veclen;
    info->m_desctype = 1;
    info->m_datatype = 1;

    state = RTC_OK;
    return (state);
}


/*****************************************************************************/

/*
 * 形状ファイル読込み
 *
 */
int FilterIoAvs::alloc_stepfileinfo(void) {
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    FilterParam *param = this->m_filter_info->m_param;

    int i, j;
    Fdetail *fdc;
    Fdetail *fdv;
    Stepfile *sf;

    sf = (Stepfile*) malloc(sizeof (Stepfile) * param->nstep);
    if (NULL == sf) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(sf).\n");
        goto garbage;
    }
    for (i = 0; i < param->nstep; i++) {
        fdc = (Fdetail*) malloc(sizeof (Fdetail) * param->nspace);
        fdv = (Fdetail*) malloc(sizeof (Fdetail) * param->veclen);
        for (j = 0; j < param->nspace; j++) {
            memset(fdc[j].fname, 0x00, LINELEN_MAX);
            memset(fdc[j].ftype, 0x00, 16);
            fdc[j].skip = 0;
            fdc[j].offset = 0;
            fdc[j].stride = 1;
            fdc[j].close = 1;
        }
        for (j = 0; j < param->veclen; j++) {
            memset(fdv[j].fname, 0x00, LINELEN_MAX);
            memset(fdv[j].ftype, 0x00, 16);
            fdv[j].skip = 0;
            fdv[j].offset = 0;
            fdv[j].stride = 1;
            fdv[j].close = 1;
        }
        sf[i].coord = fdc;
        sf[i].value = fdv;
    }
    param->stepf = sf;
    return (RTC_OK);
garbage:
    return (RTC_NG);
}

/*****************************************************************************/

/*
 * 形状ファイル読込み
 *
 */
int FilterIoAvs::read_geom( void )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    float *m_coords = info->m_coords;
    NodeInf *min = &info->node_min;
    NodeInf *max = &info->node_max;

    int state = RTC_NG;
    FILE *ifs = NULL;
    char descript[80];
    int part;
    int dimx;
    int dimy;
    int dimz;
    float *coordx = NULL;
    float *coordy = NULL;
    float *coordz = NULL;
    size_t i;
	size_t m_nnodes;
	size_t dimSize;

    sprintf(info->filename, "%s/%s", param->in_dir, param->geom_file);

    /* 1時刻目のデータから節点情報を読み込む */

    /* オープン */
    ifs = fopen(info->filename, "rb");
    if (NULL == ifs) {
        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
        goto garbage;
    }

    /* description */
    fread(descript, 80, 1, ifs);
    fread(descript, 80, 1, ifs);
    fread(descript, 80, 1, ifs);
    for (i = 0; i < 70; i++) {
        if (descript[i] == '\0') {
            break;
        }
        info->m_title[i] = descript[i];
    }
    fread(descript, 80, 1, ifs);
    fread(descript, 80, 1, ifs);
    fread(descript, 80, 1, ifs);

    /* part */
    fread(&part, 4, 1, ifs);

    /* description */
    for (i = 0; i < 2; i++) {
        fread(descript, 80, 1, ifs);
    }

    /* I J K */
    fread(&dimx, 4, 1, ifs);
    fread(&dimy, 4, 1, ifs);
    fread(&dimz, 4, 1, ifs);
    if (info->ENDIAN) {
        dimx = ConvertEndianI(dimx);
        dimy = ConvertEndianI(dimy);
        dimz = ConvertEndianI(dimz);
    }

    coordx = (float*) malloc(sizeof (float)*info->m_nnodes);
    coordy = (float*) malloc(sizeof (float)*info->m_nnodes);
    coordz = (float*) malloc(sizeof (float)*info->m_nnodes);
    if (NULL == coordz) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(coord) %s.\n", info->filename);
        goto garbage;
    }

    fread(coordx, sizeof (float), info->m_nnodes, ifs);
    fread(coordy, sizeof (float), info->m_nnodes, ifs);
    fread(coordz, sizeof (float), info->m_nnodes, ifs);

	m_nnodes = (size_t)info->m_nnodes;
    for (i = 0; i < m_nnodes; i += 1) {
        info->m_ids[i] = i + 1;
        if (info->ENDIAN) {
            coordx[i] = ConvertEndianF(coordx[i]);
            coordy[i] = ConvertEndianF(coordy[i]);
            coordz[i] = ConvertEndianF(coordz[i]);
        }
        m_coords[3 * i ] = coordx[i];
        m_coords[3 * i + 1] = coordy[i];
        m_coords[3 * i + 2] = coordz[i];
    }

    /* 最大・最小を取得 */
    min->x = max->x = m_coords[0];
    min->y = max->y = m_coords[1];
    min->z = max->z = m_coords[2];
	dimSize = (size_t)(dimx * dimy * dimz);
    for (i = 1; i < dimSize; i++) {
        if (min->x > m_coords[i * 3 ]) min->x = m_coords[i * 3];
        if (min->y > m_coords[i * 3 + 1]) min->y = m_coords[i * 3 + 1];
        if (min->z > m_coords[i * 3 + 2]) min->z = m_coords[i * 3 + 2];

        if (max->x < m_coords[i * 3 ]) max->x = m_coords[i * 3];
        if (max->y < m_coords[i * 3 + 1]) max->y = m_coords[i * 3 + 1];
        if (max->z < m_coords[i * 3 + 2]) max->z = m_coords[i * 3 + 2];
    }

    if (info->mype_f == 0) LOGOUT(FILTER_LOG, (char*)"  max -> %8.2f, %8.2f, %8.2f\n", max->x, max->y, max->z);
    if (info->mype_f == 0) LOGOUT(FILTER_LOG, (char*)"  min -> %8.2f, %8.2f, %8.2f\n", min->x, min->y, min->z);

    free(coordx);
    free(coordy);
    free(coordz);
    coordx = NULL;
    coordy = NULL;
    coordz = NULL;

    state = RTC_OK;

garbage:
    if (NULL != ifs) fclose(ifs);
    return (state);
}

/*****************************************************************************/

/*
 * 形状ファイル読込み
 *
 */
int FilterIoAvs::read_coord( void )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    float *m_coords = info->m_coords;
    NodeInf *min = &info->node_min;
    NodeInf *max = &info->node_max;

    int state = RTC_NG;
    FILE *ifs = NULL;
    int dimx;
    int dimy;
    int dimz;
    Int64 i;
    Int64 j;
    int skip;
    int stride;
    int bsize;
    int div;
    int mod;

    bsize = param->blocksize;
    dimx = param->dim1;
    dimy = param->dim2;
    dimz = param->dim3;

    if (!strcmp(param->coordfile[0], param->coordfile[1]) &&
        !strcmp(param->coordfile[0], param->coordfile[2]) &&
        param->coordstride[0] == 3 && param->coordstride[1] == 3 && param->coordstride[2] == 3 &&
        param->coordskip[0] == 0 && param->coordskip[1] == 4 && param->coordskip[2] == 8) {
        sprintf(info->filename, "%s/%s", param->in_dir, param->coordfile[0]);
        /* オープン */
        ifs = fopen(info->filename, "rb");
        if (NULL == ifs) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
            goto garbage;
        }
#ifdef BLOCK_READ
    div = (dimx * dimy * dimz * 3) / bsize;
        mod = (dimx * dimy * dimz * 3) % bsize;
        for (i = 0; i < div; i++) {
            fread(&m_coords[bsize * i], 4, bsize, ifs);
        }
        if (mod > 0) {
            fread(&m_coords[bsize * div], 4, mod, ifs);
        }
#else
        for (i = 0; i < dimx * dimy * dimz * 3; i++) {
            fread(&m_coords[i], 4, 1, ifs);
        }
#endif
    } else {
        for (j = 0; j < param->ndim; j++) {
            sprintf(info->filename, "%s/%s", param->in_dir, param->coordfile[j]);
            /* オープン */
            ifs = fopen(info->filename, "rb");
            if (NULL == ifs) {
                LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
                goto garbage;
            }
            skip = param->coordskip[j];
            stride = (param->coordstride[j] - 1)*4;
            fseek(ifs, skip, SEEK_CUR);
            fread(&m_coords[j], 4, 1, ifs);
            for (i = 1; i < info->m_nnodes; i += 1) {
                fseek(ifs, stride, SEEK_CUR);
                fread(&m_coords[i * 3 + j], 4, 1, ifs);
            }
            fclose(ifs);
            ifs = NULL;
        }
    }
#ifndef NO_ENDIAN
    if (info->ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
        for (i = 0; i < dimx * dimy * dimz * 3; i += 1) {
            m_coords[i] = ConvertEndianF(m_coords[i]);
        }
    }
#endif

    /* 最大・最小を取得 */
    min->x = max->x = m_coords[0];
    min->y = max->y = m_coords[1];
    min->z = max->z = m_coords[2];
    for (i = 1; i < dimx * dimy * dimz; i++) {
    if (min->x > m_coords[i * 3 ]) min->x = m_coords[i * 3];
    if (min->y > m_coords[i * 3 + 1]) min->y = m_coords[i * 3 + 1];
    if (min->z > m_coords[i * 3 + 2]) min->z = m_coords[i * 3 + 2];

    if (max->x < m_coords[i * 3 ]) max->x = m_coords[i * 3];
    if (max->y < m_coords[i * 3 + 1]) max->y = m_coords[i * 3 + 1];
    if (max->z < m_coords[i * 3 + 2]) max->z = m_coords[i * 3 + 2];
    }

    if (info->mype_f == 0) LOGOUT(FILTER_LOG, (char*)"  max -> %8.2f, %8.2f, %8.2f\n", max->x, max->y, max->z);
    if (info->mype_f == 0) LOGOUT(FILTER_LOG, (char*)"  min -> %8.2f, %8.2f, %8.2f\n", min->x, min->y, min->z);

    state = RTC_OK;
garbage:
    if (NULL != ifs) fclose(ifs);
    return (state);
}

/*****************************************************************************/

/*
 * 形状ファイル読込み
 *
 */
int FilterIoAvs::read_coord_1( int step )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    float *m_coords = info->m_coords;
    NodeInf *min = &info->node_min;
    NodeInf *max = &info->node_max;

    int state = RTC_NG;
    FILE *ifs = NULL;
    int dimx;
    int dimy;
    int dimz;
    Int64 i;
    Int64 dm;
    int skip;
    int stride;
    int bsize;
    int div;
    int mod;
    char *cToken;
    char buff[LINELEN_MAX];
    int max_item;
    int offset_cnt;
    int stride_cnt;
    int item_cnt;
    char *workc;
    short *works;
    int *worki;
    float *work;
    double *workd;
    int tsize;

    bsize = param->blocksize;
    dimx = param->dim1;
    dimy = param->dim2;
    dimz = param->dim3;

    if (!strcmp(param->stepf[step].coord[0].fname, param->stepf[step].coord[1].fname) &&
        !strcmp(param->stepf[step].coord[0].fname, param->stepf[step].coord[2].fname) &&
        strstr(param->stepf[step].coord[0].ftype, "binary") &&
        strstr(param->stepf[step].coord[1].ftype, "binary") &&
        strstr(param->stepf[step].coord[2].ftype, "binary") &&
        param->stepf[step].coord[0].stride == 3 &&
        param->stepf[step].coord[1].stride == 3 &&
        param->stepf[step].coord[2].stride == 3 &&
        param->stepf[step].coord[0].skip == 0 &&
        param->stepf[step].coord[1].skip == 4 &&
        param->stepf[step].coord[2].skip == 8 &&
        !strcmp(param->data, "float")) {

        sprintf(info->filename, "%s/%s", param->in_dir, param->stepf[step].coord[0].fname);
        /* オープン */
        ifs = fopen(info->filename, "rb");
        if (NULL == ifs) {
            LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
            goto garbage;
        }
        info->m_geom_fp[0] = info->m_geom_fp[1] = info->m_geom_fp[2] = ifs;
#ifdef BLOCK_READ
        div = (dimx * dimy * dimz * 3) / bsize;
        mod = (dimx * dimy * dimz * 3) % bsize;
        for (i = 0; i < div; i++) {
            fread(&m_coords[bsize * i], 4, bsize, ifs);
        }
        if (mod > 0) {
            fread(&m_coords[bsize * div], 4, mod, ifs);
        }
#else
        for (i = 0; i < dimx * dimy * dimz * 3; i++) {
            fread(&m_coords[i], 4, 1, ifs);
        }
#endif
#ifndef NO_ENDIAN
        if (info->ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
            for (i = 0; i < dimx * dimy * dimz * 3; i += 1) {
                m_coords[i] = ConvertEndianF(m_coords[i]);
            }
        }
#endif
        if (param->stepf[step].coord[0].close == 1) {
            fclose(ifs);
            ifs = NULL;
            info->m_geom_fp[0] = info->m_geom_fp[1] = info->m_geom_fp[2] = NULL;
        }
    } else {
        work = (float*) malloc(sizeof (float)*info->m_nnodes);
        for (dm = 0; dm < param->nspace; dm++) {
            sprintf(info->filename, "%s/%s", param->in_dir, param->stepf[step].coord[dm].fname);
            if (strstr(param->stepf[step].coord[dm].ftype, "ascii")) {
                if (info->m_geom_fp[dm] == NULL) {
                    /* オープン */
                    ifs = fopen(info->filename, "r");
                    if (NULL == ifs) {
                        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
                        goto garbage;
                    }
                } else {
                    ifs = info->m_geom_fp[dm];
                }
                max_item = 0;
                if (strstr(param->field, "irregular")) {
                    max_item = info->m_nnodes;
                } else {
                    if (dm == 0) max_item = param->dim1;
                    if (dm == 1) max_item = param->dim2;
                    if (dm == 2) max_item = param->dim3;
                }
                /* とりあえずSKIP行分読み飛ばす */
                for (i = 0; i < param->stepf[step].coord[dm].skip; i++) {
                    if (fgets(buff, LINELEN_MAX, ifs) == NULL) {
                        fclose(ifs);
                        ifs = NULL;
                        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
                        goto garbage;
                    }
                }
                offset_cnt = param->stepf[step].coord[dm].offset;
                stride_cnt = 0;
                item_cnt = 0;
                while (fgets(buff, LINELEN_MAX, ifs)) {
                    /* タブを空白に置き換える */
                    ChangeChar(buff, '\t', ' ');
                    /* オープン */
                    /* 1行に対してtok 1回目 */
                    if ((cToken = strtok(buff, " \n")) != NULL) {
                        if (offset_cnt > 0) {
                            /* offset分は読み飛ばす(最初だけ) */
                            offset_cnt--;
                        } else {
                            /* offset分は読み飛ばされている場合 */
                            if (stride_cnt == 0) {
                            /* 値を取得 */
                            work[item_cnt] = (float) atof(cToken);
                            item_cnt++;
                            if (item_cnt >= max_item)
                                break;
                            }
                            stride_cnt++;
                            if (stride_cnt == param->stepf[step].coord[dm].stride) {
                            stride_cnt = 0;
                            }
                        }
                    }
                    /* 1行に対してtok 2回目以降 */
                    while ((cToken = strtok(NULL, " \n")) != NULL) {
                        if (offset_cnt > 0) {
                            /* offset分は読み飛ばす(最初だけ) */
                            offset_cnt--;
                        } else {
                            /* offset分は読み飛ばされている場合 */
                            if (stride_cnt == 0) {
                            /* 値を取得 */
                            work[item_cnt] = (float) atof(cToken);
                            item_cnt++;
                            if (item_cnt >= max_item) {
                                break;
                            }
                            }
                            stride_cnt++;
                            if (stride_cnt == param->stepf[step].coord[dm].stride) {
                                stride_cnt = 0;
                            }
                        }
                    }
                }
            } else {
                /* オープン */
                if (info->m_geom_fp[dm] == NULL) {
                    ifs = fopen(info->filename, "rb");
                    if (NULL == ifs) {
                    LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
                    goto garbage;
                    }
                } else {
                    ifs = info->m_geom_fp[dm];
                }
                if (!strcmp(param->data, "byte")) {
                    tsize = sizeof (char);
                } else if (!strcmp(param->data, "short")) {
                    tsize = sizeof (short);
                } else if (!strcmp(param->data, "int")) {
                    tsize = sizeof (int);
                } else if (!strcmp(param->data, "float")) {
                    tsize = sizeof (float);
                } else if (!strcmp(param->data, "double")) {
                    tsize = sizeof (double);
                } else {
                    tsize = 1;
                }
                skip = param->stepf[step].coord[dm].skip;
                stride = (param->stepf[step].coord[dm].stride - 1) * tsize;
                fseek(ifs, skip, SEEK_CUR);
                if (!strcmp(param->data, "byte")) {
                    workc = (char*) malloc(sizeof (char)*info->m_nnodes);
                    fread(&workc[0], tsize, 1, ifs);
                    for (i = 1; i < info->m_nnodes; i += 1) {
                        fseek(ifs, stride, SEEK_CUR);
                        fread(&workc[i], tsize, 1, ifs);
                    }
                    for (i = 0; i < info->m_nnodes; i += 1) {
                        work[i] = (float) workc[i];
                    }
                    free(workc);
                } else if (!strcmp(param->data, "short")) {
                    works = (short*) malloc(sizeof (short)*info->m_nnodes);
                    fread(&works[0], tsize, 1, ifs);
                    for (i = 1; i < info->m_nnodes; i += 1) {
                        fseek(ifs, stride, SEEK_CUR);
                        fread(&works[i], tsize, 1, ifs);
                    }
                    for (i = 0; i < info->m_nnodes; i += 1) {
                        work[i] = (float) works[i];
                    }
                    free(works);
                } else if (!strcmp(param->data, "int")) {
                    worki = (int*) malloc(sizeof (int)*info->m_nnodes);
                    fread(&worki[0], tsize, 1, ifs);
                    for (i = 1; i < info->m_nnodes; i += 1) {
                        fseek(ifs, stride, SEEK_CUR);
                        fread(&worki[i], tsize, 1, ifs);
                    }
                    for (i = 0; i < info->m_nnodes; i += 1) {
                        work[i] = (float) worki[i];
                    }
                    free(worki);
                } else if (!strcmp(param->data, "float")) {
                    fread(&work[0], tsize, 1, ifs);
                    for (i = 1; i < info->m_nnodes; i += 1) {
                        fseek(ifs, stride, SEEK_CUR);
                        fread(&work[i], tsize, 1, ifs);
                    }
                } else if (!strcmp(param->data, "double")) {
                    workd = (double*) malloc(sizeof (double)*info->m_nnodes);
                    fread(&workd[0], tsize, 1, ifs);
                    for (i = 1; i < info->m_nnodes; i += 1) {
                        fseek(ifs, stride, SEEK_CUR);
                        fread(&workd[i], tsize, 1, ifs);
                    }
                    for (i = 0; i < info->m_nnodes; i += 1) {
                        work[i] = (float) workd[i];
                    }
                    free(workd);
                } else {
                    tsize = 1;
                }
#ifndef NO_ENDIAN
        if (info->ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
            for (i = 0; i < info->m_nnodes; i++) {
                work[i] = ConvertEndianF(work[i]);
            }
        }
#endif
        }
        if (param->stepf[step].coord[dm].close == 1) {
            fclose(ifs);
            ifs = NULL;
            info->m_geom_fp[dm] = NULL;
        }
        if (strstr(param->field, "irregular")) {
            for (i = 0; i < info->m_nnodes; i++) {
                m_coords[i * 3 + dm] = work[i];
            }
        } else {
        int ii, jj, kk, id;
        if (dm == 0) {
            for (kk = 0; kk < param->dim3; kk++) {
                for (jj = 0; jj < param->dim2; jj++) {
                    for (ii = 0; ii < param->dim1; ii++) {
                    id = ii + (param->dim1) * jj + (param->dim1)*(param->dim2) * kk;
                    m_coords[id * 3 + dm] = work[ii];
                    }
                }
            }
        } else if (dm == 1) {
            for (kk = 0; kk < param->dim3; kk++) {
                for (jj = 0; jj < param->dim2; jj++) {
                    for (ii = 0; ii < param->dim1; ii++) {
                    id = ii + (param->dim1) * jj + (param->dim1)*(param->dim2) * kk;
                    m_coords[id * 3 + dm] = work[jj];
                    }
                }
            }
        } else {
            for (kk = 0; kk < param->dim3; kk++) {
                for (jj = 0; jj < param->dim2; jj++) {
                    for (ii = 0; ii < param->dim1; ii++) {
                    id = ii + (param->dim1) * jj + (param->dim1)*(param->dim2) * kk;
                    m_coords[id * 3 + dm] = work[kk];
                    }
                }
            }
        }
        }
    }
    free(work);
    work = NULL;
    }

    /* 最大・最小を取得 */
    min->x = max->x = m_coords[0];
    min->y = max->y = m_coords[1];
    min->z = max->z = m_coords[2];
    for (i = 1; i < dimx * dimy * dimz; i++) {
    if (min->x > m_coords[i * 3 ]) min->x = m_coords[i * 3];
    if (min->y > m_coords[i * 3 + 1]) min->y = m_coords[i * 3 + 1];
    if (min->z > m_coords[i * 3 + 2]) min->z = m_coords[i * 3 + 2];

    if (max->x < m_coords[i * 3 ]) max->x = m_coords[i * 3];
    if (max->y < m_coords[i * 3 + 1]) max->y = m_coords[i * 3 + 1];
    if (max->z < m_coords[i * 3 + 2]) max->z = m_coords[i * 3 + 2];
    }

    if (info->mype_f == 0) LOGOUT(FILTER_LOG, (char*)"  max -> %8.2f, %8.2f, %8.2f\n", max->x, max->y, max->z);
    if (info->mype_f == 0) LOGOUT(FILTER_LOG, (char*)"  min -> %8.2f, %8.2f, %8.2f\n", min->x, min->y, min->z);

    state = RTC_OK;
garbage:
    if (NULL != ifs) fclose(ifs);
    return (state);
}



/*****************************************************************************/

/*
 * variablesファイル読込み
 *
 */
int FilterIoAvs::read_variables_per_step(int step_loop)
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *buff = info->buff;

    int state = RTC_NG;
    FILE *ifs = NULL;
    char descript[80];
    int part;
    int blk;
    Int64 cnt;
    Int64 i;
    int bsize;
    int div;
    int mod;

    bsize = param->blocksize;
    sprintf(buff, param->format, step_loop);
    sprintf(info->filename, "%s/%s%s%s", param->in_dir, param->in_prefix, buff, param->in_suffix);

    /* オープン */
    ifs = fopen(info->filename, "rb");
        if (NULL == ifs) {
        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
        goto garbage;
    }

    /* description line 1 */
    fread(descript, 80, 1, ifs);
    /* description (part) */
    fread(descript, 80, 1, ifs);
    /* part */
    fread(&part, 4, 1, ifs);
    if (info->ENDIAN) part = ConvertEndianI(part);
    /* block */
    fread(descript, 80, 1, ifs);
    /* data */
#ifdef BLOCK_READ
    div = info->m_nnodes / bsize;
    mod = info->m_nnodes % bsize;
    cnt = 0;
    for (i = 0; i < div; i++) {
    cnt += fread(&info->m_values[bsize * i], sizeof (float), bsize, ifs);
    }
    if (mod > 0) {
    cnt += fread(&info->m_values[bsize * div], sizeof (float), mod, ifs);
    }
#else
    cnt = fread(info->m_values, sizeof (float), m_nnodes, ifs);
#endif
    if (info->m_nnodes != cnt) {
        LOGOUT(FILTER_ERR, (char*)"[FilterIoAvs::read_variables_per_step] read error %d != %d .\n", cnt, info->m_nnodes);
        goto garbage;
    }
    if (info->ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
    for (i = 0; i < info->m_nnodes; i++) {
        info->m_values[i] = ConvertEndianF(info->m_values[i]);
    }
    }

    blk = step_loop - param->start_step;

    info->m_value_max[blk] = info->m_values[0];
    info->m_value_min[blk] = info->m_values[0];
    for (i = 1; i < info->m_nnodes; i++) {
        if (info->m_value_max[blk] < info->m_values[i]) info->m_value_max[blk] = info->m_values[i];
        if (info->m_value_min[blk] > info->m_values[i]) info->m_value_min[blk] = info->m_values[i];
    }
    state = RTC_OK;
garbage:
    if (NULL != ifs) fclose(ifs);
    return (state);
}

/*****************************************************************************/

/*
 * variablesファイル読込み
 *
 */
int FilterIoAvs::read_variables( int step_loop )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    int state = RTC_NG;
    FILE *ifs = NULL;
    //  int     step=0;
    int skip;
    int step;
    int blk;
    Int64 cnt;
    Int64 i;
    int bsize;
    int div;
    int mod;

    bsize = param->blocksize;

    sprintf(info->filename, "%s/%s", param->in_dir, param->valuefile);

    /* オープン */
    ifs = fopen(info->filename, "rb");
    if (NULL == ifs) {
        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
        goto garbage;
    }

    skip = info->m_nnodes * 4;
    for (;;) {
        fread(&step, 4, 1, ifs);
#ifndef NO_ENDIAN
        if (info->ENDIAN) step = ConvertEndianI(step);
#endif
        if (step == step_loop) {
            break;
        }
        fseek(ifs, skip, SEEK_CUR);
    }

    /* data */
    cnt = 0;
#ifdef BLOCK_READ
    div = (param->dim1 * param->dim2 * param->dim3) / bsize;
    mod = (param->dim1 * param->dim2 * param->dim3) % bsize;
    for (i = 0; i < div; i++) {
        cnt += fread(&info->m_values[bsize * i], sizeof (float), bsize, ifs);
    }
    if (mod > 0) {
        cnt += fread(&info->m_values[bsize * div], sizeof (float), mod, ifs);
    }
#else
    {
        Int64 j;
        for (j = 0; j < param->dim1 * param->dim2; j++) {
            i = j * param->dim3;
            cnt += fread(&info->m_values[i], sizeof (float), param->dim3, ifs);
        }
    }
#endif
    if (info->m_nnodes != cnt) {
        LOGOUT(FILTER_ERR, (char*)"[FilterIoAvs::read_variables] read error %d != %d .\n", cnt, info->m_nnodes);
        goto garbage;
    }
#ifndef NO_ENDIAN
    if (info->ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
        for (i = 0; i < info->m_nnodes; i++) {
            info->m_values[i] = ConvertEndianF(info->m_values[i]);
        }
    }
#endif

    blk = step_loop - param->start_step;

    info->m_value_max[blk] = info->m_values[0];
    info->m_value_min[blk] = info->m_values[0];
    for (i = 1; i < info->m_nnodes; i++) {
        if (info->m_value_max[blk] < info->m_values[i]) info->m_value_max[blk] = info->m_values[i];
        if (info->m_value_min[blk] > info->m_values[i]) info->m_value_min[blk] = info->m_values[i];
    }
    state = RTC_OK;
garbage:
    if (NULL != ifs) fclose(ifs);
    return (state);
}

/*****************************************************************************/

/*
 * variablesファイル読込み
 *
 */
int FilterIoAvs::read_variables_2( FILE *ifs, int step_loop )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    int state = RTC_NG;
    int skip;
    int step;
    int blk;
    Int64 cnt;
    Int64 i;
    int bsize;
    int div;
    int mod;

    bsize = param->blocksize;

    /* オープン */
    if (NULL == ifs) {
        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
        goto garbage;
    }

    skip = info->m_nnodes * 4;
    step = step_loop - (info->pre_step + 1);
    if (step > 0) {
    skip = info->m_nnodes * 4 + param->valueskip;
    for (i = 0; i < step; i++) {
        fseek(ifs, skip, SEEK_CUR);
    }
    }
    info->pre_step = step_loop;
    fseek(ifs, param->valueskip, SEEK_CUR);

    /* data */
    cnt = 0;
#ifdef BLOCK_READ
    div = (param->dim1 * param->dim2 * param->dim3) / bsize;
    mod = (param->dim1 * param->dim2 * param->dim3) % bsize;
    for (i = 0; i < div; i++) {
        cnt += fread(&info->m_values[bsize * i], sizeof (float), bsize, ifs);
    }
    if (mod > 0) {
        cnt += fread(&info->m_values[bsize * div], sizeof (float), mod, ifs);
    }
#else
    {
        Int64 j;
        for (j = 0; j < param->dim1 * param->dim2; j++) {
            i = j * param->dim3;
            cnt += fread(&info->m_values[i], sizeof (float), param->dim3, ifs);
        }
    }
#endif
    if (info->m_nnodes != cnt) {
        LOGOUT(FILTER_ERR, (char*)"[FilterIoAvs::read_variables_2] read error %d != %d .\n", cnt, info->m_nnodes);
        goto garbage;
    }
#ifndef NO_ENDIAN
    if (info->ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
        for (i = 0; i < info->m_nnodes; i++) {
            info->m_values[i] = ConvertEndianF(info->m_values[i]);
        }
    }
#endif

    blk = step_loop - param->start_step;

    info->m_value_max[blk] = info->m_values[0];
    info->m_value_min[blk] = info->m_values[0];
    for (i = 1; i < info->m_nnodes; i++) {
        if (info->m_value_max[blk] < info->m_values[i]) info->m_value_max[blk] = info->m_values[i];
        if (info->m_value_min[blk] > info->m_values[i]) info->m_value_min[blk] = info->m_values[i];
    }
    state = RTC_OK;
garbage:
    return (state);
}

/*****************************************************************************/

/*
 * variablesファイル読込み
 *
 */
int FilterIoAvs::read_variables_3( int step_loop )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    int state = RTC_NG;
    int skip;
    int blk;
    Int64 cnt;
    Int64 i;
    int kd;
    int bsize;
    int div;
    int mod;
    int sp;
    char buff[FILELEN_MAX];
    FILE *ifp;
    char *cToken;
    int offset_cnt;
    int stride_cnt;
    int item_cnt;
    char *workc;
    short *works;
    int *worki;
    float *work;
    double *workd;

    bsize = param->blocksize;
    work = (float*) malloc(sizeof (float)*info->m_nnodes);
    memset(work, 0x00, sizeof (float)*info->m_nnodes);

    /* オープン */
    for (kd = 0; kd < info->m_nkinds; kd++) {
        for (sp = info->pre_step + 1; sp <= step_loop; sp++) {
            if (strstr(param->stepf[sp].value[kd].ftype, "ascii")) {
                if (info->m_value_fp[kd] == NULL) {
                    sprintf(info->filename, "%s/%s", param->in_dir, param->stepf[sp].value[kd].fname);
                    /* オープン */
                    ifp = fopen(info->filename, "r");
                    if (NULL == ifp) {
                        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
                        goto garbage;
                    }
                } else {
                    ifp = info->m_value_fp[kd];
                }
                /* とりあえずSKIP行分読み飛ばす */
                for (i = 0; i < param->stepf[sp].value[kd].skip; i++) {
                    if (fgets(buff, LINELEN_MAX, ifp) == NULL) {
                        fclose(ifp);
                        ifp = NULL;
                        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
                        goto garbage;
                    }
                }
                offset_cnt = param->stepf[sp].value[kd].offset;
                stride_cnt = 0;
                item_cnt = 0;
                while (fgets(buff, LINELEN_MAX, ifp)) {
                    /* タブを空白に置き換える */
                    ChangeChar(buff, '\t', ' ');
                    /* オープン */
                    /* 1行に対してtok 1回目 */
                    if ((cToken = strtok(buff, " \n")) != NULL) {
                        if (offset_cnt > 0) {
                            /* offset分は読み飛ばす(最初だけ) */
                            offset_cnt--;
                        } else {
                            /* offset分は読み飛ばされている場合 */
                            if (stride_cnt == 0) {
                                /* 値を取得 */
                                work[item_cnt] = (float) atof(cToken);
                                item_cnt++;
                                if (item_cnt >= info->m_nnodes) {
                                    break;
                                }
                            }
                            stride_cnt++;
                            if (stride_cnt == param->stepf[sp].value[kd].stride) {
                                stride_cnt = 0;
                            }
                        }
                    }
                    /* 1行に対してtok 2回目以降 */
                    while ((cToken = strtok(NULL, " \n")) != NULL) {
                        if (offset_cnt > 0) {
                            /* offset分は読み飛ばす(最初だけ) */
                            offset_cnt--;
                        } else {
                            /* offset分は読み飛ばされている場合 */
                            if (stride_cnt == 0) {
                                /* 値を取得 */
                                work[item_cnt] = (float) atof(cToken);
                                item_cnt++;
                                if (item_cnt >= info->m_nnodes) {
                                    break;
                                }
                            }
                            stride_cnt++;
                            if (stride_cnt == param->stepf[sp].value[kd].stride) {
                                stride_cnt = 0;
                            }
                        }
                    }
                }
            } else {
                if (info->m_value_fp[kd] == NULL) {
                    sprintf(info->filename, "%s/%s", param->in_dir, param->stepf[sp].value[kd].fname);
                    ifp = fopen(info->filename, "rb");
                    if (NULL == ifp) {
                        LOGOUT(FILTER_ERR, (char*)"can not open %s.\n", info->filename);
                        goto garbage;
                    }
                } else {
                    ifp = info->m_value_fp[kd];
                }

                if (sp < step_loop) {
                    if (!strcmp(param->data, "byte")) {
                        skip = param->stepf[sp].value[kd].skip + info->m_nnodes * sizeof (char);
                    } else if (!strcmp(param->data, "short")) {
                        skip = param->stepf[sp].value[kd].skip + info->m_nnodes * sizeof (short);
                    } else if (!strcmp(param->data, "integer")) {
                        skip = param->stepf[sp].value[kd].skip + info->m_nnodes * sizeof (int);
                    } else if (!strcmp(param->data, "float")) {
                        skip = param->stepf[sp].value[kd].skip + info->m_nnodes * sizeof (float);
                    } else if (!strcmp(param->data, "double")) {
                        skip = param->stepf[sp].value[kd].skip + info->m_nnodes * sizeof (double);
                    } else {
                        skip = param->stepf[sp].value[kd].skip + info->m_nnodes;
                    }
                    fseek(ifp, skip, SEEK_CUR);
                } else {
                    fseek(ifp, param->stepf[step_loop].value[kd].skip, SEEK_CUR);
                    /* data */
                    cnt = 0;
                    div = info->m_nnodes / bsize;
                    mod = info->m_nnodes % bsize;

                    if (!strcmp(param->data, "byte")) {
                        workc = (char*) malloc(sizeof (char)*info->m_nnodes);
                        for (i = 0; i < div; i++) {
                            cnt += fread(&workc[bsize * i], sizeof (char), bsize, ifp);
                        }
                        if (mod > 0) {
                            cnt += fread(&workc[bsize * div], sizeof (char), mod, ifp);
                        }
                        for (i = 0; i < info->m_nnodes; i++) {
                            work[i] = (float) (workc[i]);
                        }
                        free(workc);
                    } else if (!strcmp(param->data, "short")) {
                        works = (short*) malloc(sizeof (short)*info->m_nnodes);
                        for (i = 0; i < div; i++) {
                            cnt += fread(&works[bsize * i], sizeof (short), bsize, ifp);
                        }
                        if (mod > 0) {
                            cnt += fread(&works[bsize * div], sizeof (short), mod, ifp);
                        }
                        for (i = 0; i < info->m_nnodes; i++) {
                            work[i] = (float) (works[i]);
                        }
                        free(works);
                    } else if (!strcmp(param->data, "integer")) {
                        worki = (int*) malloc(sizeof (int)*info->m_nnodes);
                        for (i = 0; i < div; i++) {
                            cnt += fread(&worki[bsize * i], sizeof (int), bsize, ifp);
                        }
                        if (mod > 0) {
                            cnt += fread(&worki[bsize * div], sizeof (int), mod, ifp);
                        }
                        for (i = 0; i < info->m_nnodes; i++) {
                            work[i] = (float) (worki[i]);
                        }
                        free(worki);
                    } else if (!strcmp(param->data, "float")) {
                        for (i = 0; i < div; i++) {
                            cnt += fread(&work[bsize * i], sizeof (float), bsize, ifp);
                        }
                        if (mod > 0) {
                            cnt += fread(&work[bsize * div], sizeof (float), mod, ifp);
                        }
                        } else if (!strcmp(param->data, "double")) {
                            workd = (double*) malloc(sizeof (double)*info->m_nnodes);
                        for (i = 0; i < div; i++) {
                            cnt += fread(&workd[bsize * i], sizeof (double), bsize, ifp);
                        }
                        if (mod > 0) {
                            cnt += fread(&workd[bsize * div], sizeof (double), mod, ifp);
                        }
                        for (i = 0; i < info->m_nnodes; i++) {
                            work[i] = (float) (workd[i]);
                        }
                        free(workd);
                    } else {
                        ;
                    }
                    if (info->m_nnodes != cnt) {
                        LOGOUT(FILTER_ERR, (char*)"[FilterIoAvs::read_variables_3] read error %d != %d .\n", cnt, info->m_nnodes);
                        goto garbage;
                    }
#ifndef NO_ENDIAN
                    if (info->ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                        for (i = 0; i < info->m_nnodes; i++) {
                            work[i] = ConvertEndianF(work[i]);
                        }
                    }
#endif
                }
            }

            if (sp == step_loop) {
                blk = step_loop * info->m_nkinds;
                info->m_value_max[blk + kd] = work[0];
                info->m_value_min[blk + kd] = work[0];
                for (i = 1; i < info->m_nnodes; i++) {
                    if (info->m_value_max[blk + kd] < work[i]) info->m_value_max[blk + kd] = work[i];
                    if (info->m_value_min[blk + kd] > work[i]) info->m_value_min[blk + kd] = work[i];
                }
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
                for (i = 0; i < info->m_nnodes; i++) {
                    info->m_values[i * info->m_nkinds + kd] = work[i];
                }
            }
            if (param->stepf[sp].value[kd].close == 1) {
                fclose(ifp);
                info->m_value_fp[kd] = NULL;
            } else {
                info->m_value_fp[kd] = ifp;
            }
        }
    }
    info->pre_step = step_loop;
    state = RTC_OK;
garbage:
    free(work);
    work = NULL;
    return (state);
}


/*
 * 要素タイプの読み込み
 * add by @hira at 2017/04/15
 */
int FilterIoAvs::read_elementtypes( void )
{
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    int rstat = RTC_NG;
    Int64 size = 0;
    int i = 0;
    int multi_count = 0;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;
    Int64 rank_elems;
    Int64 rank_nodes;
    int elem_loop;

    LOGOUT(FILTER_LOG, (char*)"|||| [read_elementtypes] read multi ucd binary data start.\n");
    /* 非構造格子                         */
    /* 節点情報・要素構成情報の読み込み */
    if (info->m_ucdbin) { // read the binary ucd file
        rstat = this->read_ucdbin_geom();
    } else {
        rstat = this->skip_ucd_ascii(1);
        rstat = this->read_ucd_ascii_geom();
        fseek(info->m_ucd_ascii_fp, 0, SEEK_SET);
    }
    if (RTC_OK != rstat) {
        LOGOUT(FILTER_ERR, (char*)"[read_elementtypes] read_ucd_ascii_geom error.\n");
        return RTC_NG;
    }
    LOGOUT(FILTER_LOG, (char*)"|||| [read_elementtypes] read multi ucd binary data end.\n");

    rank_elems = 0;
    multi_count = 0;
    for (i = 0; i < MAX_ELEMENT_TYPE; i++) {
        rank_elems += info->m_types[i];
        if (info->m_types[i] > 0) {
            multi_count++;
        }
    }
    LOGOUT(FILTER_LOG, (char*)"||| [read_elementtypes] rank_elems = %d, multi_count=%d \n", rank_elems, multi_count);

    if (info->m_elems_per_multivolume != NULL) free(info->m_elems_per_multivolume);
    info->m_elems_per_multivolume = (unsigned int *) malloc(sizeof (int)*MAX_ELEMENT_TYPE);
    if (NULL == info->m_elems_per_multivolume) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_elems_per_multivolume).\n");
        return RTC_NG;
    }
    if (info->m_conns_per_multivolume != NULL) free(info->m_conns_per_multivolume);
    info->m_conns_per_multivolume = (unsigned int *) malloc(sizeof (unsigned int)*MAX_ELEMENT_TYPE);
    if (NULL == info->m_conns_per_multivolume) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_conns_per_multivolume).\n");
        return RTC_NG;
    }
    if (info->m_nodes_per_multivolume != NULL) free(info->m_nodes_per_multivolume);
    info->m_nodes_per_multivolume = (unsigned int *) malloc(sizeof (unsigned int)*MAX_ELEMENT_TYPE);
    if (NULL == info->m_nodes_per_multivolume) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodes_per_multivolume).\n");
        return RTC_NG;
    }

    info->prov_len = info->m_nnodes;
    info->prov_len = info->m_nnodes / MAX_ELEMENT_TYPE * 2;

    if (info->m_subvolume_nodes != NULL) free(info->m_subvolume_nodes);
    info->m_subvolume_nodes = (int *) malloc(sizeof (int)*info->prov_len);
    if (NULL == info->m_subvolume_nodes) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_nodes).\n");
        return RTC_NG;
    }
    if (info->m_subvolume_trans != NULL) free(info->m_subvolume_trans);
    info->m_subvolume_trans = (int *) malloc(sizeof (int)*info->prov_len);
    if (NULL == info->m_subvolume_trans) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_trans).\n");
        return RTC_NG;
    }
    info->prov_len_e = rank_elems;
    LOGOUT(FILTER_LOG, (char*)"|||  param->mult_element_type   = %d\n", param->mult_element_type);
    LOGOUT(FILTER_LOG, (char*)"|||  prov_len_e = %d\n", info->prov_len_e);

    if (info->m_subvolume_elems != NULL) free(info->m_subvolume_elems);
    info->m_subvolume_elems = (int *) malloc(sizeof (int)*info->prov_len_e);
    if (NULL == info->m_subvolume_elems) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_elems).\n");
        return RTC_NG;
    }
    if (info->m_nodes_trans != NULL) free(info->m_nodes_trans);
    info->m_nodes_trans = (int *) malloc(sizeof (int)*info->m_nnodes);
    if (NULL == info->m_nodes_trans) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodes_trans).\n");
        return RTC_NG;
    }
    if (info->m_nodeflag != NULL) free(info->m_nodeflag);
    info->m_nodeflag = (char *) malloc(sizeof (char)*info->m_nnodes);
    if (NULL == info->m_nodeflag) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodeflag).\n");
        return RTC_NG;
    }

    rank_nodes = 0;
    rank_elems = 0;
    LOGOUT(FILTER_LOG, (char*)"|||  count_in_elements start.\n");
    for (elem_loop = 0; elem_loop < MAX_ELEMENT_TYPE; elem_loop++) {
        /* サブボリューム内の要素・節点の判別 */
        if (RTC_OK != count_in_elements(
                            elem_loop,
                            &subvolume_elems,
                            &subvolume_nodes,
                            &subvolume_conns,
                            &info->prov_len,
                            rank_nodes,
                            rank_elems,
                            info)) {
            LOGOUT(FILTER_ERR, (char*)"count_in_subvolume error.\n");
            return RTC_NG;
        }
        LOGOUT(FILTER_LOG, (char*)"|||   elem_loop : %7d , multivolume_elems : %7d\n", elem_loop, subvolume_elems);
        info->m_elems_per_multivolume[elem_loop] = subvolume_elems;
        info->m_nodes_per_multivolume[elem_loop] = subvolume_nodes;
        info->m_conns_per_multivolume[elem_loop] = subvolume_conns;
        rank_nodes += info->m_nodes_per_multivolume[elem_loop];
        rank_elems += info->m_elems_per_multivolume[elem_loop];
    }

    info->m_multivolume_nodes = (int *) malloc(sizeof (int)*rank_nodes);
    if (NULL == info->m_multivolume_nodes) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_multivolume_nodes).\n");
        return RTC_NG;
    }
    memcpy(info->m_multivolume_nodes, info->m_subvolume_nodes, sizeof (int)*rank_nodes);
    LOGOUT(FILTER_LOG, (char*)"|||  count_in_elements ended.\n");

    return RTC_OK;
}


/**
 * avs binary:multi要素の読み込みを行う
 * add by @hira at 2017/04/15
 * @param element_type_num		読込要素
 * @return		成否
 */
int FilterIoAvs::read_unstructer_multi_geom(int element_type_num) {
    Int64 i;
    int subvolume_id;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;
    Int64 rank_nodes;
    Int64 rank_elems;
    int elem_loop = 0;
    int rstat = RTC_NG;
    Int64 size = 0;
    int type_count = 0;

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    LOGOUT(FILTER_LOG, (char*)"|||| read multi ucd binary data start.\n");
    /* 非構造格子                         */
    /* 節点情報・要素構成情報の読み込み */
    if (info->m_ucdbin) { // read the binary ucd file
        rstat = this->read_ucdbin_geom();
    } else {
        rstat = this->skip_ucd_ascii(1);
        rstat = this->read_ucd_ascii_geom();
        rstat = this->read_ucd_ascii_value(-1);
        fseek(info->m_ucd_ascii_fp, 0, SEEK_SET);
    }
    if (RTC_OK != rstat) {
        LOGOUT(FILTER_ERR, (char*)"read_ucd_ascii_geom error.\n");
        goto garbage;
    }
    LOGOUT(FILTER_LOG, (char*)"|||| read multi ucd binary data end.\n");


    rank_elems = info->m_types[element_type_num];
    LOGOUT(FILTER_LOG, (char*)"|||  rank_elems = %d\n", rank_elems);

    if (info->m_elems_per_subvolume != NULL) free(info->m_elems_per_subvolume);
    info->m_elems_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    if (NULL == info->m_elems_per_subvolume) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_elems_per_subvolume).\n");
        goto garbage;
    }

    if (info->m_conns_per_subvolume != NULL) free(info->m_conns_per_subvolume);
    info->m_conns_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    if (NULL == info->m_conns_per_subvolume) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_conns_per_subvolume).\n");
        goto garbage;
    }
    if (info->m_nodes_per_subvolume != NULL) free(info->m_nodes_per_subvolume);
    info->m_nodes_per_subvolume = (unsigned int *) malloc(sizeof (unsigned int)*info->m_nvolumes);
    if (NULL == info->m_nodes_per_subvolume) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodes_per_subvolume).\n");
        goto garbage;
    }

    info->prov_len = info->m_nnodes;
    info->prov_len = info->m_nnodes / MAX_ELEMENT_TYPE * 2;

    LOGOUT(FILTER_LOG, (char*)"|||  prov_len = %d\n", info->prov_len);

    if (info->m_subvolume_nodes != NULL) free(info->m_subvolume_nodes);
    info->m_subvolume_nodes = (int *) malloc(sizeof (int)*info->prov_len);
    if (NULL == info->m_subvolume_nodes) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_nodes).\n");
        goto garbage;
    }
    if (info->m_subvolume_trans != NULL) free(info->m_subvolume_trans);
    info->m_subvolume_trans = (int *) malloc(sizeof (int)*info->prov_len);
    if (NULL == info->m_subvolume_trans) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_trans).\n");
        goto garbage;
    }
    info->prov_len_e = rank_elems;
    LOGOUT(FILTER_LOG, (char*)"|||  param->mult_element_type   = %d\n", param->mult_element_type);
    LOGOUT(FILTER_LOG, (char*)"|||  prov_len_e = %d\n", info->prov_len_e);

    if (info->m_subvolume_elems != NULL) free(info->m_subvolume_elems);
    info->m_subvolume_elems = (int *) malloc(sizeof (int)*info->prov_len_e);
    if (NULL == info->m_subvolume_elems) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_subvolume_elems).\n");
        goto garbage;
    }
    if (info->m_nodes_trans != NULL) free(info->m_nodes_trans);
    info->m_nodes_trans = (int *) malloc(sizeof (int)*info->m_nnodes);
    if (NULL == info->m_nodes_trans) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodes_trans).\n");
        goto garbage;
    }
    if (info->m_nodeflag != NULL) free(info->m_nodeflag);
    info->m_nodeflag = (char *) malloc(sizeof (char)*info->m_nnodes);
    if (NULL == info->m_nodeflag) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_nodeflag).\n");
        goto garbage;
    }

    for (i = 0; i < info->m_nvolumes; i++) {
        //      m_elems_per_subvolume[i] = 0;
        info->m_conns_per_subvolume[i] = 0;
        info->m_nodes_per_subvolume[i] = 0;
    }
    for (i = 0; i < info->m_nnodes; i++) {
        info->m_nodeflag[i] = 0;
    }

    rank_nodes = 0;
    rank_elems = 0;

    LOGOUT(FILTER_LOG, (char*)"|||  count_in_elements start [element_type=%d].\n", element_type_num);
    /* サブボリューム内の要素・節点の判別 */
    if (RTC_OK != count_in_elements(
                        element_type_num,
                        &subvolume_elems,
                        &subvolume_nodes,
                        &subvolume_conns,
                        &info->prov_len,
                        rank_nodes,
                        rank_elems,
                        info)) {
        LOGOUT(FILTER_ERR, (char*)"count_in_subvolume error.\n");
        goto garbage;
    }
    LOGOUT(FILTER_LOG, (char*)"|||   elem_loop : %7d , subvolume_elems : %7d\n", elem_loop, subvolume_elems);
    info->m_elems_per_subvolume[elem_loop] = subvolume_elems;
    info->m_nodes_per_subvolume[elem_loop] = subvolume_nodes;
    info->m_conns_per_subvolume[elem_loop] = subvolume_conns;
    rank_nodes += info->m_nodes_per_subvolume[elem_loop];
    rank_elems += info->m_elems_per_subvolume[elem_loop];

    LOGOUT(FILTER_LOG, (char*)"|||  count_in_elements ended.\n");

    info->m_nsteps = param->end_step - param->start_step + 1;
    if (info->m_value_max != NULL) free(info->m_value_max);
    info->m_value_max = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->m_value_max) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_value_min).\n");
        goto garbage;
    }
    if (info->m_value_min != NULL) free(info->m_value_min);
    info->m_value_min = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->m_value_min) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(m_value_min).\n");
        goto garbage;
    }
    if (info->mt_nulldata != NULL) free(info->mt_nulldata);
    info->mt_nulldata = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->mt_nulldata) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mt_nulldata).\n");
        goto garbage;
    }
    if (info->mt_nullflag != NULL) free(info->mt_nullflag);
    info->mt_nullflag = (int *) malloc(sizeof (int)*info->m_nkinds * info->m_nsteps);
    if (NULL == info->mt_nullflag) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(mt_nullflag).\n");
        goto garbage;
    }
    for (i = 0; i < info->m_nkinds * info->m_nsteps; i++) {
        info->m_value_max[i] = 0.0;
        info->m_value_min[i] = 0.0;
    }
    if (info->m_values != NULL) free(info->m_values);
    info->m_values = (float *) malloc(sizeof (float)*info->m_nkinds * info->m_nnodes);
    size = info->getNodeCount();
    // info->m_values = (float *) malloc(sizeof (float)*size);
    if (NULL == info->m_values) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory(values).\n");
        goto garbage;
    }
    if (info->m_nelemcomponents > 0) {
        if (info->m_elem_in_node != NULL) free(info->m_elem_in_node);
        if (info->m_elem_in_node_addr != NULL) free(info->m_elem_in_node_addr);
        info->m_elem_in_node = (int *) malloc(sizeof (int)*info->m_nnodes);
        info->m_elem_in_node_addr = (int *) malloc(sizeof (int)*info->m_nnodes);
        memset(info->m_elem_in_node, 0x00, sizeof (float)*info->m_nnodes);
        LOGOUT(FILTER_LOG, (char*)"|||  set_element_in_node start.\n");
        if (RTC_OK != set_element_in_node(info)) {
            LOGOUT(FILTER_ERR, (char*)"set_element_in_node error.\n");
            goto garbage;
        }
        LOGOUT(FILTER_LOG, (char*)"|||  set_element_in_node ended.\n");
    }

    rank_nodes = 0;
    rank_elems = 0;
    type_count = 0;
    for (elem_loop = 0; elem_loop < MAX_ELEMENT_TYPE; elem_loop++) {
        if (info->m_types[elem_loop] == 0) continue;
        if (elem_loop != element_type_num) {
            continue;
        }
        subvolume_id = info->m_tree_node_no - info->m_nvolumes + elem_loop;
        /* サブボリューム内の要素・節点の判別 */
        subvolume_elems = info->m_elems_per_subvolume[type_count];
        subvolume_nodes = info->m_nodes_per_subvolume[type_count];
        subvolume_conns = info->m_conns_per_subvolume[type_count];
        /* サブボリューム内の要素・節点の判別 */
        if (RTC_OK != set_nodes_in_subvolume(
                            subvolume_nodes,
                            rank_nodes,
                            info)) {
            LOGOUT(FILTER_ERR, (char*)"set_node_in_subvolume error.\n");
            goto garbage;
        }

        if (info->m_types[elem_loop] != 0) {
            if (RTC_OK != this->write_ucdbin_data(-1,
                    elem_loop,
                    subvolume_elems,
                    subvolume_nodes,
                    rank_nodes,
                    rank_elems)) {
                LOGOUT(FILTER_ERR, (char*)"write_ucdbin error.\n");
                goto garbage;
            }
        }
        rank_nodes += info->m_nodes_per_subvolume[type_count];
        rank_elems += info->m_elems_per_subvolume[type_count];
        break;
    }

    rstat = RTC_OK;
    garbage:
    subvolume_elems = 0;
    subvolume_nodes = 0;
    subvolume_conns = 0;
    rank_nodes = 0;
    rank_elems = 0;

    return (rstat);
}


/**
 * avs binary:multi要素から物理量データの読み込みを行う
 * add by @hira at 2017/04/15
 * @param element_type_num		読込要素
 * @return		成否
 */
int FilterIoAvs::read_unstructer_multi_value(int element_type_num, int step_loop) {
    Int64 i;
    int subvolume_id;
    Int64 subvolume_elems;
    Int64 subvolume_nodes;
    Int64 subvolume_conns;
    Int64 rank_nodes;
    Int64 rank_elems;
    Int64 rank_conns;
    int elem_loop = 0;
    int rstat = RTC_NG;
    Int64 size = 0;
    int type_count = 0;
    Int64 total_nodes = 0;

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;

    // 要素タイプを指定して読み込みを行う  add by @hira at 2017/04/15
    if (RTC_OK != this->read_unstructer_multi_geom(element_type_num)) {
        LOGOUT(FILTER_ERR, (char*)"read_unstructer_multi_value->read unstructure multi error.[element_type_num=%d]\n", element_type_num);
        goto garbage;
    }

    rank_nodes = 0;
    rank_elems = 0;
    rank_conns = 0;
    for (elem_loop = 0; elem_loop < MAX_ELEMENT_TYPE; elem_loop++) {
        if (info->m_types[elem_loop] == 0) continue;
        rank_elems += info->m_elems_per_multivolume[elem_loop];
        rank_nodes += info->m_nodes_per_multivolume[elem_loop];
        rank_conns += info->m_conns_per_multivolume[elem_loop];
    }
    // 一時的に全体サイズとする
    info->m_nnodes = rank_nodes;
    info->m_nelements = rank_elems;

    LOGOUT(FILTER_LOG, (char*)"<><> time_step %d start.\n", step_loop);
    if (strlen(param->ucd_inp) != 0) {
        if (info->m_ucdbin == 0) {
            rstat = this->skip_ucd_ascii(step_loop);
            if (info->m_cycle_type == 1) {
                rstat = this->skip_ucd_ascii_geom();
            } else if (info->m_cycle_type == 2) {
                if (step_loop == param->start_step) {
                    rstat = this->skip_ucd_ascii_geom();
                }
            }
            rstat = this->read_ucd_ascii_value(step_loop);
        } else {
            rstat = this->read_ucdbin_data(step_loop);
        }
    } else {
        rstat = this->read_ucdbin_data(step_loop);
    }
    if (RTC_OK != rstat) {
        LOGOUT(FILTER_ERR, (char*)"read_ucdbin_data error.\n");
        goto garbage;
    }
    rank_nodes = 0;
    rank_elems = 0;
    rank_conns = 0;
    type_count = 0;
    for (elem_loop = 0; elem_loop < MAX_ELEMENT_TYPE; elem_loop++) {
        if (info->m_types[elem_loop] == 0) continue;
        subvolume_id = info->m_tree_node_no - info->m_nvolumes + elem_loop;
        /* サブボリューム内の要素・節点の判別 */
        subvolume_elems = info->m_elems_per_multivolume[elem_loop];
        subvolume_nodes = info->m_nodes_per_multivolume[elem_loop];
        subvolume_conns = info->m_conns_per_multivolume[elem_loop];

        if (elem_loop == element_type_num) {
            if (info->m_types[elem_loop] != 0) {
                // value
                float *value = (float *)malloc(sizeof(float) * info->m_nkinds*subvolume_nodes);
                memset(value, 0x00, sizeof(float) * info->m_nkinds*subvolume_nodes);
                int count = 0;
                for(i=0;i<subvolume_nodes;i++){
                    int id = info->m_multivolume_nodes[rank_nodes+i];
                    int j = id*info->m_nkinds;
                    int k = 0;
                    while(k<info->m_nkinds){
                        value[count++] = info->m_values[j+k];
                        k++;
                    }
                }
                // 自要素タイプのサイズに戻す
                info->m_nnodes = subvolume_nodes;
                info->m_nelements = subvolume_elems;
                memcpy(info->m_values, value, sizeof(float) * info->m_nkinds*subvolume_nodes);
                free(value);
                value = NULL;
            }
            break;
        }

        rank_nodes += info->m_nodes_per_multivolume[elem_loop];
        rank_elems += info->m_elems_per_multivolume[elem_loop];
    }

    rstat = RTC_OK;
garbage:
    subvolume_elems = 0;
    subvolume_nodes = 0;
    subvolume_conns = 0;
    rank_nodes = 0;
    rank_elems = 0;

    return (rstat);
}

} /* namespace filter */
} /* namespace pbvr */
