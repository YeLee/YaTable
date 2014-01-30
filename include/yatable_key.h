#ifndef YATABLE_KEY_H
#define YATABLE_KEY_H
#include "yatable_def.h"

char* YaTableKeyGetCommitstrByKeyEvent(YaTableSid sid, YaTableKeyEvent event);
int YaTableKeyGetKeyIndex(YaTableSid sid);
YaTableKeyEvent YaTableKeyGetKeyEvent(YaTableSid sid);
boolean YaTableProcessKey(YaTableSid sid, KeySym sym, unsigned int state);

#endif

