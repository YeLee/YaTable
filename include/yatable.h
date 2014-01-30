#ifndef YATABLE_H
#define YATABLE_H
#include "yatable_def.h"

YaTableSid YaTableStartSession(YaTableInfo* info);
void YaTableCleanSession(YaTableSid sid);

YaTableCommit* YaTableGetCommitBySid(YaTableSid sid);
YaTableInfo* YaTableGetYaTableInfoBySid(YaTableSid sid);
void YaTableUpdateCommitData(YaTableSid sid);

#endif

