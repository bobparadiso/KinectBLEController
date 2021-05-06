#include <Arduino.h>
#include "Terminal.h"
#include "Trace.h"
#include "gamepad_report.h"

#define BUILD_VERSION "0.2"

#define CMD_BUF_SIZE 64

extern gamepad_report_t newValue;

//
void terminalCmdHandler(char *cmd)
{
	if (strcmp(cmd, "help") == 0)
	{
		tracef("TRANSMITTER version:%s\r\n", BUILD_VERSION);
		trace("command list:\r\n");
		trace("help\r\n");
	}
	else if (strcmp(cmd, "x") == 0)
	{
		newValue.x = atoi(strtok(NULL, " \r\n"));		
	}
	else if (strcmp(cmd, "y") == 0)
	{
		newValue.y = atoi(strtok(NULL, " \r\n"));		
	}
	/*
	else if (strcmp(cmd, "z") == 0)
	{
		newValue.z = atoi(strtok(NULL, " \r\n"));		
	}
	else if (strcmp(cmd, "rx") == 0)
	{
		newValue.rx = atoi(strtok(NULL, " \r\n"));		
	}
	else if (strcmp(cmd, "ry") == 0)
	{
		newValue.ry = atoi(strtok(NULL, " \r\n"));		
	}
	else if (strcmp(cmd, "rz") == 0)
	{
		newValue.rz = atoi(strtok(NULL, " \r\n"));		
	}
	*/
	else if (strcmp(cmd, "b") == 0)
	{
		newValue.buttons = atoi(strtok(NULL, " \r\n"));		
	}
	else if (strcmp(cmd, "h") == 0)
	{
		newValue.hatSwitch = atoi(strtok(NULL, " \r\n"));		
	}
	else if (cmd)
	{
		trace("ERROR\r\n");
	}
}
