#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "filter_io_plot3d_config.h"
#include "../filter_log.h"

/**
 * Initialize plot3d confiuration to default values;
 */
void plot3d_config_initializeConfig() {
    grid_params* cp = &sParams.coordinate_params;
    memset(cp->file_prefix, 0x00, LINELEN_MAX);
    memset(cp->file_suffix, 0x00, 12);
    cp->has_iblanks = false;
    cp->readmode = ASCII;
    cp->precision = DOUBLE_T;
    cp->endian = LITTLE_ENDIAN_T;
    cp->use_bytecount = true;
    cp->num_vars = 3;
    cp->use_blockcount = true;
    cp->enabled = false;

    grid_params* sp = &sParams.solution_params;
    memset(sp->file_prefix, 0x00, LINELEN_MAX);
    memset(sp->file_suffix, 0x00, 12);
    sp->has_iblanks = false;
    sp->readmode = ASCII;
    sp->precision = DOUBLE_T;
    sp->endian = LITTLE_ENDIAN_T;
    sp->use_bytecount = true;
    sp->num_vars = 5;
    sp->use_blockcount = true;
    sp->enabled = false;

    grid_params* fp = &sParams.function_params;
    memset(fp->file_prefix, 0x00, LINELEN_MAX);
    memset(fp->file_suffix, 0x00, 12);
    fp->has_iblanks = false;
    fp->readmode = ASCII;
    fp->precision = DOUBLE_T;
    fp->endian = LITTLE_ENDIAN_T;
    fp->use_bytecount = true;
    fp->num_vars = 0;
    fp->use_blockcount = true;
    fp->enabled = false;

    sprintf(sParams.fieldType, "%s", "irregular");
}

/**
 * Dump a configuration block to log output
 * @param pBlock the parameter block to dump
 */
void plot3d_config_dumpConfigBlock(grid_params* pBlock) {
    filter_log(FILTER_LOG, "##\t grid_file_prefix:%s \n", pBlock->file_prefix);
    filter_log(FILTER_LOG, "##\t grid_file_suffix:%s \n", pBlock->file_suffix);
    filter_log(FILTER_LOG, "##\t  has_iblanks: %i \n", pBlock->has_iblanks);
    filter_log(FILTER_LOG, "##\t  readmode: %i \n", pBlock->readmode);
    filter_log(FILTER_LOG, "##\t  precision: %i \n", pBlock->precision);
    filter_log(FILTER_LOG, "##\t  use bytecount: %i \n", pBlock->use_bytecount);
    filter_log(FILTER_LOG, "##\t  num vars: %i \n", pBlock->num_vars);
    filter_log(FILTER_LOG, "##\t  read grid: %i \n", pBlock->num_vars);
}

/**
 * Reads the plot 3d configuration file
 * @return error state
 */
int plot3d_config_read() {
    int n_fields;
    char filePath[FILELEN_MAX];
    char line[LINELEN_MAX];
    char scanned_key[51];
    char scanned_val[51];
    char scan_group[51];

    sprintf(filePath, "%s/%s", Fparam.in_dir, Fparam.plot3d_config);
    FILE* fp = fopen(filePath, "r");

    if (fp == NULL) {
	filter_log(FILTER_ERR, "can not open PLOT3D config: %s .\n", filePath);
	ASSERT_FAIL("Cannot open config file.");
    }
    filter_log(FILTER_LOG, "Reading PLOT3D config file:%s\n", filePath);


    plot3d_config_initializeConfig();

    sParams.coordinate_params.gridType = COORDINATE_GRID;
    sParams.function_params.gridType = FUNCTION_GRID;
    sParams.solution_params.gridType = SOLUTION_GRID;

    grid_params* current_pBlock;
    while (fgets(line, LINELEN_MAX, fp) != NULL) {
	if (line[0] == '#') {
	    continue;
	}
	n_fields = sscanf(line, "%[a-zA-Z]_%[a-zA-Z_]=%s", scan_group, scanned_key, scanned_val);
	if (n_fields != 3) {
	    continue;
	}

	if (strstr(scan_group, "coordinate")) {
	    current_pBlock = &(sParams.coordinate_params);
	} else if (strstr(scan_group, "solution")) {
	    current_pBlock = &(sParams.solution_params);
	} else if (strstr(scan_group, "function")) {
	    current_pBlock = &(sParams.function_params);
	} else {
	    continue;
	}

	if (strstr(scanned_key, "field_type")) {
	    sprintf(sParams.fieldType, "%s", scanned_val);
	}
	if (strstr(scanned_key, "file_prefix")) {
	    sprintf(current_pBlock->file_prefix, "%s", scanned_val);
	    current_pBlock->enabled = true;
	}
	if (strstr(scanned_key, "file_suffix")) {
	    sprintf(current_pBlock->file_suffix, "%s", scanned_val);
	    current_pBlock->enabled = true;
	}
	if (strstr(scanned_key, "mode_iblanks")) {
	    if (scanned_val[0] == '1') {
		current_pBlock->has_iblanks = true;
	    }
	}
	if (strstr(scanned_key, "mode_endian")) {
	    if (scanned_val[0] == 'b' || scanned_val[0] == 'B') {
		current_pBlock->endian = BIG_ENDIAN_T;
	    }
	}
	if (strstr(scanned_key, "read_mode")) {
	    if (scanned_val[0] == 'b' || scanned_val[0] == 'B') {
		current_pBlock->readmode = BINARY;
	    }
	}
	if (strstr(scanned_key, "mode_precision")) {

	    if (scanned_val[0] == 'f' || scanned_val[0] == 'F') {
		current_pBlock->precision = FLOAT_T;
	    }
	}
	if (strstr(scanned_key, "mode_usebytecount")) {
	    if (scanned_val[0] == '0') {
		current_pBlock->use_bytecount = false;
	    }
	}
	if (strstr(scanned_key, "mode_useblockcount")) {
	    if (scanned_val[0] == '0') {
		current_pBlock->use_blockcount = false;
	    }
	}
	if (strstr(scanned_key, "num_vars")) {
	    current_pBlock->num_vars = atoi(scanned_val);
	}
    }
    fclose(fp);
#ifdef DEBUG_LEVEL
    filter_log(FILTER_LOG, "## PLOT3D Coordinate params:\n");
    plot3d_config_dumpConfigBlock(&sParams.coordinate_params);
    filter_log(FILTER_LOG, "## PLOT3D Solution params:\n");
    plot3d_config_dumpConfigBlock(&sParams.solution_params);
    filter_log(FILTER_LOG, "## PLOT3D Function params:\n");
    plot3d_config_dumpConfigBlock(&sParams.function_params);
#endif
    return RTC_OK;
}