#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "yatable.h"
#include "yatable_dict.h"
#include "yatable_phrase.h"
#include "yatable_private.h"

YaTableCommit* YaTableGetCommitBySid(YaTableSid sid)
{
    return YATABLESID(sid)->commit;
}

YaTableInfo* YaTableGetYaTableInfoBySid(YaTableSid sid)
{
    return YATABLESID(sid)->info;
}

YaTableSid YaTableStartSession(YaTableInfo* info)
{
    YaTableSid sid = yamalloc(sizeof(YaTableSession));
    YATABLESID(sid)->commit = yamalloc(sizeof(YaTableCommit));
    YATABLESID(sid)->info = info;
    if(!YaTableDictInitYaTableInfo(sid)) {
        YaTableCleanSession(sid);
        return NULL;
    }
    if(!YaTableDictLoaddicts(sid)) {
        YaTableCleanSession(sid);
        return NULL;
    }
    info->rule = YaTablePhraseLoadRule(sid);
    if(info->rule == NULL) {
        YaTableCleanSession(sid);
        return NULL;
    }

    return sid;
}

static void YaTableFreeKeyInfo(YaTableKeyInfo* info, size_t num)
{
    int i = 0;
    for(i = 0; i < num; i++) {
        if((info + i)->keyalias != NULL) {
            yafree((info + i)->keyalias);
        }
    }
}

void YaTableCleanSession(YaTableSid sid)
{
    if(sid != NULL) {

        YaTableInfo* info = YATABLESID(sid)->info;
        if(info->id != NULL) yafree(info->id);
        if(info->DisplayName != NULL) yafree(info->DisplayName);
        if(info->LangCode != NULL) yafree(info->LangCode);

        if(info->keycode != NULL) {
            YaTableFreeKeyInfo(info->keycode, info->num_keycode);
            yafree(info->keycode);
        }
        if(info->keyselect != NULL) {
            YaTableFreeKeyInfo(info->keyselect, info->num_keyselect);
            yafree(info->keyselect);
        }
        if(info->otherkeys != NULL) {
            YaTableOtherKey* prev,* cur;
            cur = info->otherkeys;
            while(cur != NULL) {
                prev = cur;
                cur = cur->next;
                yafree(prev);
            }
        }
        YaTableDictUnLoaddicts(sid);
        if(info->rule != NULL) {
            YaTablePhraseUnLoadRule(info->rule);
        }

        yafree(YATABLESID(sid)->commit);
        yafree(sid);
        sid = NULL;
    }
}

void YaTableUpdateCommitData(YaTableSid sid)
{
    YaTableCommitData* data = YATABLESID(sid)->commitdata;
    if(data == NULL) return;

    if(data->next == NULL) {
        YaTableDictChangeWordWeight(sid, data);
    } else {
        YaTablePhraseAddNewPhrase(sid);
    }
}

