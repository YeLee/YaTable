#include <sqlite3.h>
#include "yatable_context.h"
#include "yatable_phrase.h"
#include "yatable_private.h"

static int* YaTablePhraseGenRule(char* srcstr, unsigned int codelen)
{
    if(srcstr == NULL) return NULL;
    if(codelen == 0) return NULL;
    char* rulestr = srcstr;
    int* rule = yamalloc(sizeof(int) * ((codelen * 2) + 2));

    if((rulestr[0] == 'e') || (rulestr[0] == 'E')) {
        *rule = 0;
    } else if((rulestr[0] == 'a') || (rulestr[0] == 'A')) {
        *rule = 1;
    } else {
        yafree(rule);
        return NULL;
    }

    int wordlen = str2num(rulestr + 1);
    if(wordlen <= 0) {
        yafree(rule);
        return NULL;
    }
    *(rule + 1) = wordlen;

    rulestr = strchr(rulestr, '=');
    if(rulestr == NULL) {
        yafree(rule);
        return NULL;
    }
    rulestr ++;

    size_t index = 0;
    int res = 0;
    boolean bplus = false;
    while(*rulestr != '\0') {
        res = str2num(rulestr + 1);
        if(res == 0) break;
        if(bplus && (res > codelen)) break;
        if(!bplus && (res > *(rule + 1))) break;

        if((*rulestr == 'p') || (*rulestr == 'P')) {
            *(rule + index + 2) = res;
        } else if((*rulestr == 'r') || (*rulestr == 'R')) {
            *(rule + index + 2) = 0 - res;
        } else {
            break;
        }
        if(bplus) {
            rulestr = strchr(rulestr, '+');
            if(rulestr == NULL) break;
            rulestr ++;
            bplus = false;
        } else {
            rulestr = endofnum(rulestr + 1);
            bplus = true;
        }
        index ++;
        if((index + 1) > codelen * 2) {
            yafree(rule);
            return NULL;
        }
    };

    if(((index + 1) / 2) != codelen) {
        yafree(rule);
        return NULL;
    }

    return rule;
}

static boolean YaTableCodeCheckUsed(int* index, int codecount,
                                    size_t pi, size_t ci)
{
    size_t i = 0;
    if(codecount == 0) return false;
    for(i = 0; i < codecount; i++) {
        size_t dpi = * (index + (i * 2)) - 1;
        size_t dci = * (index + (i * 2) + 1) - 1;
        if((dpi == pi) && (dci == ci)) return true;
    }
    return false;
}

static void YaTableCodeSetUsed(int* index, int codecount,
                               size_t pi, size_t ci)
{
    *(index + (codecount * 2)) = (int)(pi + 1);
    *(index + ((codecount * 2) + 1)) = (int)(ci + 1);
}

static size_t YaTableCodeGetUsedMax(int* index, int codecount, size_t pi)
{
    size_t i = 0;
    size_t max = 0;
    if(codecount == 0) return false;
    for(i = 0; i < codecount; i++) {
        size_t dpi = * (index + (i * 2)) - 1;
        size_t dci = * (index + (i * 2) + 1);
        if((dpi == pi) && (dci > max)) max = dci;
    }
    return max;
}

char* YaTablePhraseGenCode(YaTableCommitData* data, YaTablePhraseRule* rule,
                           char wordsepa, boolean noempty, boolean useonce)
{
    YaTableCommitData* cur;
    YaTablePhraseRule* p,* currule;

    if(data == NULL) return NULL;
    if(rule == NULL) return NULL;

    cur = data;
    while(cur->next != NULL) {
        cur = cur->next;
    }
    size_t wordcount = cur->index + 1;

    currule = rule;
    p= NULL;
    while(currule != NULL) {
        int* pi = currule->rule;

        if((*pi == 0) && (wordcount == *(pi + 1))) {
            p = currule;
            break;
        }
        if((*pi == 1) && (wordcount >= *(pi + 1))) {
            p = currule;
            break;
        }

        currule = currule->next;
    }
    if(p == NULL) return NULL;

    size_t i = 0;
    int* val = p->rule;
    int* pindex = yamalloc(sizeof(int) * (p->codelen));
    for(i = 0; i < p->codelen;  i++) {
        int cvar = *(val + ((i + 1) * 2));
        *(pindex + i) = cvar > 0? cvar:(wordcount + cvar +1);
    }

    i = 0;
    val = p->rule;
    char* code = yamalloc(sizeof(char) * (p->codelen + 1));
    int* codemap = yamalloc(sizeof(int) * ((p->codelen + 1) * 2));
    size_t codecount = 0;
    for(i = 0; i < p->codelen; i++) {
        char* shortstem = NULL;
        char* stem = NULL;
        size_t ishortstem = 0;
        size_t istem = 0;

        cur = data;
        while((cur->index + 1) != *(pindex + i))
            cur = cur->next;
        if(strchr(cur->code, wordsepa) != NULL) {
            istem = strlen(cur->code);
            stem = yamalloc(istem * sizeof(char));
            strcpyexceptchar(stem, cur->code, wordsepa);
            istem = strlen(stem);

            shortstem = yamalloc(istem * sizeof(char));
            strcpy2chr(shortstem, cur->code, wordsepa);
            ishortstem = strlen(shortstem);
        } else {
            stem = cur->code;
            istem = strlen(stem);
        }

        size_t pi = cur->index;
        int cindex = *(val + (i * 2) + 3);
        if(ishortstem >= absval(cindex)) {
            if(useonce && (!noempty)) {
                size_t max = YaTableCodeGetUsedMax(codemap,
                                                   codecount, pi);
                if(max >= ishortstem) {
                    yafree(shortstem);
                } else {
                    yafree(stem);
                    istem = ishortstem;
                    stem = shortstem;
                }
            } else {
                yafree(stem);
                istem = ishortstem;
                stem = shortstem;
            }
        }

        size_t tcindex = 0;
        if(cindex > 0) {
            if(cindex <= istem) {
                tcindex = cindex - 1;
                code[codecount] = stem[tcindex];
                codecount ++;
            } else if(noempty == true) {
                tcindex = istem - 1;
                code[codecount] = stem[tcindex];
                codecount ++;
            }
        } else {
            if(absval(cindex) <= istem) {
                tcindex = istem + cindex;
                code[codecount] = stem[tcindex];
                codecount ++;
            } else if(noempty == true) {
                tcindex = 0;
                code[codecount] = stem[tcindex];
                codecount ++;
            }
        }
        if(useonce && (!noempty)) {
            if(YaTableCodeCheckUsed(codemap, codecount - 1,
                                    pi, tcindex)) {
                codecount --;
                code[codecount] = '\0';
            } else {
                YaTableCodeSetUsed(codemap, codecount - 1, pi, tcindex);
            }
        }

        if(shortstem != NULL) yafree(stem);
    }
    yafree(codemap);
    yafree(pindex);
    return code;
}

YaTablePhraseRule* YaTablePhraseLoadRule(YaTableSid sid)
{
    sqlite3* sqlhandle = YATABLESID(sid)->handle;
    sqlite3_stmt* stmt = NULL;
    char sql[64] = "SELECT rule FROM [PhraseRule] ORDER BY rule;";

    int above = 0;
    YaTablePhraseRule* pr = NULL,* prev = NULL,* head = NULL,* tail = NULL;
    char* sztmp = NULL;
    char* rulestr = NULL;
    int* rule = NULL;
    size_t codelen = YATABLESID(sid)->info->keylength;

    if(sqlite3_prepare(sqlhandle, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }

    while(sqlite3_step(stmt) == SQLITE_ROW) {
        sztmp = (char*) sqlite3_column_text(stmt, 0);
        if((above > 0) && (sztmp[0] == 'a'))
            continue;

        rulestr = yamalloc(strlen(sztmp) + 1);
        strcpy(rulestr, sztmp);
        delwhitespace(rulestr);

        rule = YaTablePhraseGenRule(rulestr, codelen);
        yafree(rulestr);
        if(rule == NULL) continue;

        if(*rule == 1) {
            above = *(rule + 1);
        } else {
            if((above > 0) && (*(rule + 1) >= above)) {
                yafree(rule);
                break;
            }
        }

        if(head != NULL) {
            int* val = prev->rule;
            if(*(rule + 1) == *(val + 1)) {
                yafree(rule);
                continue;
            }
        }

        pr = yamalloc(sizeof(YaTablePhraseRule));
        pr->codelen = codelen;
        pr->next = NULL;
        pr->rule = rule;

        if(*rule == 1) {
            tail = pr;
            continue;
        }

        if(head == NULL) {
            head = pr;
            prev = pr;
        } else {
            prev->next = pr;
            prev = pr;
        }
    };

    if(head == NULL) {
        head = tail;
    } else {
        prev->next = tail;
    }

    return head;
}

void YaTablePhraseUnLoadRule(YaTablePhraseRule* head)
{
    YaTablePhraseRule* prev,* cur;
    cur = head;
    while(cur != NULL) {
        prev = cur;
        cur = cur->next;
        yafree(prev->rule);
        yafree(prev);
    };
}

static char* YaTablePhraseGetstem(YaTableSid sid, char* code, char* text)
{
    char* sql = NULL;
    sqlite3* handle = YATABLESID(sid)->handle;
    sqlite3_stmt* stmt = NULL;
    char* stem = NULL;
    size_t istem = 0;

    sql = sqlite3_mprintf("SELECT stem FROM [Data] WHERE "
                          "code=%q%q%q AND text=%q%q%q;",
                          "\"", code, "\"", "\"", text, "\"");
    sqlite3_prepare(handle, sql, -1, &stmt, NULL);
    sqlite3_free(sql);

    if(sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    char* sztmp = (char*) sqlite3_column_text(stmt, 0);

    if((sztmp != NULL) && (sztmp[0] != '\0')) {
        istem = strlen(sztmp);
        stem = yamalloc((istem + 1) * sizeof(char));
        strcpy(stem, sztmp);
    }

    sqlite3_finalize(stmt);
    return stem;
}

boolean YaTablePhraseRemoveUserPhrase(YaTableContext* context)
{
    YaTableCandInfo* cand = context->currentcand;
    char* sql = NULL;
    sqlite3* handle = YATABLESID(context->sid)->handle;

    do {
        if(cand->selected == true) break;
        cand = cand->nextcand;
        if(cand == NULL) break;
    } while(cand->indexofpage != 0);

    if(cand->selected != true) return false;

    sql = sqlite3_mprintf("DELETE FROM [Data] WHERE code=%q%q%q AND "
                          "text=%q%q%q AND weight=%u AND type=%d;",
                          "\"", cand->code, "\"",
                          "\"", cand->candword, "\"",
                          cand->weight , YATABLE_USER_PHRASE);
    sqlite3_exec(handle, sql, 0, 0, NULL);
    sqlite3_free(sql);
    return true;
}

boolean YaTablePhraseAddNewPhrase(YaTableSid sid)
{
    YaTableCommitData* commitdata = YATABLESID(sid)->commitdata;
    YaTableCommitData* data = NULL;
    sqlite3* handle = YATABLESID(sid)->handle;
    sqlite3_stmt* stmt = NULL;
    size_t codelen = 0;
    char* commitstr = YATABLESID(sid)->commitstr;
    char* sql = NULL;
    char wordsepar = YATABLESID(sid)->info->WordSepar;
    YaTablePhraseRule* rule =  YATABLESID(sid)->info->rule;;
    boolean noempty = YATABLESID(sid)->info->PhraseCodeNoempty;
    boolean useonce = YATABLESID(sid)->info->PhraseCodeUseonce;

    if(commitdata == NULL) return false;

    data = commitdata;
    while(data != NULL) {
        codelen += strlen(data->code);
        data = data->next;
    }
    char* commitcode = yamalloc(sizeof(char) * (codelen + 1));

    data = commitdata;
    while(data != NULL) {
        strcat(commitcode, data->code);
        data = data->next;
    }

    if(sqlite3_exec(handle, "CREATE TABLE IF NOT EXISTS [tempPhrase] "
                    "([code] char, [text] char, [used] int);", 0, 0, NULL) !=
       SQLITE_OK) {
        yafree(commitcode);
        return false;
    }

    sqlite3_free(sql);
    sql = sqlite3_mprintf("SELECT used FROM [tempPhrase] WHERE "
                          "code=%q%q%q AND text=%q%q%q;",
                          "\"", commitcode, "\"", "\"", commitstr, "\"");
    sqlite3_prepare(handle, sql, -1, &stmt, NULL);
    sqlite3_free(sql);

    if(sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);

        sql = sqlite3_mprintf("INSERT INTO [tempPhrase] "
                              "VALUES(\'%q\', \'%q\', 1);",
                              commitcode, commitstr);
        yafree(commitcode);

        boolean val = true;
        if(sqlite3_exec(handle, sql, 0, 0, NULL) != SQLITE_OK) val = false;

        sqlite3_free(sql);
        return val;
    }

    size_t codeweight = 3;
    size_t used = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if(used >= codeweight) {
        sql = sqlite3_mprintf("DELETE FROM [tempPhrase] "
                              "WHERE  code=\"%q\" AND text=\"%q\";",
                              commitcode, commitstr);
        if(sqlite3_exec(handle, sql, 0, 0, NULL) != SQLITE_OK) {
            yafree(commitcode);
            sqlite3_free(sql);
            return false;
        }

        data = commitdata;
        char* sztmp = NULL;
        while(data != NULL) {
            sztmp = YaTablePhraseGetstem(sid, data->code, data->text);
            if(sztmp != NULL) {
                yafree(data->code);
                data->code = sztmp;
            }
            data = data->next;
        }

        char* code = NULL;
        code = YaTablePhraseGenCode(commitdata, rule,
                                    wordsepar, noempty, useonce);
        if(code == NULL) {
            yafree(commitcode);
            sqlite3_free(sql);
            return false;
        }

        boolean val = true;
        sql = sqlite3_mprintf("SELECT weight FROM [Data] WHERE "
                              "code=%q%q%q AND text=%q%q%q;",
                              "\"", code, "\"", "\"", commitstr, "\"");
        sqlite3_prepare(handle, sql, -1, &stmt, NULL);
        sqlite3_free(sql);

        if(sqlite3_step(stmt) != SQLITE_ROW) {

            sql = sqlite3_mprintf("INSERT INTO [Data] "
                                  "VALUES(\'%q\', \'%q\', \'\', %u, %d);",
                                  code, commitstr, codeweight, \
                                  YATABLE_USER_PHRASE);
            if(sqlite3_exec(handle, sql, 0, 0, NULL) != SQLITE_OK) val = false;
            sqlite3_free(sql);
        }

        sqlite3_finalize(stmt);
        yafree(code);
        yafree(commitcode);
        return val;
    }
    boolean val = true;
    sql = sqlite3_mprintf("UPDATE [tempPhrase] SET used=%d "
                          "WHERE code=%q%q%q AND text=%q%q%q;", used + 1,
                          "\"", commitcode, "\"", "\"", commitstr, "\"");
    if(sqlite3_exec(handle, sql, 0, 0, NULL) != SQLITE_OK) val = false;
    yafree(commitcode);
    sqlite3_free(sql);
    return val;
}

