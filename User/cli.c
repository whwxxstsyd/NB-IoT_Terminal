/*-----------------------------------------------------------------------------
File Name   	  :   cli.c
Author          :   zhaoji
Created Time    :   2018.01.12
Description     :   cli接口
-----------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "cli.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"


/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
void _CMIOT_SetDebugLevel();


static const CLI_Command_Definition_t setDebugFlag =
{
    "debug 1",
    "del <filename>: Deletes <filename> from the disk\r\n",
    _CMIOT_SetDebugLevel,
    1
};

/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/



void _CMIOT_SetDebugLevel(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	
}