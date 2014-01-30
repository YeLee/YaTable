#ifndef YATABLE_COMMIT_H
#define YATABLE_COMMIT_H
#include "yatable_def.h"

void YaTableCommitClean(YaTableSid sid);
char* YaTableCommitGetCommitStr(YaTableSid sid);
char* YaTableCommitGetAliasStr(YaTableSid sid);
char* YaTableCommitGetPrevStr(YaTableSid sid);
char* YaTableCommitGetNextStr(YaTableSid sid);
YaTableCommitData* YaTableCommitGenWord(YaTableSid sid,
                                        YaTableContext* context, size_t index);

#endif

