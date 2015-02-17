#ifndef YATABLE_PHRASE_H
#define YATABLE_PHRASE_H
#include "yatable_def.h"

char* YaTablePhraseGenCode(YaTableCommitData* data, YaTablePhraseRule* rule,
                           char wordsepa, boolean noempty, boolean useonce);
YaTablePhraseRule* YaTablePhraseLoadRule(YaTableSid sid);
void YaTablePhraseUnLoadRule(YaTablePhraseRule* head);
boolean YaTablePhraseRemoveUserPhrase(YaTableContext* context);
boolean YaTablePhraseAddNewPhrase(YaTableSid sid);

#endif
