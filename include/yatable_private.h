#ifndef YATABLE_PRIVATE_H
#define YATABLE_PRIVATE_H

#include "mem.h"

typedef enum _YATABLE_TABLE_TYPE_ {
    TABLEABOUT = 1,
    TABLEOPTION = 2,
} YaTableTableType;

typedef void* DictHandle;
typedef struct _YATABLE_SESSION_ {
    YaTableCommit* commit;
    YaTableInfo* info;
    YaTableCommitData* commitdata;
    size_t commitlen;
    char* commitstr;
    size_t commitstrmaxlen;
    int keyevent;
    int keyindex;
    DictHandle handle;
    int errorcode;
} YaTableSession;

#define YATABLESID(x) ((YaTableSession*) x)
#define COMMIT_STR_LEN 256

void YaTableCommitUpdate(YaTableSid sid, YaTableKeyInfo* keyinfo,
                         unsigned int state, boolean delprev, boolean delnext);

boolean YaTableDictInitYaTableInfo(YaTableSid sid);
boolean YaTableDictLoaddicts(YaTableSid sid);
void YaTableDictUnLoaddicts(YaTableSid sid);

char* YaTableKeyGenAliasByKeyname(YaTableSid sid, char* keyname);
YaTableKeyInfo* YaTableKeyGetKeyInfoBySym(YaTableSid sid, KeySym sym);
YaTableKeyInfo* YaTableKeyGetKeyInfoByKeynameaddr(YaTableKeyInfo* addr,
        size_t num, char keyname);

#endif

