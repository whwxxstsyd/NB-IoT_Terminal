/*-----------------------------------------------------------------------------
File Name   	  :   cli.c
Author          :   zhaoji
Created Time    :   2018.02.23
Description     :   CLI接口
-----------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "FreeRTOS_CLI.h"
#include "cli.h"
#include "common.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
**                             Function Declare                               *
**----------------------------------------------------------------------------*/
BaseType_t prvDebugLevelCommand(char *pcWriteBuffer,
                                size_t xWriteBufferLen,
                                const char *pcCommandString)
{
	_CMIOT_Debug("enter debug level cli\r\n");
	return 1;
}


BaseType_t prvGetSysTimeCommand(char *pcWriteBuffer,
                                size_t xWriteBufferLen,
                                const char *pcCommandString)
{
	_CMIOT_Debug("enter getsystime cli\r\n");
	return 1;
}


/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**----------------------------------------------------------------------------*/
static CLI_Command_Definition_t CliCommand[] = 
{
	{
		"at+debuglevel",
		"\r\ndebuglevel <level>, level can be: 0 or 1\r\n",
		prvDebugLevelCommand,
		1
	},
	{
		"at+getsystime",
		"\r\nat+getsystime , get system run time(ms)\r\n",
		prvGetSysTimeCommand,
		0
	},
	
};



void _CMIOT_CliInit(void)
{
	uint8_t i;
	for(i=0; i<sizeof(CliCommand)/sizeof(CliCommand[0]); i++)
	{
		FreeRTOS_CLIRegisterCommand(&CliCommand[i]);
	}
}











