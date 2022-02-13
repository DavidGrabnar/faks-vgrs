/*
 * storage.c
 *
 *  Created on: 13 Feb 2022
 *      Author: GrabPC
 */

#include "storage.h"

STORAGE_STATUS storage_init()
{
	if(FATFS_LinkDriver(&SD_Driver, SD_Path) != 0)
	{
	  return STORAGE_NOK;
	}
	if(f_mount(&SD_FatFs, (TCHAR const*)"",0) != FR_OK)
	{
	  return STORAGE_NOK;
	}

	return STORAGE_OK;
}

STORAGE_STATUS storage_open(FIL * file, char * file_path)
{
	if (f_open(file, (TCHAR const*)file_path, FA_READ) != FR_OK)
	{
		return STORAGE_NOK;
	}
	return STORAGE_OK;
}

STORAGE_STATUS storage_close(FIL * file)
{
	if(f_close(file) != FR_OK)
	{
		return STORAGE_NOK;
	}
	return STORAGE_OK;
}

int storage_read_line(FIL * file, char * buffer, int length)
{
	if (f_gets(buffer, length, file) == 0) {
		return -1;
	}
	return strlen(buffer);
}

int storage_read_value(FIL * file, char * buffer, int length)
{
	int len;
	do {
		len = storage_read_line(file, buffer, length);
	} while(len != -1 && buffer[0] == '#');

	if (len > 0) {
		buffer[len - 1] = '\0';
	}

	return len;
}

int storage_read_integer(FIL * file, char * buffer, int length)
{
	storage_read_value(file, buffer, length);
	return atoi(buffer);
}

float storage_read_float(FIL * file, char * buffer, int length)
{
	storage_read_value(file, buffer, length);
	return atof(buffer);
}
