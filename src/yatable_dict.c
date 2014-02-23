#include <sqlite3.h>
#include <unistd.h>
#include "yatable_dict.h"
#include "yatable_key.h"
#include "yatable_private.h"

static sqlite3* YaTableDictInitDatabase(const char* path)
{
    sqlite3* sqlhandle = NULL;
    if(sqlite3_open(path, &sqlhandle) == SQLITE_OK) return sqlhandle;
    return NULL;
}

static boolean YaTableDictDumpDatabase(sqlite3* memdb, YaTableSid sid,
                                       boolean dumptodisk)
{
    boolean res = false;
    YaTableInfo* info = YATABLESID(sid)->info;
    sqlite3* sqlhandle = YaTableDictInitDatabase(info->userdata);
    if(sqlhandle == NULL) return res;

    sqlite3* pfrom = dumptodisk == true? memdb:sqlhandle;
    sqlite3* pto = dumptodisk == true? sqlhandle:memdb;
    sqlite3_backup* pbackup = NULL;

    pbackup = sqlite3_backup_init(pto, "main", pfrom, "main");

    if(pbackup != NULL) {
        sqlite3_backup_step(pbackup, -1);
        sqlite3_backup_finish(pbackup);
        res = true;
    }

    sqlite3_close(sqlhandle);
    return res;
}

boolean YaTableDictLoaddicts(YaTableSid sid)
{
    YATABLESID(sid)->handle = YaTableDictInitDatabase(":memory:");
    if(YATABLESID(sid)->handle == NULL) return false;
    if(YaTableDictDumpDatabase(YATABLESID(sid)->handle, sid, false) == false) {
        sqlite3_close((YATABLESID(sid))->handle);
        YATABLESID(sid)->handle = NULL;
        return false;
    }
    return true;
}

void YaTableDictUnLoaddicts(YaTableSid sid)
{
    if(YATABLESID(sid)->handle != NULL) {
        YaTableDictDumpDatabase(YATABLESID(sid)->handle, sid, true);
        sqlite3_close((YATABLESID(sid))->handle);
    }
}

static void* YaTableGetYaTableInfoValue(sqlite3* sqlhandle,
                                        YaTableTableType table, char* keyname)
{
    static char sql[64];
    static char* tablename[] = {"", "About", "option"};
    sprintf(sql, "SELECT %s FROM [%s];", keyname ,tablename[table]);

    sqlite3_stmt* stmt = NULL;
    if(sqlite3_prepare(sqlhandle, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }

    if(sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    int type = 0;
    type = sqlite3_column_type(stmt, 0);

    static int nvalue;
    char* value = NULL;
    char* svalue = NULL;
    switch(type) {
    case SQLITE_INTEGER:
        nvalue = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        return (void*) &nvalue;
    case SQLITE_TEXT:
        value = (char*) sqlite3_column_text(stmt, 0);
        svalue = yamalloc(sizeof(char) * (strlen(value) + 1));
        strcpy(svalue, value);
        sqlite3_finalize(stmt);

        return (void*) svalue;
    }
    sqlite3_finalize(stmt);
    return NULL;
}

static boolean YaTableGenUserDatabase(char* src, char* dest)
{
    static char* NewTable = "CREATE TABLE IF NOT EXISTS [tempPhrase] "
                            "([code] char, [text] char, [used] int);";
    FILE* fpsrc,* fpdest;
    char c = '\0';
    sqlite3* sqlhandle = NULL;

    fpsrc = fopen(src, "rb");
    if(fpsrc == NULL) return false;
    fpdest = fopen(dest, "wb");
    if(fpdest == NULL) {
        fclose(fpsrc);
        return false;
    }

    while(!feof(fpsrc)) {
        c = fgetc(fpsrc);
        fputc(c, fpdest);
    };
    fclose(fpsrc);
    fclose(fpdest);

    if(sqlite3_open(dest, &sqlhandle) != SQLITE_OK) {
        return false;
    }
    if(sqlite3_exec(sqlhandle, NewTable, 0, 0, NULL) != SQLITE_OK) {
        sqlite3_close(sqlhandle);
        return false;
    }
    sqlite3_close(sqlhandle);

    return true;
}

static YaTableKeyInfo* YaTableGetKeyInfo(sqlite3* sqlhandle,
        YaTableKeyType type, size_t* keynums)
{
    static char* tablename[] = {"", "keycode", "keyselect"};
    static char sql[64];

    int ikeyindex = 0, ikeycode = 0, ikeyname = 0, ikeyalias = 0;
    int count = 0;
    int max = 0;
    char* sztmp = NULL;
    char* szswap = NULL;
    sqlite3_stmt* stmt = NULL;
    YaTableKeyInfo* keyinfo = NULL;
    YaTableKeyInfo* pinfo = NULL;

    sprintf(sql, "SELECT * FROM [%s] ORDER BY keyindex DESC;", tablename[type]);

    if(sqlite3_prepare(sqlhandle, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }

    if(sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    count = sqlite3_column_count(stmt);

    int i = 0;
    for(i = 0; i < count; i++) {
        if(strcmp(sqlite3_column_name(stmt ,i), "keyindex") == 0) {
            ikeyindex = i;
        }
        if(strcmp(sqlite3_column_name(stmt ,i), "keycode") == 0) {
            ikeycode = i;
        }
        if(strcmp(sqlite3_column_name(stmt ,i), "keyname") == 0) {
            ikeyname = i;
        }
        if(strcmp(sqlite3_column_name(stmt ,i), "keyalias") == 0) {
            ikeyalias = i;
        }
    }

    count = -1;
    do {
        i = sqlite3_column_int(stmt, ikeyindex);

        if(count == -1) {
            count = i;
            max = i;
        }
        if((i < 0) || (i != count)) {
            for(i = 0; i <= max; i++) {
                if((keyinfo + i)->keyalias != NULL) {
                    yafree((keyinfo + i)->keyalias);
                }
            }
            if(keyinfo != NULL) yafree(keyinfo);
            sqlite3_finalize(stmt);
            return 0;
        }
        if(i == max) {
            keyinfo = yamalloc(sizeof(YaTableKeyInfo) * (i + 1));
            pinfo = keyinfo + i;
        }

        pinfo->keyindex = i;
        pinfo->keycode = sqlite3_column_int(stmt, ikeycode);
        pinfo->keytype = type;
        sztmp = (char*) sqlite3_column_text(stmt, ikeyname);
        pinfo->keyname = sztmp[0];
        if(type == KEYCODE) {
            szswap = (char*) sqlite3_column_text(stmt, ikeyalias);
            if(szswap != NULL) {
                pinfo->aliaslen = strlen(szswap);
                pinfo->keyalias = yamalloc(sizeof(char) *
                                           (pinfo->aliaslen  + 1));
                strcpy(pinfo->keyalias, szswap);
            } else {
                pinfo->aliaslen = strlen(sztmp);
                pinfo->keyalias = yamalloc(sizeof(char) *
                                           (pinfo->aliaslen  + 1));
                strcpy(pinfo->keyalias, sztmp);
            }
        }

        count --;
        pinfo --;

    } while(sqlite3_step(stmt) != SQLITE_DONE);
    max ++;

    if(i != 0) {
        count = i;
        int j = 0;
        while(i < max) {
            memcpy(keyinfo + j, pinfo + j + count, sizeof(YaTableKeyInfo));
            (pinfo + j)->keyindex = j;
            i++;
            j++;
        }
        max = j;
    }

    if((type == KEYSELECT) && (max > YATABLE_MAX_CAND_NUM))
        max = YATABLE_MAX_CAND_NUM;

    sqlite3_finalize(stmt);
    *keynums = max;
    return keyinfo;
}

static YaTableOtherKey* YaTableGetOtherKeys(sqlite3* sqlhandle)
{
    static char sql[64];

    int ikeytype = 0, iotherkey = 0, istat = 0;
    sqlite3_stmt* stmt = NULL;

    sprintf(sql, "SELECT KeyType, OtherKey, ModKey FROM [otherkeys];");
    if(sqlite3_prepare(sqlhandle, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }

    if(sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    size_t count = sqlite3_column_count(stmt);

    int i = 0;
    for(i = 0; i < count; i++) {
        if(strcmp(sqlite3_column_name(stmt ,i), "KeyType") == 0) {
            ikeytype = i;
        }
        if(strcmp(sqlite3_column_name(stmt ,i), "OtherKey") == 0) {
            iotherkey = i;
        }
        if(strcmp(sqlite3_column_name(stmt ,i), "ModKey") == 0) {
            istat = i;
        }
    }

    YaTableOtherKey* head = NULL,* cur = NULL,* prev = NULL;
    do {
        YaTableOtherKeyType tkeytype = KEYNULL;
        char* szkeytype = NULL;

        szkeytype = (char*) sqlite3_column_text(stmt, ikeytype);

        if(strcmp(szkeytype, "KeyBackSpace") == 0) {
            tkeytype = KEYBACKSPACE;
        } else if(strcmp(szkeytype, "KeyCommitCurrent") == 0) {
            tkeytype = KEYCOMMITCURRENT;
        } else if(strcmp(szkeytype, "KeyCommitRaw") == 0) {
            tkeytype = KEYCOMMITRAW;
        } else if(strcmp(szkeytype, "KeyCommitClear") == 0) {
            tkeytype = KEYCOMMITCLEAR;
        } else if(strcmp(szkeytype, "KeyMoveBackward") == 0) {
            tkeytype = KEYMOVEBACKWARD;
        } else if(strcmp(szkeytype, "KeyMoveForward") == 0) {
            tkeytype = KEYMOVEFORWARD;
        } else if(strcmp(szkeytype, "KeyPrevPage") == 0) {
            tkeytype = KEYPREVPAGE;
        } else if(strcmp(szkeytype, "KeyNextPage") == 0) {
            tkeytype = KEYNEXTPAGE;
        } else if(strcmp(szkeytype, "KeyPrevWord") == 0) {
            tkeytype = KEYPREVWORD;
        } else if(strcmp(szkeytype, "KeyNextWord") == 0) {
            tkeytype = KEYNEXTWORD;
        } else if(strcmp(szkeytype, "KeyHome") == 0) {
            tkeytype = KEYHOME;
        } else if(strcmp(szkeytype, "KeyEnd") == 0) {
            tkeytype = KEYEND;
        } else {
            tkeytype = KEYNULL;
        }

        if(tkeytype != KEYNULL) {
            if(head == NULL) {
                head = yamalloc(sizeof(YaTableOtherKey));
                cur = head;
            } else {
                cur = yamalloc(sizeof(YaTableOtherKey));
                prev->next = cur;
            }

            prev = cur;
            prev->keytype = tkeytype;
            prev->otherkey = sqlite3_column_int(stmt, iotherkey);
            prev->stat = sqlite3_column_int(stmt, istat);
            prev->next = NULL;
        }

    } while(sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    return head;
}

boolean YaTableDictInitYaTableInfo(YaTableSid sid)
{
    YaTableInfo* info = YATABLESID(sid)->info;
    static char* KeyName[] = {"Enable", "ProgramVersion", "DatabaseVersion",
                              "id", "DisplayName", "YaTableIndex", "LangCode",
                              "keylength", "WordSepar",
                              "CodeAllmatch", "CodeMaxAllmatch",
                              "PhraseCodeNoempty", "PhraseCodeUseonce"
                             };
    sqlite3* sqlhandle = NULL;
    boolean userdatabase = true;

    if(access(info->userdata, F_OK) != F_OK) {
        userdatabase = false;
    } else if(access(info->userdata, R_OK|W_OK) != F_OK) {
        return false;
    }

    if(userdatabase == true) {
        sqlhandle = YaTableDictInitDatabase(info->userdata);
    }

    if(sqlhandle == NULL) {
        if(access(info->sharedata, R_OK) != F_OK) {
            return false;
        }
        sqlhandle = YaTableDictInitDatabase(info->sharedata);
        if(sqlhandle == NULL) return false;
    }

    int* rint = NULL;

    rint = YaTableGetYaTableInfoValue(sqlhandle, TABLEOPTION, KeyName[0]);
    info->Enable = (rint == NULL)? 0:*rint;
    if(info->Enable == false) return false;

    rint = YaTableGetYaTableInfoValue(sqlhandle, TABLEABOUT, KeyName[1]);
    info->ProgramVer = (rint == NULL)? 0:*rint;

    rint = YaTableGetYaTableInfoValue(sqlhandle, TABLEABOUT, KeyName[2]);
    info->DatabaseVer = (rint == NULL)? 0:*rint;

    info->id = YaTableGetYaTableInfoValue(sqlhandle, TABLEOPTION, KeyName[3]);
    info->DisplayName = YaTableGetYaTableInfoValue(sqlhandle, TABLEOPTION,
                        KeyName[4]);

    rint = YaTableGetYaTableInfoValue(sqlhandle, TABLEOPTION, KeyName[5]);
    info->YaTableIndex = (rint == NULL)? 0:*rint;

    info->LangCode = YaTableGetYaTableInfoValue(sqlhandle, TABLEOPTION,
                     KeyName[6]);

    rint = YaTableGetYaTableInfoValue(sqlhandle, TABLEABOUT, KeyName[7]);
    info->keylength = (rint == NULL)? 0:*rint;

    char* sztmp = YaTableGetYaTableInfoValue(sqlhandle, TABLEOPTION,
                  KeyName[8]);
    if(sztmp != NULL) {
        info->WordSepar = sztmp[0];
        yafree(sztmp);
    }

    rint = YaTableGetYaTableInfoValue(sqlhandle, TABLEOPTION, KeyName[9]);
    info->CodeAllmatch = (rint == NULL)? 0:*rint;

    rint = YaTableGetYaTableInfoValue(sqlhandle, TABLEOPTION, KeyName[10]);
    info->CodeMaxAllmatch = (rint == NULL)? 0:*rint;

    rint = YaTableGetYaTableInfoValue(sqlhandle, TABLEOPTION, KeyName[11]);
    info->PhraseCodeNoempty = (rint == NULL)? 0:*rint;

    rint = YaTableGetYaTableInfoValue(sqlhandle, TABLEOPTION, KeyName[12]);
    info->PhraseCodeUseonce = (rint == NULL)? 0:*rint;

    info->keycode =
        YaTableGetKeyInfo(sqlhandle, KEYCODE, &(info->num_keycode));
    info->keyselect =
        YaTableGetKeyInfo(sqlhandle, KEYSELECT, &(info->num_keyselect));
    info->otherkeys = YaTableGetOtherKeys(sqlhandle);
    sqlite3_close(sqlhandle);

    if(userdatabase == false) {
        if(YaTableGenUserDatabase(info->sharedata, info->userdata) == false)
            return false;
    }

    return true;
}

void YaTableDictChangeWordWeight(YaTableSid sid, YaTableCommitData* data)
{
    if(data == NULL) return;
    sqlite3* handle = YATABLESID(sid)->handle;
    sqlite3_stmt* stmt = NULL;
    char* sql = NULL;
    unsigned long ulweight = 0;

    sql = sqlite3_mprintf("SELECT weight FROM [Data] WHERE "
                          "code=%q%q%q AND text=%q%q%q;",
                          "\"", data->code, "\"",
                          "\"", data->text, "\"");
    sqlite3_prepare(handle, sql, -1, &stmt, NULL);
    sqlite3_free(sql);

    if(sqlite3_step(stmt) == SQLITE_ROW) {
        ulweight = sqlite3_column_int64(stmt, 0);
    }
    sqlite3_finalize(stmt);

    sql = sqlite3_mprintf("UPDATE [Data] SET weight=%lu "
                          "WHERE code=%q%q%q AND text=%q%q%q;", ulweight + 1,
                          "\"", data->code, "\"",
                          "\"", data->text, "\"");
    sqlite3_exec(handle, sql, 0, 0, NULL);
    sqlite3_free(sql);
}

