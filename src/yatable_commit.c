#include <string.h>
#include "yatable_key.h"
#include "yatable_commit.h"
#include "yatable_private.h"

char* YaTableCommitGetCommitStr(YaTableSid sid)
{
    return YATABLESID(sid)->commitstr;
}

char* YaTableCommitGetNextStr(YaTableSid sid)
{
    YaTableCommit* commit = YATABLESID(sid)->commit;
    if(commit->commitnextlen == 0) return NULL;
    return commit->commitnext + (commit->commitnextmaxlen -
                                 commit->commitnextlen);
}

static size_t YaTableCommitPreProclen(YaTableSid sid)
{
    return YATABLESID(sid)->commitlen;
}

char* YaTableCommitGetAliasStr(YaTableSid sid)
{
    YaTableCommit* commit = YATABLESID(sid)->commit;
    YaTableKeyInfo* key = YATABLESID(sid)->info->keycode;
    YaTableKeyInfo* keycode = NULL;
    size_t keynum = YATABLESID(sid)->info->num_keycode;
    char* commitalias = commit->commitalias;
    char* commitprev = commit->commitprev;
    size_t prevlen = YaTableCommitPreProclen(sid);
    size_t aliaslen = 0;
    size_t i = 0;

    for(i = 0; i < prevlen; i++) {
        keycode = YaTableKeyGetKeyInfoByKeynameaddr(key, keynum, commitprev[i]);
        if(keycode != NULL) aliaslen += keycode->aliaslen;
    }

    return commitalias + aliaslen;
}

char* YaTableCommitGetPrevStr(YaTableSid sid)
{
    YaTableCommit* commit = YATABLESID(sid)->commit;
    char* commitprev = commit->commitprev;

    return commitprev + YaTableCommitPreProclen(sid);
}

void YaTableCommitClean(YaTableSid sid)
{
    YaTableCommit* commit = YATABLESID(sid)->commit;

    if(commit->commitprev != NULL) {
        yafree(commit->commitprev);
        commit->commitprev = NULL;
    }

    if(commit->commitnext != NULL) {
        yafree(commit->commitnext);
        commit->commitnext = NULL;
    }

    if(commit->commitalias != NULL) {
        yafree(commit->commitalias);
        commit->commitalias = NULL;
    }

    commit->commitprevmaxlen = 0;
    commit->commitnextmaxlen = 0;
    commit->commitaliasmaxlen = 0;
    commit->state = false;
    commit->cursorindex = 0;
    commit->commitnextlen = 0;
    YATABLESID(sid)->commitlen = 0;

    if(YATABLESID(sid)->commitdata != NULL) {
        YaTableCommitData* data = YATABLESID(sid)->commitdata;
        YaTableCommitData* prev;
        while(data != NULL) {
            yafree(data->text);
            yafree(data->code);
            prev = data;
            data = data->next;
            yafree(prev);
        }
        yafree(YATABLESID(sid)->commitstr);
        YATABLESID(sid)->commitdata = NULL;
        YATABLESID(sid)->commitstr = NULL;
        YATABLESID(sid)->commitstrmaxlen = 0;
    }

}

static void YaTableCommitUpdateAlias(YaTableSid sid,
                                     YaTableKeyInfo* keyinfo, boolean del)
{
    YaTableCommit* commit = YATABLESID(sid)->commit;
    YaTableInfo* info = YATABLESID(sid)->info;
    char* commitprev = commit->commitprev;

    if(del) {
        if(commit->cursorindex > 0) {
            size_t len = 0;
            char lastcommit = '\0';
            YaTableKeyInfo* lastkey= NULL;

            len = strlen(commitprev) - 1;
            lastcommit = commitprev[len];
            lastkey = YaTableKeyGetKeyInfoByKeynameaddr(info->keycode,
                      info->num_keycode, lastcommit);

            len = strlen(commit->commitalias);
            commit->commitalias[len - lastkey->aliaslen] = '\0';

        }
        return;
    }

    if(commit->commitalias == NULL) {
        commit->commitalias = yamalloc(sizeof(char) * (COMMIT_STR_LEN + 1));
        commit->commitaliasmaxlen = COMMIT_STR_LEN;
        strcpy(commit->commitalias, keyinfo->keyalias);
        return;
    } else {
        size_t i = strlen(commit->commitalias);
        if((i + keyinfo->aliaslen)  >= commit->commitaliasmaxlen) {
            char* sztemp = yamalloc(sizeof(char) * (commit->commitaliasmaxlen +
                                                    COMMIT_STR_LEN + 1));
            commit->commitaliasmaxlen += COMMIT_STR_LEN;

            strcpy(sztemp, commit->commitalias);
            yafree(commit->commitalias);
            commit->commitalias = sztemp;
        }
    }
    strcat(commit->commitalias, keyinfo->keyalias);
}

static void YaTableCommitUpdatePrev(YaTableSid sid, YaTableKeyInfo* keyinfo,
                                    unsigned int state, boolean del)
{
    YaTableCommit* commit = YATABLESID(sid)->commit;

    if(del) {
        if(commit->cursorindex > 0) {
            commit->commitprev[strlen(commit->commitprev) - 1] = '\0';
            commit->cursorindex --;
        }
        return;
    }

    int len = 0;
    if(commit->commitprev == NULL) {
        commit->commitprev = yamalloc(sizeof(char) * (COMMIT_STR_LEN + 1));
        commit->commitprevmaxlen = COMMIT_STR_LEN;
    } else {
        len = strlen(commit->commitprev);
        if((len + sizeof(char)) >= commit->commitprevmaxlen) {
            char* sztemp = yamalloc(sizeof(char) * (commit->commitprevmaxlen +
                                                    COMMIT_STR_LEN + 1));
            strcpy(sztemp, commit->commitprev);
            commit->commitprevmaxlen += COMMIT_STR_LEN;

            yafree(commit->commitprev);
            commit->commitprev = sztemp;
        }
    }
    memcpy(commit->commitprev + len, &(keyinfo->keyname), sizeof(char));
    commit->cursorindex ++;
}

static size_t rememoffset(char* src, char* dest, size_t count)
{
    size_t i;
    for(i = count; i > 0; i--) {
        dest[i - 1] = src[i - 1];
        src[i - 1] = '\0';
    }
    return count - i;
}

static void YaTableCommitUpdateNext(YaTableSid sid, YaTableKeyInfo* keyinfo,
                                    boolean del)
{
    YaTableCommit* commit = YATABLESID(sid)->commit;
    char* commitprev = commit->commitprev;

    if(del) {
        commit->commitnextlen --;
    } else {
        size_t len = 0;
        char lastcommit = '\0';

        len = strlen(commitprev) - 1;
        lastcommit = commitprev[len];
        if(commit->commitnext == NULL) {
            commit->commitnext = yamalloc(sizeof(char) * (COMMIT_STR_LEN + 1));
            commit->commitnextmaxlen = COMMIT_STR_LEN;
            memcpy(commit->commitnext + (sizeof(char) * (COMMIT_STR_LEN - 1)),
                   &lastcommit, sizeof(char));
            commit->commitnextlen = 1;
            return;
        } else {
            size_t nextlen = commit->commitnextlen;
            size_t maxlen = commit->commitnextmaxlen;
            if(nextlen >= maxlen) {
                maxlen += COMMIT_STR_LEN;
                char* sztmp = yamalloc(sizeof(char) * (maxlen + 1));
                rememoffset(commit->commitnext, sztmp + COMMIT_STR_LEN,
                            sizeof(char) * commit->commitnextlen);
                yafree(commit->commitnext);
                commit->commitnext = sztmp;
                commit->commitnextmaxlen = maxlen;
            }
            commit->commitnext[maxlen - nextlen - 1] = lastcommit;
            commit->commitnextlen ++;
            return;
        }
    }

}

void YaTableCommitUpdate(YaTableSid sid, YaTableKeyInfo* keyinfo,
                         unsigned int state, boolean delprev, boolean delnext)
{
    if(!delnext && delprev) {
        YaTableCommitUpdateNext(sid, keyinfo, delnext);
    }

    if(!delprev && delnext) {
        YaTableInfo* info = YATABLESID(sid)->info;
        YaTableKeyInfo* tkeyinfo = NULL;
        char nextcommit = *YaTableCommitGetNextStr(sid);
        tkeyinfo = YaTableKeyGetKeyInfoByKeynameaddr(info->keycode,
                   info->num_keycode, nextcommit);
        YaTableCommitUpdateNext(sid, keyinfo, delnext);
        YaTableCommitUpdateAlias(sid, tkeyinfo, delprev);
        YaTableCommitUpdatePrev(sid, tkeyinfo, state, delprev);
        return;
    }
    YaTableCommitUpdateAlias(sid, keyinfo, delprev);
    YaTableCommitUpdatePrev(sid, keyinfo, state, delprev);
}

YaTableCommitData* YaTableCommitGenWord(YaTableSid sid,
                                        YaTableContext* context, size_t index)
{
    if(context == NULL) return NULL;
    YaTableCandInfo* cand = context->currentcand;
    char* commitstr = NULL;

    if(context == NULL) return NULL;
    if(cand == NULL) return NULL;

    if(cand->indexofpage > index) {
        while(cand->indexofpage != index)
            cand = cand->prevcand;
    } else if(cand->indexofpage < index) {
        while(cand->indexofpage != index) {
            cand = cand->nextcand;
            if(cand == NULL) break;
        }
        if(cand->indexofpage != index) return NULL;
    }

    YaTableCommitData* cur = YATABLESID(sid)->commitdata;
    YaTableCommitData* data = yamalloc(sizeof(YaTableCommitData));

    if(cur == NULL) {
        YATABLESID(sid)->commitdata = data;
        data->prev = NULL;
        data->next = NULL;
        data->index = 0;
    } else {
        while(cur->next != NULL)
            cur = cur->next;
        size_t index = cur->index;
        cur->next = data;
        data->prev = cur;
        data->index = index + 1;
    }

    data->text = cand->candword;
    data->code = cand->code;
    data->commitlen = context->procindex;
    cand->candword = NULL;
    cand->code = NULL;

    commitstr = YATABLESID(sid)->commitstr;
    if(YATABLESID(sid)->commitstr == NULL) {
        commitstr = yamalloc(sizeof(char) * (COMMIT_STR_LEN + 1));
        YATABLESID(sid)->commitstr = commitstr;
        YATABLESID(sid)->commitstrmaxlen = COMMIT_STR_LEN;
    }

    size_t maxlen = YATABLESID(sid)->commitstrmaxlen;
    if((strlen(commitstr) + strlen(data->text)) >= maxlen) {
        commitstr = yamalloc(sizeof(char) * (COMMIT_STR_LEN + maxlen + 1));
        strcpy(commitstr, YATABLESID(sid)->commitstr);
        yafree(YATABLESID(sid)->commitstr);
        YATABLESID(sid)->commitstr = commitstr;
    }
    strcat(commitstr, data->text);

    size_t commitlen = YATABLESID(sid)->commitlen;
    YATABLESID(sid)->commitlen = commitlen + data->commitlen;

    return data;
}

