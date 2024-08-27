#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "../filter_log.h"
#include "../filter_convert.h"

static int alloc_stepfileinfo(void);

static int ChangeChar(char Text[], char Chr1, char Chr2) {
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
int read_fld_file(void) {
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
    FilterParam *param;
    char flag0 = 0;
    char flag1 = 0;
    char flag2 = 0;
    int count;
    int do_step;
    char alloc_flag;

    count = 0;
    do_step = -1;
    alloc_flag = 0;

    param = &Fparam;
    sprintf(filename, "%s/%s", Fparam.in_dir, Fparam.field_file);
    ifs = fopen(filename, "r");
    if (ifs == NULL) {
	printf("Can't open file : %s\n", filename);
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
	    filter_log(FILTER_ERR, "Not support file_type=ascii .\n");
	    return state;
	}
	if (strstr(line, "filetype=unformatted")) {
	    filter_log(FILTER_ERR, "Not support file_type=unformatted .\n");
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

    filter_log(FILTER_LOG, "param->nstep    : %d\n", param->nstep);
    filter_log(FILTER_LOG, "param->ndim     : %d\n", param->ndim);
    filter_log(FILTER_LOG, "param->dim1     : %d\n", param->dim1);
    filter_log(FILTER_LOG, "param->dim2     : %d\n", param->dim2);
    filter_log(FILTER_LOG, "param->dim3     : %d\n", param->dim3);
    filter_log(FILTER_LOG, "param->nspace   : %d\n", param->nspace);
    filter_log(FILTER_LOG, "param->veclen   : %d\n", param->veclen);
    filter_log(FILTER_LOG, "param->data     : %s\n", param->data);
    filter_log(FILTER_LOG, "param->field    : %s\n", param->field);
    for (i = 0; i < 3; i++) {
	filter_log(FILTER_LOG, "param->coordfile   : %s\n", param->coordfile[i]);
	filter_log(FILTER_LOG, "param->coordtype   : %s\n", param->coordtype[i]);
	filter_log(FILTER_LOG, "param->coordskip   : %d\n", param->coordskip[i]);
	filter_log(FILTER_LOG, "param->coordstride : %d\n", param->coordstride[i]);
    }
    filter_log(FILTER_LOG, "param->valuefile   : %s\n", param->valuefile);
    filter_log(FILTER_LOG, "param->valuetype   : %s\n", param->valuetype);
    filter_log(FILTER_LOG, "param->valueskip   : %d\n", param->valueskip);
    filter_log(FILTER_LOG, "do_step            : %d\n", do_step);
    filter_log(FILTER_LOG, "param->min_ext[0]  : %f\n", param->min_ext[0]);
    filter_log(FILTER_LOG, "param->min_ext[1]  : %f\n", param->min_ext[1]);
    filter_log(FILTER_LOG, "param->min_ext[2]  : %f\n", param->min_ext[2]);
    filter_log(FILTER_LOG, "param->max_ext[0]  : %f\n", param->max_ext[0]);
    filter_log(FILTER_LOG, "param->max_ext[1]  : %f\n", param->max_ext[1]);
    filter_log(FILTER_LOG, "param->max_ext[2]  : %f\n", param->max_ext[2]);

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
	    filter_log(FILTER_LOG, "param->stepf[%3d].coord[%2d].fname : %s\n", i, j, param->stepf[i].coord[j].fname);
	    filter_log(FILTER_LOG, "param->stepf[%3d].coord[%2d].ftype : %s\n", i, j, param->stepf[i].coord[j].ftype);
	    filter_log(FILTER_LOG, "param->stepf[%3d].coord[%2d].skip  : %d\n", i, j, param->stepf[i].coord[j].skip);
	    filter_log(FILTER_LOG, "param->stepf[%3d].coord[%2d].offset: %d\n", i, j, param->stepf[i].coord[j].offset);
	    filter_log(FILTER_LOG, "param->stepf[%3d].coord[%2d].stride: %d\n", i, j, param->stepf[i].coord[j].stride);
	    filter_log(FILTER_LOG, "param->stepf[%3d].coord[%2d].close : %d\n", i, j, param->stepf[i].coord[j].close);
	}
	for (j = 0; j < param->veclen; j++) {
	    filter_log(FILTER_LOG, "param->stepf[%3d].value[%2d].fname : %s\n", i, j, param->stepf[i].value[j].fname);
	    filter_log(FILTER_LOG, "param->stepf[%3d].value[%2d].ftype : %s\n", i, j, param->stepf[i].value[j].ftype);
	    filter_log(FILTER_LOG, "param->stepf[%3d].value[%2d].skip  : %d\n", i, j, param->stepf[i].value[j].skip);
	    filter_log(FILTER_LOG, "param->stepf[%3d].value[%2d].offset: %d\n", i, j, param->stepf[i].value[j].offset);
	    filter_log(FILTER_LOG, "param->stepf[%3d].value[%2d].stride: %d\n", i, j, param->stepf[i].value[j].stride);
	    filter_log(FILTER_LOG, "param->stepf[%3d].value[%2d].close : %d\n", i, j, param->stepf[i].value[j].close);
	}
    }

    //  if((param->start_step==0)&&(param->end_step==0)){
    //      param->end_step = param->nstep - 1;
    //  }

    if (param->ndim == 2) {
	m_nnodes = Fparam.dim1 * Fparam.dim2;
	m_nelements = (Fparam.dim1 - 1)*(Fparam.dim2 - 1);
	m_elemcoms = 4;
	m_element_type = 3;
    } else if (param->ndim == 3) {
	m_nnodes = Fparam.dim1 * Fparam.dim2 * Fparam.dim3;
	m_nelements = (Fparam.dim1 - 1)*(Fparam.dim2 - 1)*(Fparam.dim3 - 1);
	m_elemcoms = 8;
	m_element_type = 7;
    } else {
	filter_log(FILTER_ERR, "Not support dimension : %d\n", param->ndim);
	return (RTC_NG);
    }
    m_connectsize = m_nelements*m_elemcoms;
    m_ncomponents = param->veclen;
    m_nkinds = param->veclen;
    m_desctype = 1;
    m_datatype = 1;

    state = RTC_OK;
    return (state);
}


/*****************************************************************************/

/*
 * 形状ファイル読込み
 *
 */
int alloc_stepfileinfo(void) {
    int i, j;
    Fdetail *fdc;
    Fdetail *fdv;
    Stepfile *sf;
    FilterParam *param = &Fparam;

    sf = (Stepfile*) malloc(sizeof (Stepfile) * param->nstep);
    if (NULL == sf) {
	filter_log(FILTER_ERR, "can not allocate memory(sf).\n");
	goto garbage;
    }
    for (i = 0; i < param->nstep; i++) {
	fdc = (Fdetail*) malloc(sizeof (Fdetail) * param->nspace);
	fdv = (Fdetail*) malloc(sizeof (Fdetail) * param->veclen);
	for (j = 0; j < param->nspace; j++) {
	    memset(fdc[j].fname, 0x00, LINELEN_MAX);
	    fdc[j].skip = 0;
	    fdc[j].offset = 0;
	    fdc[j].stride = 1;
	    fdc[j].close = 1;
	}
	for (j = 0; j < param->veclen; j++) {
	    memset(fdv[j].fname, 0x00, LINELEN_MAX);
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
int read_geom( void )
{
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

    sprintf(filename, "%s/%s", Fparam.in_dir, Fparam.geom_file);

    /* 1時刻目のデータから節点情報を読み込む */

    /* オープン */
    ifs = fopen(filename, "rb");
    if (NULL == ifs) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
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
	m_title[i] = descript[i];
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
    if (ENDIAN) {
	dimx = ConvertEndianI(dimx);
	dimy = ConvertEndianI(dimy);
	dimz = ConvertEndianI(dimz);
    }

    coordx = (float*) malloc(sizeof (float)*m_nnodes);
    coordy = (float*) malloc(sizeof (float)*m_nnodes);
    coordz = (float*) malloc(sizeof (float)*m_nnodes);
    if (NULL == coordz) {
	filter_log(FILTER_ERR, "can not allocate memory(coord) %s.\n", filename);
	goto garbage;
    }

    fread(coordx, sizeof (float), m_nnodes, ifs);
    fread(coordy, sizeof (float), m_nnodes, ifs);
    fread(coordz, sizeof (float), m_nnodes, ifs);

    for (i = 0; i < (size_t)m_nnodes; i += 1) {
	m_ids[i] = i + 1;
	if (ENDIAN) {
	    coordx[i] = ConvertEndianF(coordx[i]);
	    coordy[i] = ConvertEndianF(coordy[i]);
	    coordz[i] = ConvertEndianF(coordz[i]);
	}
	m_coords[3 * i ] = coordx[i];
	m_coords[3 * i + 1] = coordy[i];
	m_coords[3 * i + 2] = coordz[i];
    }

    /* 最大・最小を取得 */
    min.x = max.x = m_coords[0];
    min.y = max.y = m_coords[1];
    min.z = max.z = m_coords[2];
    for (i = 1; i < dimx * dimy * dimz; i++) {
	if (min.x > m_coords[i * 3 ]) min.x = m_coords[i * 3];
	if (min.y > m_coords[i * 3 + 1]) min.y = m_coords[i * 3 + 1];
	if (min.z > m_coords[i * 3 + 2]) min.z = m_coords[i * 3 + 2];

	if (max.x < m_coords[i * 3 ]) max.x = m_coords[i * 3];
	if (max.y < m_coords[i * 3 + 1]) max.y = m_coords[i * 3 + 1];
	if (max.z < m_coords[i * 3 + 2]) max.z = m_coords[i * 3 + 2];
    }

    if (mype_f == 0) filter_log(FILTER_LOG, "  max -> %8.2f, %8.2f, %8.2f\n", max.x, max.y, max.z);
    if (mype_f == 0) filter_log(FILTER_LOG, "  min -> %8.2f, %8.2f, %8.2f\n", min.x, min.y, min.z);

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
int read_coord( void )
{
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

    bsize = Fparam.blocksize;
    dimx = Fparam.dim1;
    dimy = Fparam.dim2;
    dimz = Fparam.dim3;

    if (!strcmp(Fparam.coordfile[0], Fparam.coordfile[1]) &&
	    !strcmp(Fparam.coordfile[0], Fparam.coordfile[2]) &&
	    Fparam.coordstride[0] == 3 && Fparam.coordstride[1] == 3 && Fparam.coordstride[2] == 3 &&
	    Fparam.coordskip[0] == 0 && Fparam.coordskip[1] == 4 && Fparam.coordskip[2] == 8) {
	sprintf(filename, "%s/%s", Fparam.in_dir, Fparam.coordfile[0]);
	/* オープン */
	ifs = fopen(filename, "rb");
	if (NULL == ifs) {
	    filter_log(FILTER_ERR, "can not open %s.\n", filename);
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
	for (j = 0; j < Fparam.ndim; j++) {
	    sprintf(filename, "%s/%s", Fparam.in_dir, Fparam.coordfile[j]);
	    /* オープン */
	    ifs = fopen(filename, "rb");
	    if (NULL == ifs) {
		filter_log(FILTER_ERR, "can not open %s.\n", filename);
		goto garbage;
	    }
	    skip = Fparam.coordskip[j];
	    stride = (Fparam.coordstride[j] - 1)*4;
	    fseek(ifs, skip, SEEK_CUR);
	    fread(&m_coords[j], 4, 1, ifs);
	    for (i = 1; i < m_nnodes; i += 1) {
		fseek(ifs, stride, SEEK_CUR);
		fread(&m_coords[i * 3 + j], 4, 1, ifs);
	    }
	    fclose(ifs);
	    ifs = NULL;
	}
    }
#ifndef NO_ENDIAN
    if (ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
	for (i = 0; i < dimx * dimy * dimz * 3; i += 1) {
	    m_coords[i] = ConvertEndianF(m_coords[i]);
	}
    }
#endif

    /* 最大・最小を取得 */
    min.x = max.x = m_coords[0];
    min.y = max.y = m_coords[1];
    min.z = max.z = m_coords[2];
    for (i = 1; i < dimx * dimy * dimz; i++) {
	if (min.x > m_coords[i * 3 ]) min.x = m_coords[i * 3];
	if (min.y > m_coords[i * 3 + 1]) min.y = m_coords[i * 3 + 1];
	if (min.z > m_coords[i * 3 + 2]) min.z = m_coords[i * 3 + 2];

	if (max.x < m_coords[i * 3 ]) max.x = m_coords[i * 3];
	if (max.y < m_coords[i * 3 + 1]) max.y = m_coords[i * 3 + 1];
	if (max.z < m_coords[i * 3 + 2]) max.z = m_coords[i * 3 + 2];
    }

    if (mype_f == 0) filter_log(FILTER_LOG, "  max -> %8.2f, %8.2f, %8.2f\n", max.x, max.y, max.z);
    if (mype_f == 0) filter_log(FILTER_LOG, "  min -> %8.2f, %8.2f, %8.2f\n", min.x, min.y, min.z);

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
int read_coord_1( int step )
{
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

    bsize = Fparam.blocksize;
    dimx = Fparam.dim1;
    dimy = Fparam.dim2;
    dimz = Fparam.dim3;

    if (!strcmp(Fparam.stepf[step].coord[0].fname, Fparam.stepf[step].coord[1].fname) &&
	    !strcmp(Fparam.stepf[step].coord[0].fname, Fparam.stepf[step].coord[2].fname) &&
	    strstr(Fparam.stepf[step].coord[0].ftype, "binary") &&
	    strstr(Fparam.stepf[step].coord[1].ftype, "binary") &&
	    strstr(Fparam.stepf[step].coord[2].ftype, "binary") &&
	    Fparam.stepf[step].coord[0].stride == 3 &&
	    Fparam.stepf[step].coord[1].stride == 3 &&
	    Fparam.stepf[step].coord[2].stride == 3 &&
	    Fparam.stepf[step].coord[0].skip == 0 &&
	    Fparam.stepf[step].coord[1].skip == 4 &&
	    Fparam.stepf[step].coord[2].skip == 8 &&
	    !strcmp(Fparam.data, "float")) {
	sprintf(filename, "%s/%s", Fparam.in_dir, Fparam.stepf[step].coord[0].fname);
	/* オープン */
	ifs = fopen(filename, "rb");
	if (NULL == ifs) {
	    filter_log(FILTER_ERR, "can not open %s.\n", filename);
	    goto garbage;
	}
	m_geom_fp[0] = m_geom_fp[1] = m_geom_fp[2] = ifs;
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
	if (ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
	    for (i = 0; i < dimx * dimy * dimz * 3; i += 1) {
		m_coords[i] = ConvertEndianF(m_coords[i]);
	    }
	}
#endif
	if (Fparam.stepf[step].coord[0].close == 1) {
	    fclose(ifs);
	    ifs = NULL;
	    m_geom_fp[0] = m_geom_fp[1] = m_geom_fp[2] = NULL;
	}
    } else {
	work = (float*) malloc(sizeof (float)*m_nnodes);
	for (dm = 0; dm < Fparam.nspace; dm++) {
	    sprintf(filename, "%s/%s", Fparam.in_dir, Fparam.stepf[step].coord[dm].fname);
	    if (strstr(Fparam.stepf[step].coord[dm].ftype, "ascii")) {
		if (m_geom_fp[dm] == NULL) {
		    /* オープン */
		    ifs = fopen(filename, "r");
		    if (NULL == ifs) {
			filter_log(FILTER_ERR, "can not open %s.\n", filename);
			goto garbage;
		    }
		} else {
		    ifs = m_geom_fp[dm];
		}
		max_item = 0;
		if (strstr(Fparam.field, "irregular")) {
		    max_item = m_nnodes;
		} else {
		    if (dm == 0) max_item = Fparam.dim1;
		    if (dm == 1) max_item = Fparam.dim2;
		    if (dm == 2) max_item = Fparam.dim3;
		}
		/* とりあえずSKIP行分読み飛ばす */
		for (i = 0; i < Fparam.stepf[step].coord[dm].skip; i++) {
		    if (fgets(buff, LINELEN_MAX, ifs) == NULL) {
			fclose(ifs);
			ifs = NULL;
			filter_log(FILTER_ERR, "can not open %s.\n", filename);
			goto garbage;
		    }
		}
		offset_cnt = Fparam.stepf[step].coord[dm].offset;
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
			    if (stride_cnt == Fparam.stepf[step].coord[dm].stride) {
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
			    if (stride_cnt == Fparam.stepf[step].coord[dm].stride) {
				stride_cnt = 0;
			    }
			}
		    }
		}
	    } else {
		/* オープン */
		if (m_geom_fp[dm] == NULL) {
		    ifs = fopen(filename, "rb");
		    if (NULL == ifs) {
			filter_log(FILTER_ERR, "can not open %s.\n", filename);
			goto garbage;
		    }
		} else {
		    ifs = m_geom_fp[dm];
		}
		if (!strcmp(Fparam.data, "byte")) {
		    tsize = sizeof (char);
		} else if (!strcmp(Fparam.data, "short")) {
		    tsize = sizeof (short);
		} else if (!strcmp(Fparam.data, "int")) {
		    tsize = sizeof (int);
		} else if (!strcmp(Fparam.data, "float")) {
		    tsize = sizeof (float);
		} else if (!strcmp(Fparam.data, "double")) {
		    tsize = sizeof (double);
		} else {
		    tsize = 1;
		}
		skip = Fparam.stepf[step].coord[dm].skip;
		stride = (Fparam.stepf[step].coord[dm].stride - 1) * tsize;
		fseek(ifs, skip, SEEK_CUR);
		if (!strcmp(Fparam.data, "byte")) {
		    workc = (char*) malloc(sizeof (char)*m_nnodes);
		    fread(&workc[0], tsize, 1, ifs);
		    for (i = 1; i < m_nnodes; i += 1) {
			fseek(ifs, stride, SEEK_CUR);
			fread(&workc[i], tsize, 1, ifs);
		    }
		    for (i = 0; i < m_nnodes; i += 1) {
			work[i] = (float) workc[i];
		    }
		    free(workc);
		} else if (!strcmp(Fparam.data, "short")) {
		    works = (short*) malloc(sizeof (short)*m_nnodes);
		    fread(&works[0], tsize, 1, ifs);
		    for (i = 1; i < m_nnodes; i += 1) {
			fseek(ifs, stride, SEEK_CUR);
			fread(&works[i], tsize, 1, ifs);
		    }
		    for (i = 0; i < m_nnodes; i += 1) {
			work[i] = (float) works[i];
		    }
		    free(works);
		} else if (!strcmp(Fparam.data, "int")) {
		    worki = (int*) malloc(sizeof (int)*m_nnodes);
		    fread(&worki[0], tsize, 1, ifs);
		    for (i = 1; i < m_nnodes; i += 1) {
			fseek(ifs, stride, SEEK_CUR);
			fread(&worki[i], tsize, 1, ifs);
		    }
		    for (i = 0; i < m_nnodes; i += 1) {
			work[i] = (float) worki[i];
		    }
		    free(worki);
		} else if (!strcmp(Fparam.data, "float")) {
		    fread(&work[0], tsize, 1, ifs);
		    for (i = 1; i < m_nnodes; i += 1) {
			fseek(ifs, stride, SEEK_CUR);
			fread(&work[i], tsize, 1, ifs);
		    }
		} else if (!strcmp(Fparam.data, "double")) {
		    workd = (double*) malloc(sizeof (double)*m_nnodes);
		    fread(&workd[0], tsize, 1, ifs);
		    for (i = 1; i < m_nnodes; i += 1) {
			fseek(ifs, stride, SEEK_CUR);
			fread(&workd[i], tsize, 1, ifs);
		    }
		    for (i = 0; i < m_nnodes; i += 1) {
			work[i] = (float) workd[i];
		    }
		    free(workd);
		} else {
		    tsize = 1;
		}
#ifndef NO_ENDIAN
		if (ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		    for (i = 0; i < m_nnodes; i++) {
			work[i] = ConvertEndianF(work[i]);
		    }
		}
#endif
	    }
	    if (Fparam.stepf[step].coord[dm].close == 1) {
		fclose(ifs);
		ifs = NULL;
		m_geom_fp[dm] = NULL;
	    }
	    if (strstr(Fparam.field, "irregular")) {
		for (i = 0; i < m_nnodes; i++) {
		    m_coords[i * 3 + dm] = work[i];
		}
	    } else {
		int ii, jj, kk, id;
		if (dm == 0) {
		    for (kk = 0; kk < Fparam.dim3; kk++) {
			for (jj = 0; jj < Fparam.dim2; jj++) {
			    for (ii = 0; ii < Fparam.dim1; ii++) {
				id = ii + (Fparam.dim1) * jj + (Fparam.dim1)*(Fparam.dim2) * kk;
				m_coords[id * 3 + dm] = work[ii];
			    }
			}
		    }
		} else if (dm == 1) {
		    for (kk = 0; kk < Fparam.dim3; kk++) {
			for (jj = 0; jj < Fparam.dim2; jj++) {
			    for (ii = 0; ii < Fparam.dim1; ii++) {
				id = ii + (Fparam.dim1) * jj + (Fparam.dim1)*(Fparam.dim2) * kk;
				m_coords[id * 3 + dm] = work[jj];
			    }
			}
		    }
		} else {
		    for (kk = 0; kk < Fparam.dim3; kk++) {
			for (jj = 0; jj < Fparam.dim2; jj++) {
			    for (ii = 0; ii < Fparam.dim1; ii++) {
				id = ii + (Fparam.dim1) * jj + (Fparam.dim1)*(Fparam.dim2) * kk;
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
    min.x = max.x = m_coords[0];
    min.y = max.y = m_coords[1];
    min.z = max.z = m_coords[2];
    for (i = 1; i < dimx * dimy * dimz; i++) {
	if (min.x > m_coords[i * 3 ]) min.x = m_coords[i * 3];
	if (min.y > m_coords[i * 3 + 1]) min.y = m_coords[i * 3 + 1];
	if (min.z > m_coords[i * 3 + 2]) min.z = m_coords[i * 3 + 2];

	if (max.x < m_coords[i * 3 ]) max.x = m_coords[i * 3];
	if (max.y < m_coords[i * 3 + 1]) max.y = m_coords[i * 3 + 1];
	if (max.z < m_coords[i * 3 + 2]) max.z = m_coords[i * 3 + 2];
    }

    if (mype_f == 0) filter_log(FILTER_LOG, "  max -> %8.2f, %8.2f, %8.2f\n", max.x, max.y, max.z);
    if (mype_f == 0) filter_log(FILTER_LOG, "  min -> %8.2f, %8.2f, %8.2f\n", min.x, min.y, min.z);

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
int read_variables_per_step(int step_loop)
{
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
   
    bsize = Fparam.blocksize;
    sprintf(buff, Fparam.format, step_loop);
    sprintf(filename, "%s/%s%s%s", Fparam.in_dir, Fparam.in_prefix, buff, Fparam.in_suffix);

    /* オープン */
    ifs = fopen(filename, "rb");
    if (NULL == ifs) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }

    /* description line 1 */
    fread(descript, 80, 1, ifs);
    /* description (part) */
    fread(descript, 80, 1, ifs);
    /* part */
    fread(&part, 4, 1, ifs);
    if (ENDIAN) part = ConvertEndianI(part);
    /* block */
    fread(descript, 80, 1, ifs);
    /* data */
#ifdef BLOCK_READ
    div = m_nnodes / bsize;
    mod = m_nnodes % bsize;
    cnt = 0;
    for (i = 0; i < div; i++) {
	cnt += fread(&m_values[bsize * i], sizeof (float), bsize, ifs);
    }
    if (mod > 0) {
	cnt += fread(&m_values[bsize * div], sizeof (float), mod, ifs);
    }
#else
    cnt = fread(m_values, sizeof (float), m_nnodes, ifs);
#endif
    if (m_nnodes != cnt) {
	filter_log(FILTER_ERR, "read error %d != %d .\n", cnt, m_nnodes);
	goto garbage;
    }
    if (ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
	for (i = 0; i < m_nnodes; i++) {
	    m_values[i] = ConvertEndianF(m_values[i]);
	}
    }

    blk = step_loop - Fparam.start_step;

    m_value_max[blk] = m_values[0];
    m_value_min[blk] = m_values[0];
    for (i = 1; i < m_nnodes; i++) {
	if (m_value_max[blk] < m_values[i]) m_value_max[blk] = m_values[i];
	if (m_value_min[blk] > m_values[i]) m_value_min[blk] = m_values[i];
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
int read_variables( int step_loop )
{
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

    bsize = Fparam.blocksize;

    sprintf(filename, "%s/%s", Fparam.in_dir, Fparam.valuefile);

    /* オープン */
    ifs = fopen(filename, "rb");
    if (NULL == ifs) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }

    skip = m_nnodes * 4;
    for (;;) {
	fread(&step, 4, 1, ifs);
#ifndef NO_ENDIAN
	if (ENDIAN) step = ConvertEndianI(step);
#endif
	if (step == step_loop) {
	    break;
	}
	fseek(ifs, skip, SEEK_CUR);
    }

    /* data */
    cnt = 0;
#ifdef BLOCK_READ
    div = (Fparam.dim1 * Fparam.dim2 * Fparam.dim3) / bsize;
    mod = (Fparam.dim1 * Fparam.dim2 * Fparam.dim3) % bsize;
    for (i = 0; i < div; i++) {
	cnt += fread(&m_values[bsize * i], sizeof (float), bsize, ifs);
    }
    if (mod > 0) {
	cnt += fread(&m_values[bsize * div], sizeof (float), mod, ifs);
    }
#else
    {
	Int64 j;
	for (j = 0; j < Fparam.dim1 * Fparam.dim2; j++) {
	    i = j * Fparam.dim3;
	    cnt += fread(&m_values[i], sizeof (float), Fparam.dim3, ifs);
	}
    }
#endif
    if (m_nnodes != cnt) {
	filter_log(FILTER_ERR, "read error %d != %d .\n", cnt, m_nnodes);
	goto garbage;
    }
#ifndef NO_ENDIAN
    if (ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
	for (i = 0; i < m_nnodes; i++) {
	    m_values[i] = ConvertEndianF(m_values[i]);
	}
    }
#endif

    blk = step_loop - Fparam.start_step;

    m_value_max[blk] = m_values[0];
    m_value_min[blk] = m_values[0];
    for (i = 1; i < m_nnodes; i++) {
	if (m_value_max[blk] < m_values[i]) m_value_max[blk] = m_values[i];
	if (m_value_min[blk] > m_values[i]) m_value_min[blk] = m_values[i];
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
int read_variables_2( FILE *ifs, int step_loop )
{
    int state = RTC_NG;
    int skip;
    int step;
    int blk;
    Int64 cnt;
    Int64 i;
    int bsize;
    int div;
    int mod;

    bsize = Fparam.blocksize;

    /* オープン */
    if (NULL == ifs) {
	filter_log(FILTER_ERR, "can not open %s.\n", filename);
	goto garbage;
    }

    skip = m_nnodes * 4;
    step = step_loop - (pre_step + 1);
    if (step > 0) {
	skip = m_nnodes * 4 + Fparam.valueskip;
	for (i = 0; i < step; i++) {
	    fseek(ifs, skip, SEEK_CUR);
	}
    }
    pre_step = step_loop;
    fseek(ifs, Fparam.valueskip, SEEK_CUR);

    /* data */
    cnt = 0;
#ifdef BLOCK_READ
    div = (Fparam.dim1 * Fparam.dim2 * Fparam.dim3) / bsize;
    mod = (Fparam.dim1 * Fparam.dim2 * Fparam.dim3) % bsize;
    for (i = 0; i < div; i++) {
	cnt += fread(&m_values[bsize * i], sizeof (float), bsize, ifs);
    }
    if (mod > 0) {
	cnt += fread(&m_values[bsize * div], sizeof (float), mod, ifs);
    }
#else
    {
	Int64 j;
	for (j = 0; j < Fparam.dim1 * Fparam.dim2; j++) {
	    i = j * Fparam.dim3;
	    cnt += fread(&m_values[i], sizeof (float), Fparam.dim3, ifs);
	}
    }
#endif
    if (m_nnodes != cnt) {
	filter_log(FILTER_ERR, "read error %d != %d .\n", cnt, m_nnodes);
	goto garbage;
    }
#ifndef NO_ENDIAN
    if (ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
	for (i = 0; i < m_nnodes; i++) {
	    m_values[i] = ConvertEndianF(m_values[i]);
	}
    }
#endif

    blk = step_loop - Fparam.start_step;

    m_value_max[blk] = m_values[0];
    m_value_min[blk] = m_values[0];
    for (i = 1; i < m_nnodes; i++) {
	if (m_value_max[blk] < m_values[i]) m_value_max[blk] = m_values[i];
	if (m_value_min[blk] > m_values[i]) m_value_min[blk] = m_values[i];
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
int read_variables_3( int step_loop )
{
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
  
    bsize = Fparam.blocksize;
    work = (float*) malloc(sizeof (float)*m_nnodes);

    /* オープン */
    for (kd = 0; kd < m_nkinds; kd++) {
	for (sp = pre_step + 1; sp <= step_loop; sp++) {
	    if (strstr(Fparam.stepf[sp].value[kd].ftype, "ascii")) {
		if (m_value_fp[kd] == NULL) {
		    sprintf(filename, "%s/%s", Fparam.in_dir, Fparam.stepf[sp].value[kd].fname);
		    /* オープン */
		    ifp = fopen(filename, "r");
		    if (NULL == ifp) {
			filter_log(FILTER_ERR, "can not open %s.\n", filename);
			goto garbage;
		    }
		} else {
		    ifp = m_value_fp[kd];
		}
		/* とりあえずSKIP行分読み飛ばす */
		for (i = 0; i < Fparam.stepf[sp].value[kd].skip; i++) {
		    if (fgets(buff, LINELEN_MAX, ifp) == NULL) {
			fclose(ifp);
			ifp = NULL;
			filter_log(FILTER_ERR, "can not open %s.\n", filename);
			goto garbage;
		    }
		}
		offset_cnt = Fparam.stepf[sp].value[kd].offset;
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
				if (item_cnt >= m_nnodes) {
				    break;
				}
			    }
			    stride_cnt++;
			    if (stride_cnt == Fparam.stepf[sp].value[kd].stride) {
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
				if (item_cnt >= m_nnodes) {
				    break;
				}
			    }
			    stride_cnt++;
			    if (stride_cnt == Fparam.stepf[sp].value[kd].stride) {
				stride_cnt = 0;
			    }
			}
		    }
		}
	    } else {
		if (m_value_fp[kd] == NULL) {
		    sprintf(filename, "%s/%s", Fparam.in_dir, Fparam.stepf[sp].value[kd].fname);
		    ifp = fopen(filename, "rb");
		    if (NULL == ifp) {
			filter_log(FILTER_ERR, "can not open %s.\n", filename);
			goto garbage;
		    }
		} else {
		    ifp = m_value_fp[kd];
		}
		if (sp < step_loop) {
		    if (!strcmp(Fparam.data, "byte")) {
			skip = Fparam.stepf[sp].value[kd].skip + m_nnodes * sizeof (char);
		    } else if (!strcmp(Fparam.data, "short")) {
			skip = Fparam.stepf[sp].value[kd].skip + m_nnodes * sizeof (short);
		    } else if (!strcmp(Fparam.data, "integer")) {
			skip = Fparam.stepf[sp].value[kd].skip + m_nnodes * sizeof (int);
		    } else if (!strcmp(Fparam.data, "float")) {
			skip = Fparam.stepf[sp].value[kd].skip + m_nnodes * sizeof (float);
		    } else if (!strcmp(Fparam.data, "double")) {
			skip = Fparam.stepf[sp].value[kd].skip + m_nnodes * sizeof (double);
		    } else {
			skip = Fparam.stepf[sp].value[kd].skip + m_nnodes;
		    }
		    fseek(ifp, skip, SEEK_CUR);
		} else {
		    fseek(ifp, Fparam.stepf[step_loop].value[kd].skip, SEEK_CUR);
		    /* data */
		    cnt = 0;
		    div = m_nnodes / bsize;
		    mod = m_nnodes % bsize;

		    if (!strcmp(Fparam.data, "byte")) {
			workc = (char*) malloc(sizeof (char)*m_nnodes);
			for (i = 0; i < div; i++) {
			    cnt += fread(&workc[bsize * i], sizeof (char), bsize, ifp);
			}
			if (mod > 0) {
			    cnt += fread(&workc[bsize * div], sizeof (char), mod, ifp);
			}
			for (i = 0; i < m_nnodes; i++) {
			    work[i] = (float) (workc[i]);
			}
			free(workc);
		    } else if (!strcmp(Fparam.data, "short")) {
			works = (short*) malloc(sizeof (short)*m_nnodes);
			for (i = 0; i < div; i++) {
			    cnt += fread(&works[bsize * i], sizeof (short), bsize, ifp);
			}
			if (mod > 0) {
			    cnt += fread(&works[bsize * div], sizeof (short), mod, ifp);
			}
			for (i = 0; i < m_nnodes; i++) {
			    work[i] = (float) (works[i]);
			}
			free(works);
		    } else if (!strcmp(Fparam.data, "integer")) {
			worki = (int*) malloc(sizeof (int)*m_nnodes);
			for (i = 0; i < div; i++) {
			    cnt += fread(&worki[bsize * i], sizeof (int), bsize, ifp);
			}
			if (mod > 0) {
			    cnt += fread(&worki[bsize * div], sizeof (int), mod, ifp);
			}
			for (i = 0; i < m_nnodes; i++) {
			    work[i] = (float) (worki[i]);
			}
			free(worki);
		    } else if (!strcmp(Fparam.data, "float")) {
			for (i = 0; i < div; i++) {
			    cnt += fread(&work[bsize * i], sizeof (float), bsize, ifp);
			}
			if (mod > 0) {
			    cnt += fread(&work[bsize * div], sizeof (float), mod, ifp);
			}
		    } else if (!strcmp(Fparam.data, "double")) {
			workd = (double*) malloc(sizeof (double)*m_nnodes);
			for (i = 0; i < div; i++) {
			    cnt += fread(&workd[bsize * i], sizeof (double), bsize, ifp);
			}
			if (mod > 0) {
			    cnt += fread(&workd[bsize * div], sizeof (double), mod, ifp);
			}
			for (i = 0; i < m_nnodes; i++) {
			    work[i] = (float) (workd[i]);
			}
			free(workd);
		    } else {
			;
		    }
		    if (m_nnodes != cnt) {
			filter_log(FILTER_ERR, "read error %d != %d .\n", cnt, m_nnodes);
			goto garbage;
		    }
#ifndef NO_ENDIAN
		    if (ENDIAN) {
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
			for (i = 0; i < m_nnodes; i++) {
			    work[i] = ConvertEndianF(work[i]);
			}
		    }
#endif
		}
	    }

	    if (sp == step_loop) {
		blk = step_loop * m_nkinds;
		m_value_max[blk + kd] = work[0];
		m_value_min[blk + kd] = work[0];
		for (i = 1; i < m_nnodes; i++) {
		    if (m_value_max[blk + kd] < work[i]) m_value_max[blk + kd] = work[i];
		    if (m_value_min[blk + kd] > work[i]) m_value_min[blk + kd] = work[i];
		}
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(i)
#endif
		for (i = 0; i < m_nnodes; i++) {
		    m_values[i * m_nkinds + kd] = work[i];
		}
	    }
	    if (Fparam.stepf[sp].value[kd].close == 1) {
		fclose(ifp);
		m_value_fp[kd] = NULL;
	    } else {
		m_value_fp[kd] = ifp;
	    }
	}
    }
    pre_step = step_loop;
    state = RTC_OK;
garbage:
    free(work);
    work = NULL;
    return (state);
}
