#include "yatable_key.h"
#include "yatable_commit.h"
#include "yatable_private.h"

char* YaTableKeyGetCommitstrByKeyEvent(YaTableSid sid, YaTableKeyEvent event)
{
    YaTableCommit* commit = YATABLESID(sid)->commit;
    if((event == KEY_EVENT_COMMIT_RAW) && (commit->commitprev != NULL))
        return commit->commitprev;

    char* commitstr = YATABLESID(sid)->commitstr;
    if(commitstr != NULL)
        return commitstr;
    return NULL;
}

char* YaTableKeyGenAliasByKeyname(YaTableSid sid, char* keyname)
{
    if(*keyname == '\0') return NULL;
    if(keyname == NULL) return NULL;
    size_t knlen = strlen(keyname);
    size_t aliaslen = 0;
    char* alias = NULL;
    YaTableInfo* info = YATABLESID(sid)->info;
    YaTableKeyInfo* keycode = info->keycode;
    size_t num_keycode = info->num_keycode;
    YaTableKeyInfo* cur = NULL;

    int i = 0;
    for(i = 0; i < knlen; i++) {
        cur = YaTableKeyGetKeyInfoByKeynameaddr(keycode, num_keycode,
                                                keyname[i]);
        aliaslen += cur->aliaslen;
    }

    alias = yamalloc(sizeof(char) * (aliaslen + 1));

    for(i = 0; i < knlen; i++) {
        cur = YaTableKeyGetKeyInfoByKeynameaddr(keycode, num_keycode,
                                                keyname[i]);
        strcat(alias, cur->keyalias);
    }
    return alias;
}

YaTableKeyInfo* YaTableKeyGetKeyInfoByKeynameaddr(YaTableKeyInfo* addr,
        size_t num, char keyname)
{
    size_t i = 0;
    YaTableKeyInfo* keyinfo = addr;
    for(i = 0; i < num; i++) {
        if(keyinfo->keyname == keyname) return keyinfo;
        keyinfo ++;
    }
    return NULL;
}

static YaTableKeyInfo* YaTableKeyGetKeyInfoBySymaddr(YaTableKeyInfo* addr,
        size_t num, KeySym sym)
{
    size_t i = 0;
    YaTableKeyInfo* keyinfo = addr;
    for(i = 0; i < num; i++) {
        if(keyinfo->keycode == sym) return keyinfo;
        keyinfo ++;
    }
    return NULL;
}

static YaTableOtherKey* YaTableKeyGetOtherKey(YaTableSid sid, KeySym otherkey,
        int stat)
{
    YaTableOtherKey* key = YATABLESID(sid)->info->otherkeys;
    while(key != NULL) {
        if((key->otherkey == otherkey) && (key->stat == stat))
            return key;
        key = key->next;
    }
    return NULL;
}

YaTableKeyInfo* YaTableKeyGetKeyInfoBySym(YaTableSid sid, KeySym sym)
{
    YaTableKeyInfo* addr = NULL;
    size_t num = 0;
    YaTableKeyInfo* info = NULL;
    YaTableInfo* yatableinfo = YATABLESID(sid)->info;

    if(info == NULL) {
        addr = yatableinfo->keycode;
        num = yatableinfo->num_keycode;
        info = YaTableKeyGetKeyInfoBySymaddr(addr, num, sym);
    }

    if(info == NULL) {
        addr = yatableinfo->keyselect;
        num = yatableinfo->num_keyselect;
        info = YaTableKeyGetKeyInfoBySymaddr(addr, num, sym);
    }

    return info;
}

static void YaTableUpdateCommitStr(YaTableSid sid)
{
    size_t index = YATABLESID(sid)->commitlen;
    YaTableCommitData* data = YATABLESID(sid)->commitdata;
    char* commitstr = YATABLESID(sid)->commitstr;

    if(data == NULL) return;

    while(data->next != NULL)
        data = data->next;

    YATABLESID(sid)->commitlen = index - data->commitlen;
    if(data->prev == NULL) {
        yafree(data->text);
        yafree(data->code);
        yafree(data);
        YATABLESID(sid)->commitdata = NULL;
        yafree(YATABLESID(sid)->commitstr);
        YATABLESID(sid)->commitstr = NULL;
    } else {
        commitstr[strlen(commitstr) - strlen(data->text)] = '\0';
        data = data->prev;
        yafree(data->next->text);
        yafree(data->next->code);
        yafree(data->next);
        data->next = NULL;
    }
}

int YaTableKeyGetKeyIndex(YaTableSid sid)
{
    return YATABLESID(sid)->keyindex;
}

YaTableKeyEvent YaTableKeyGetKeyEvent(YaTableSid sid)
{
    return YATABLESID(sid)->keyevent;
}

static void YaTableKeySetKeyEvent(YaTableSid sid, YaTableKeyEvent event)
{
    YATABLESID(sid)->keyevent = event;
}

static void YaTableGotoHome(YaTableSid sid)
{
    YaTableCommit* commit = YATABLESID(sid)->commit;
    char* prevstr = YaTableCommitGetPrevStr(sid);

    if((prevstr != NULL) && (*prevstr != '\0')) {
        size_t nextmaxlen = commit->commitnextmaxlen;
        size_t nextlen = commit->commitnextlen;
        size_t prevlen = strlen(prevstr);

        if((prevlen + nextlen) >= nextmaxlen) {
            size_t newmax = ((nextlen + prevlen) / COMMIT_STR_LEN + 1) *
                            COMMIT_STR_LEN;
            char* newstr = NULL;
            newstr = yamalloc((newmax + 1) * sizeof(char));

            char* nextstr = YaTableCommitGetNextStr(sid);
            if(nextstr != NULL) {
                strcpy(newstr + newmax - nextlen, nextstr);
            }

            yafree(commit->commitnext);
            commit->commitnext = newstr;
            commit->commitnextmaxlen = newmax;
            nextmaxlen = newmax;
        }

        char* commitnext = commit->commitnext;
        strncpy(commitnext + nextmaxlen - nextlen - prevlen, prevstr, prevlen);

        char* alias = YaTableCommitGetAliasStr(sid);

        memset(prevstr, '\0', prevlen * sizeof(char));
        memset(alias, '\0', strlen(alias) * sizeof(char));
        commit->cursorindex -= prevlen;
        commit->commitnextlen = nextlen + prevlen;
    }
}

static void YaTableGotoEnd(YaTableSid sid)
{
    YaTableCommit* commit = YATABLESID(sid)->commit;
    char* nextstr = YaTableCommitGetNextStr(sid);

    if((nextstr != NULL) && (*nextstr != '\0')) {
        size_t prevmaxlen = commit->commitprevmaxlen;
        size_t prevlen = commit->cursorindex;
        size_t nextlen = commit->commitnextlen;

        if((nextlen + prevlen) >= prevmaxlen) {
            size_t newmax = ((nextlen + prevlen) / COMMIT_STR_LEN + 1) *
                            COMMIT_STR_LEN;
            char* newstr = NULL;
            newstr = yamalloc((newmax + 1) * sizeof(char));

            strcpy(newstr, commit->commitprev);
            yafree(commit->commitprev);
            commit->commitprev = newstr;
            commit->commitprevmaxlen = newmax;
        }
        strcat(commit->commitprev, nextstr);

        char* alias = YaTableKeyGenAliasByKeyname(sid, commit->commitprev);
        size_t aliaslen = strlen(alias);
        if(aliaslen >= commit->commitaliasmaxlen) {
            size_t newmax = (aliaslen / COMMIT_STR_LEN + 1) * COMMIT_STR_LEN;
            char* newstr = yamalloc((newmax + 1) * sizeof(char));

            yafree(commit->commitalias);
            commit->commitalias = newstr;
            commit->commitaliasmaxlen = newmax;
        }
        strcpy(commit->commitalias, alias);
        yafree(alias);

        commit->cursorindex = prevlen + nextlen;
        commit->commitnextlen = 0;
    }
}

boolean YaTableProcessKey(YaTableSid sid, KeySym sym, unsigned int state)
{
    YaTableCommit* commit = YATABLESID(sid)->commit;
    YaTableKeyInfo* keyinfo = NULL;
    YaTableOtherKey* otherkey = NULL;
    boolean stat = commit->state;
    YATABLESID(sid)->keyindex = -1;

    if(state == 0) keyinfo = YaTableKeyGetKeyInfoBySym(sid, sym);
    if(keyinfo != NULL) {
        if((keyinfo->keytype == KEYSELECT) && (stat != 0)) {
            YATABLESID(sid)->keyindex = keyinfo->keyindex;
            YaTableKeySetKeyEvent(sid, KEY_EVENT_SELECT);
            return true;
        }

        if(keyinfo->keytype == KEYCODE) {
            if(!stat) commit->state = true;
            YaTableCommitUpdate(sid, keyinfo, state, false, false);
            YATABLESID(sid)->keyindex = keyinfo->keyindex;
            YaTableKeySetKeyEvent(sid, KEY_EVENT_CODE);
            return true;
        }
    }

    otherkey = YaTableKeyGetOtherKey(sid, sym, state);
    if(otherkey != NULL) {
        switch(otherkey->keytype) {
        case KEYBACKSPACE:
            if((commit->commitprev != NULL) && (otherkey->stat == state) &&
               stat) {
                if(commit->cursorindex == YATABLESID(sid)->commitlen)
                    YaTableUpdateCommitStr(sid);
                YaTableCommitUpdate(sid, keyinfo, state, true, true);
                if((*(commit->commitprev) == '\0') &&
                   ((commit->commitnext == NULL) ||
                    (*(commit->commitnext) == '\0'))) {
                    YaTableKeySetKeyEvent(sid, KEY_EVENT_COMMIT_CLEAR);
                    YaTableCommitClean(sid);
                    return true;
                }
                YaTableKeySetKeyEvent(sid, KEY_EVENT_COMMIT_CHANGED);
                return true;
            }
            break;
        case KEYCOMMITCURRENT:
            if(stat && (otherkey->stat == state)) {
                YaTableKeySetKeyEvent(sid, KEY_EVENT_COMMIT_CURRENT);
                return true;
            }
            break;
        case KEYCOMMITRAW:
            if(stat && (otherkey->stat == state)) {
                YaTableKeySetKeyEvent(sid, KEY_EVENT_COMMIT_RAW);
                return true;
            }
            break;
        case KEYCOMMITCLEAR:
            if(stat && (otherkey->stat == state)) {
                YaTableKeySetKeyEvent(sid, KEY_EVENT_COMMIT_CLEAR);
                return true;
            }
            break;
        case KEYCOMMITSELECTED:
            if(stat && (otherkey->stat == state)) {
                YaTableKeySetKeyEvent(sid, KEY_EVENT_COMMIT_SELECTED);
                return true;
            }
            break;
        case KEYMOVEBACKWARD:
            if(stat && (otherkey->stat == state)) {
                if((commit->commitprev != NULL) && (commit->cursorindex > 0)) {
                    if(commit->cursorindex == YATABLESID(sid)->commitlen)
                        YaTableUpdateCommitStr(sid);
                    YaTableCommitUpdate(sid, keyinfo, state, true, false);
                }
                YaTableKeySetKeyEvent(sid, KEY_EVENT_COMMIT_CHANGED);
                return true;
            }
            break;
        case KEYMOVEFORWARD:
            if(stat && (otherkey->stat == state)) {
                if((commit->commitprev != NULL) &&
                   (commit->commitnextlen > 0)) {
                    YaTableCommitUpdate(sid, keyinfo, state, false, true);
                }
                YaTableKeySetKeyEvent(sid, KEY_EVENT_COMMIT_CHANGED);
                return true;
            }
            break;
        case KEYPREVPAGE:
            if(stat && (otherkey->stat == state)) {
                YaTableKeySetKeyEvent(sid, KEY_EVENT_PAGE_PREV);
                return true;
            }
            break;
        case KEYNEXTPAGE:
            if(stat && (otherkey->stat == state)) {
                YaTableKeySetKeyEvent(sid, KEY_EVENT_PAGE_NEXT);
                return true;
            }
            break;
        case KEYPREVWORD:
            if(stat && (otherkey->stat == state)) {
                YaTableKeySetKeyEvent(sid, KEY_EVENT_CAND_PREV);
                return true;
            }
            break;
        case KEYNEXTWORD:
            if(stat && (otherkey->stat == state)) {
                YaTableKeySetKeyEvent(sid, KEY_EVENT_CAND_NEXT);
                return true;
            }
            break;
        case KEYHOME:
        case KEYEND:
            if(stat && (otherkey->stat == state)) {
                if(otherkey->keytype == KEYHOME) {
                    YaTableGotoHome(sid);
                } else {
                    YaTableGotoEnd(sid);
                }
                YaTableKeySetKeyEvent(sid, KEY_EVENT_COMMIT_CHANGED);
                return true;
            }
            break;
        case KEYREMOVEUSERPHRASE:
            if(stat && (otherkey->stat == state)) {
                YaTableKeySetKeyEvent(sid, KEY_EVENT_REMOVE_USER_PHRASE);
                YaTableCommitClean(sid);
                return true;
            }
            break;
        default:
            YaTableKeySetKeyEvent(sid, KEY_EVENT_NOTHING);
            return false;
        }
    }

    YaTableKeySetKeyEvent(sid, KEY_EVENT_NOTHING);
    return false;
}

