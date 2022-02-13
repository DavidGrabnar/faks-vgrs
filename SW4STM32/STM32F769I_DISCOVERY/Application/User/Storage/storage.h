/*
 * storage.h
 *
 *  Created on: 13 Feb 2022
 *      Author: GrabPC
 *
 *  With help of example program LCD_PicturesFromSDCard
 */

#ifndef APPLICATION_USER_STORAGE_STORAGE_H_
#define APPLICATION_USER_STORAGE_STORAGE_H_

/* FatFs includes component */
#include "ff_gen_drv.h"
#include "sd_diskio.h"

#include <string.h>
#include <stdlib.h>

typedef enum {
	STORAGE_OK,
	STORAGE_NOK
} STORAGE_STATUS;

FATFS SD_FatFs;
char SD_Path[4];

STORAGE_STATUS storage_init();
STORAGE_STATUS storage_open(FIL * file, char * file_path);
STORAGE_STATUS storage_close(FIL * file);

int storage_read_line(FIL * file, char * buffer, int length); // reads next line in file
int storage_read_value(FIL * file, char * buffer, int length); // reads next value in file (skips empty lines and lines starting with #)
int storage_read_integer(FIL * file, char * buffer, int length); // reads next value in file and converts to integer
float storage_read_float(FIL * file, char * buffer, int length); // reads next value in file and converts to float

#endif /* APPLICATION_USER_STORAGE_STORAGE_H_ */
