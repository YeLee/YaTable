#include <sqlite3.h>
#include "yatable.h"
#include "yatable_context.h"
#include "yatable_key.h"
#include "yatable_dict.h"
#include "yatable_private.h"

void YaTableContextCleanContext(YaTableContext* context)
{
    YaTableCandInfo* cand = NULL,* prev = NULL;
    if(context == NULL) return;
    cand = context->cand;

    while(cand != NULL) {
        if(cand->compalias != NULL) yafree(cand->compalias);
        if(cand->candword) yafree(cand->candword);
        if(cand->code)yafree(cand->code);
        prev = cand->nextcand;
        yafree(prev);
        cand = prev;
    }
    if(context->procstr) yafree(context->procstr);
    yafree(context);
}

static unsigned long YaTableGetMatchCandWords(YaTableContext* context,
        char* sql)
{
    unsigned long candnum = 0;
    YaTableCandInfo* prev = NULL;
    if(context->cand != NULL) {
        prev = context->cand;
        while(prev->nextcand != NULL)
            prev = prev->nextcand;
        candnum = prev->index + 1;
    }

    sqlite3* dbhandle = (sqlite3*) YATABLESID(context->sid)->handle;
    size_t numofpage = context->candnumofpage;
    sqlite3_stmt* stmt = NULL;
    if(sqlite3_prepare(dbhandle, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_free(sql);
        return candnum;
    }
    sqlite3_free(sql);

    if(sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return candnum;
    }

    int count = 0;
    int icode = 0, itext = 0, iweight = 0;
    YaTableCandInfo* cand = NULL;
    char* sztmp = NULL;

    count = sqlite3_column_count(stmt);

    int i = 0;
    for(i = 0; i < count; i++) {
        if(strcmp(sqlite3_column_name(stmt ,i), "code") == 0) {
            icode = i;
        }
        if(strcmp(sqlite3_column_name(stmt ,i), "text") == 0) {
            itext = i;
        }
        if(strcmp(sqlite3_column_name(stmt ,i), "weight") == 0) {
            iweight = i;
        }
    }

    do {
        sztmp = (char*) sqlite3_column_text(stmt, itext);
        if(sztmp == NULL) continue;
        if(*sztmp == '\0') continue;

        cand = yamalloc(sizeof(YaTableCandInfo));
        if(context->cand == NULL) {
            context->cand = cand;
            cand->prevcand = NULL;
            prev = cand;
        } else {
            prev->nextcand = cand;
            cand->prevcand = prev;
            prev = cand;
        }

        cand->candword = yamalloc(sizeof(char) * (strlen(sztmp) + 1));
        strcpy(cand->candword, sztmp);

        sztmp = (char*) sqlite3_column_text(stmt, icode);
        cand->code = yamalloc(sizeof(char) * (strlen(sztmp) + 1));
        strcpy(cand->code, sztmp);

        cand->weight = sqlite3_column_int64(stmt, iweight);
        cand->index = candnum;
        cand->indexofpage = candnum % numofpage;
        if(candnum == 0) cand->selected = true;
        candnum ++;
    } while(sqlite3_step(stmt) == SQLITE_ROW);

    cand->nextcand = NULL;
    sqlite3_finalize(stmt);
    return candnum;
}

static boolean YaTableGetCandWords(YaTableContext* context, char* procstr)
{
    char* sql = NULL;
    size_t maxallmatch = YATABLESID(context->sid)->info->CodeMaxAllmatch;
    sql = (char*)sqlite3_mprintf("SELECT code, text, weight"
                                 " FROM [Data] "
                                 "WHERE code=%q%q%q ORDER BY weight DESC;",
                                 "\"", procstr, "\"");
    context->candnum = YaTableGetMatchCandWords(context, sql);

    if(context->procindex > maxallmatch) {
        sql = (char*)sqlite3_mprintf("SELECT code, text, weight"
                                     " FROM [Data] "
                                     "WHERE code LIKE %q%q%%%q "
                                     "AND code<>%q%q%q ORDER BY weight DESC;",
                                     "\"", procstr, "\"",
                                     "\"", procstr, "\"");
        context->candnum = YaTableGetMatchCandWords(context, sql);
    }

    if(context->candnum == 0) return false;
    return true;
}

static boolean YaTableContextGetCandWords(YaTableContext* context,
        boolean matchonce)
{
    if(context == NULL) return false;
    boolean resmatch = false;

    context->procindex = strlen(context->procstr);
    char* procstr = yamalloc(sizeof(char) * (context->procindex + 1));
    strcpy(procstr, context->procstr);
    context->procstr = procstr;

    do {
        resmatch = YaTableGetCandWords(context, procstr);

        if(matchonce == true) break;
        if(resmatch != true) {
            context->procindex --;
            procstr[context->procindex] = '\0';
        }
    } while((resmatch != true) && (*procstr != '\0'));
    if(resmatch == false) yafree(procstr);

    if(context != NULL) {
        context->currentpage = 0;
        context->currentcand = context->cand;
    }
    return resmatch;
}

YaTableContext* YaTableContextGetContext(YaTableSid sid, char* procstr,
        boolean matchonce)
{
    if(procstr == NULL) return NULL;
    if(*procstr == '\0') return NULL;
    boolean resmatch = false;
    YaTableContext* context = yamalloc(sizeof(YaTableContext));
    context->procstr = procstr;
    context->candnumofpage = YATABLESID(sid)->info->num_keyselect;
    context->sid = sid;
    resmatch = YaTableContextGetCandWords(context, matchonce);

    if(resmatch == false) {
        yafree(context);
        return NULL;
    }

    YaTableCandInfo* cand = context->cand;
    size_t stroffset = context->procindex;
    do {
        cand->compalias = YaTableKeyGenAliasByKeyname(sid,
                          cand->code + stroffset);
        cand = cand->nextcand;
        if(cand == NULL) break;
    } while(cand->indexofpage != 0);

    return context;
}

YaTableCandInfo* YaTableContextGetPrevPage(YaTableContext* context,
        size_t select)
{
    YaTableCandInfo* cand = context->currentcand;
    YaTableCandInfo* prev,* next;
    size_t matchlen = context->procindex;
    size_t num = context->candnumofpage - 1;
    size_t selectindex = select;

    if(cand == NULL) return NULL;
    if(selectindex > num) selectindex = num;
    if(cand->index / num == 0) return NULL;
    next = cand;

    while(next->indexofpage != 0)
        next = next->prevcand;

    prev = next->prevcand;
    while(prev->indexofpage != 0)
        prev = prev->prevcand;

    do {
        if(next->compalias != NULL) yafree(next->compalias);
        next->compalias = NULL;
        if(next->selected == true) next->selected = false;
        next = next->nextcand;
        if(next == NULL) break;
    } while(next->indexofpage != 0);

    next = prev;
    do {
        next->compalias = YaTableKeyGenAliasByKeyname(context->sid,
                          next->code + matchlen);
        if(next->indexofpage == selectindex) next->selected = true;
        next = next->nextcand;
    } while(next->indexofpage != 0);

    context->currentpage = prev->index / (num + 1);
    context->currentcand = prev;
    return prev;
}

YaTableCandInfo* YaTableContextGetNextPage(YaTableContext* context,
        size_t select)
{
    YaTableCandInfo* cand = context->currentcand;
    YaTableCandInfo* prev,* next;

    size_t matchlen = context->procindex;
    size_t num = context->candnumofpage - 1;
    size_t count = 0;
    size_t selectindex = select;

    if(cand == NULL) return NULL;
    if(selectindex > num) selectindex = num;
    if((cand->index / num) == (context->candnum / num)) return NULL;
    prev = cand;
    next = cand;

    if((cand->nextcand != NULL) && (cand->indexofpage == 0))
        next = cand->nextcand;
    if(next->nextcand == NULL) return NULL;

    while((prev->indexofpage != 0) && (prev->prevcand != NULL))
        prev = prev->prevcand;
    while((next->indexofpage != 0) && (next->nextcand != NULL))
        next = next->nextcand;

    if(next->indexofpage != 0) return NULL;
    do {
        if(prev->compalias != NULL) yafree(prev->compalias);
        prev->compalias = NULL;
        if(prev->selected == true) prev->selected = false;
        prev = prev->nextcand;
    } while(prev->indexofpage != 0);

    prev = next;
    do {
        prev->compalias = YaTableKeyGenAliasByKeyname(context->sid,
                          prev->code + matchlen);
        if(prev->indexofpage == selectindex) prev->selected = true;
        prev = prev->nextcand;
        count ++;
        if(prev == NULL) break;
    } while(prev->indexofpage != 0);

    if((count - 1) < selectindex) {
        prev = next;
        while(prev->nextcand != NULL)
            prev = prev->nextcand;
        prev->selected =  true;
    }

    context->currentpage = next->index / (num + 1);
    context->currentcand = next;
    return next;
}

YaTableCandInfo* YaTableContextSelectPrev(YaTableContext* context)
{
    YaTableCandInfo* cand = YaTableContextGetSelectedOfPage(context);
    size_t num = context->candnumofpage - 1;

    if((cand == NULL) || (cand->prevcand == NULL)) return NULL;

    if(cand->indexofpage == 0)
        return YaTableContextGetPrevPage(context, num);

    cand->selected = false;
    cand = cand->prevcand;
    cand->selected = true;
    return cand;
}

YaTableCandInfo* YaTableContextSelectNext(YaTableContext* context)
{
    YaTableCandInfo* cand = YaTableContextGetSelectedOfPage(context);
    size_t num = context->candnumofpage - 1;

    if((cand == NULL) || (cand->nextcand == NULL)) return NULL;

    if(cand->indexofpage == num)
        return YaTableContextGetNextPage(context, 0);

    cand->selected = false;
    cand = cand->nextcand;
    cand->selected = true;
    return cand;
}

YaTableCandInfo* YaTableContextGetSelectedOfPage(YaTableContext* context)
{
    YaTableCandInfo* cand = context->currentcand;
    YaTableCandInfo* next;

    size_t num = context->candnumofpage - 1;
    next = cand;
    while((next->indexofpage != num) || (next->nextcand != NULL)) {
        if(next->selected == true) return next;
        next = next->nextcand;
    }

    context->currentpage = next->index / num;
    return NULL;
}

YaTableCandInfo* YaTableContextGetSelectedOfList(YaTableContext* context)
{
    YaTableCandInfo* next = context->cand;
    while(next != NULL) {
        if(next->selected == true) return next;
        next = next->nextcand;
    };
    return NULL;
}

