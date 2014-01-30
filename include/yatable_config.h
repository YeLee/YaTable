#ifndef YATABLE_CONFIG_H
#define YATABLE_CONFIG_H
#include "yatable_def.h"

int YaTableConfigGetINT(YaTableSid sid, char* option);
boolean YaTableConfigGetBOOL(YaTableSid sid, char* option);
char* YaTableConfigGetSTRING(YaTableSid sid, char* option);
char YaTableConfigGetCHAR(YaTableSid sid, char* option);

boolean YaTableConfigSetINT(YaTableSid sid, char* option, int value);
boolean YaTableConfigSetBOOL(YaTableSid sid, char* option, boolean value);
boolean YaTableConfigSetSTRING(YaTableSid sid, char* option, char* value);
boolean YaTableConfigSetCHAR(YaTableSid sid, char* option, char value);

#endif

