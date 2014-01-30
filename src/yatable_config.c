#include <sqlite3.h>
#include "yatable_config.h"
#include "yatable_private.h"

int YaTableConfigGetINT(YaTableSid sid, char* option)
{
    char* sql = NULL;
    sqlite3_stmt* stmt = NULL;
    sqlite3* sqlhandle = YATABLESID(sid)->handle;
    int res = 0;
    int sqlres = SQLITE_OK;

    sql = sqlite3_mprintf("SELECT %q FROM [option];", option);
    sqlres = sqlite3_prepare(sqlhandle, sql, -1, &stmt, NULL);
    sqlite3_free(sql);

    if(sqlres != SQLITE_OK) return res;

    sqlres = sqlite3_step(stmt);
    if(sqlres == SQLITE_ROW) {
        res = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    return res;
}

boolean YaTableConfigGetBOOL(YaTableSid sid, char* option)
{
    if(YaTableConfigGetINT(sid, option) == 0) return false;
    return true;
}

char* YaTableConfigGetSTRING(YaTableSid sid, char* option)
{
    char* sql = NULL;
    sqlite3_stmt* stmt = NULL;
    sqlite3* sqlhandle = YATABLESID(sid)->handle;
    char* res = NULL;
    int sqlres = SQLITE_OK;

    sql = sqlite3_mprintf("SELECT %q FROM [option];", option);
    sqlres = sqlite3_prepare(sqlhandle, sql, -1, &stmt, NULL);
    sqlite3_free(sql);

    if(sqlres != SQLITE_OK) return res;

    sqlres = sqlite3_step(stmt);
    if(sqlres == SQLITE_ROW) {
        char* sztmp = (char*) sqlite3_column_text(stmt, 0);
        if((sztmp != NULL) && (*sztmp != '\0')) {
            res = yamalloc((strlen(sztmp + 1)) * sizeof(char));
            strcpy(res, sztmp);
        }
    }
    sqlite3_finalize(stmt);

    return res;
}

char YaTableConfigGetCHAR(YaTableSid sid, char* option)
{
    char res = '\0';
    char* sqlres = YaTableConfigGetSTRING(sid, option);
    if(sqlres != NULL) {
        res = *sqlres;
        yafree(sqlres);
    }
    return res;
}

boolean YaTableConfigSetINT(YaTableSid sid, char* option, int value)
{
    char* sql = NULL;
    sqlite3* sqlhandle = YATABLESID(sid)->handle;
    boolean res = true;

    sql = sqlite3_mprintf("UPDATE [option] SET %q=%d;", option, value);
    if(sqlite3_exec(sqlhandle, sql, 0, 0, NULL) != SQLITE_OK) res = false;
    sqlite3_free(sql);

    return res;
}

boolean YaTableConfigSetBOOL(YaTableSid sid, char* option, boolean value)
{
    boolean res = false;
    if(value == false) {
        res = YaTableConfigSetINT(sid, option, false);
    } else {
        res = YaTableConfigSetINT(sid, option, true);
    }
    return res;
}

boolean YaTableConfigSetSTRING(YaTableSid sid, char* option, char* value)
{
    char* sql = NULL;
    sqlite3* sqlhandle = YATABLESID(sid)->handle;
    boolean res = true;

    sql = sqlite3_mprintf("UPDATE [option] SET %q=%q%q%q;", option,
                          "\"", value, "\"");
    if(sqlite3_exec(sqlhandle, sql, 0, 0, NULL) != SQLITE_OK) res = false;
    sqlite3_free(sql);

    return res;
}

boolean YaTableConfigSetCHAR(YaTableSid sid, char* option, char value)
{
    char* sql = NULL;
    sqlite3* sqlhandle = YATABLESID(sid)->handle;
    boolean res = true;

    sql = sqlite3_mprintf("UPDATE [option] SET %q=%q%c%q;", option,
                          "\"", value, "\"");
    if(sqlite3_exec(sqlhandle, sql, 0, 0, NULL) != SQLITE_OK) res = false;
    sqlite3_free(sql);

    return res;
}

