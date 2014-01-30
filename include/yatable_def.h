#ifndef YATABLE_DEF_H
#define YATABLE_DEF_H

#include <X11/Xutil.h>
#include <X11/Xlib.h>

#define YATABLE_MAX_CAND_NUM 10

#ifndef boolean
#define boolean int
#endif

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

typedef enum _YATABLE_KEY_EVENT_ {
    KEY_EVENT_NOTHING = 0,
    KEY_EVENT_COMMIT_CURRENT = 1,
    KEY_EVENT_COMMIT_RAW = 2,
    KEY_EVENT_COMMIT_CLEAR = 3,
    KEY_EVENT_COMMIT_CHANGED = 4,

    KEY_EVENT_SELECT = 5,
    KEY_EVENT_CODE = 6,

    KEY_EVENT_PAGE_PREV = 11,
    KEY_EVENT_PAGE_NEXT = 12,
    KEY_EVENT_CAND_PREV = 13,
    KEY_EVENT_CAND_NEXT = 14,
} YaTableKeyEvent;

typedef void* YaTableSid;

typedef enum _YATABLE_KEY_TYPE_ {
    KEYCODE = 1,
    KEYSELECT = 2,
} YaTableKeyType;

typedef struct _YATABLE_KEY_INFO_ {
    YaTableKeyType keytype;
    size_t keyindex;
    KeySym keycode;
    char keyname;
    char* keyalias;
    size_t aliaslen;
} YaTableKeyInfo;

typedef enum _YATABLE_OTHER_KEY_TYPE_ {
    KEYNULL = 0,
    KEYBACKSPACE = 1,
    KEYCOMMITCURRENT = 2,
    KEYCOMMITRAW = 3,
    KEYCOMMITCLEAR = 4,
    KEYMOVEBACKWARD = 11,
    KEYMOVEFORWARD = 12,
    KEYPREVPAGE = 13,
    KEYNEXTPAGE = 14,
    KEYPREVWORD = 15,
    KEYNEXTWORD = 16,
    KEYHOME = 17,
    KEYEND = 18
} YaTableOtherKeyType;

typedef struct _YATABLE_OTHER_KEYS_ {
    struct _YATABLE_OTHER_KEYS_* next;
    YaTableOtherKeyType keytype;
    KeySym otherkey;
    int stat;
} YaTableOtherKey;

typedef struct _YATABLE_PHRASE_RULE_ {
    struct _YATABLE_PHRASE_RULE_* next;
    int* rule;
    unsigned int codelen;
} YaTablePhraseRule;

typedef struct _YATABLE_INFO_ {
    boolean Enable;
    unsigned int ProgramVer;
    unsigned int DatabaseVer;
    char* id;
    char* DisplayName;
    unsigned int YaTableIndex;
    char* LangCode;

    char* userdata;
    char* sharedata;

    boolean CodeAllmatch;

    char WordSepar;
    boolean PhraseCodeNoempty;
    boolean PhraseCodeUseonce;
    YaTablePhraseRule* rule;

    YaTableOtherKey* otherkeys;
    size_t keylength;
    size_t num_keycode;
    YaTableKeyInfo* keycode;
    size_t num_keyselect;
    YaTableKeyInfo* keyselect;
} YaTableInfo;

typedef struct _YATABLE_COMMIT_DATA_ {
    struct _YATABLE_COMMIT_DATA_* prev;
    struct _YATABLE_COMMIT_DATA_* next;
    char* text;
    char* code;
    size_t commitlen;
    size_t index;
} YaTableCommitData;

typedef struct _YATABLE_COMMIT_ {
    char* commitprev;
    char* commitnext;
    char* commitalias;
    int state;
    size_t cursorindex;
    size_t commitprevmaxlen;
    size_t commitnextlen;
    size_t commitnextmaxlen;
    size_t commitaliasmaxlen;
} YaTableCommit;

typedef struct _YATABLE_CAND_INFO_ {
    struct _YATABLE_CAND_INFO_* prevcand;
    struct _YATABLE_CAND_INFO_* nextcand;
    unsigned long index;
    unsigned int weight;
    int indexofpage;
    char* code;
    char* candword;
    char* compalias;
    boolean selected;
} YaTableCandInfo;

typedef struct _YATABLE_CONTEXT_ {
    YaTableSid sid;
    unsigned long currentpage;
    YaTableCandInfo* currentcand;
    size_t procindex;
    char* procstr;
    YaTableCandInfo* cand;
    size_t candnumofpage;
    unsigned long candnum;
} YaTableContext;

#endif

