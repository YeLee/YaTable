#ifndef YATABLE_API_H
#define YATABLE_API_H
#include "yatable_def.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct _YATABLE_API_ {
        void* (*yamalloc)(const size_t i);
        void (*yafree)(void* mem);
        YaTableSid(*startsession)(YaTableInfo* info);
        void (*cleansession)(YaTableSid sid);
        YaTableCommit* (*getcommitbysid)(YaTableSid sid);
        YaTableInfo* (*getyatableinfobysid)(YaTableSid sid);
        void (*updatecommitdata)(YaTableSid sid);

        void (*commitclean)(YaTableSid sid);
        char* (*commitgetcommitstr)(YaTableSid sid);
        char* (*commitgetaliasstr)(YaTableSid sid);
        char* (*commitgetprevstr)(YaTableSid sid);
        char* (*commitgetnextstr)(YaTableSid sid);
        YaTableCommitData* (*commitgenword)(YaTableSid sid,
                                            YaTableContext* context,
                                            size_t index);

        void (*contextcleancontext)(YaTableContext* context);
        YaTableContext* (*contextgetcontext)(YaTableSid sid, char* procstr,
                                             boolean matchonce);
        YaTableCandInfo* (*contextgetprevpage)(YaTableContext* context,
                                               size_t select);
        YaTableCandInfo* (*contextgetnextpage)(YaTableContext* context,
                                               size_t select);
        YaTableCandInfo* (*contextselectprev)(YaTableContext* context);
        YaTableCandInfo* (*contextselectnext)(YaTableContext* context);
        YaTableCandInfo* (*contextgetselectedofpage)(YaTableContext* context);
        YaTableCandInfo* (*contextgetselectedoflist)(YaTableContext* context);

        void (*dictchangewordweight)(YaTableSid sid, YaTableCommitData* data);

        char* (*keygetcommitstrbykeyevent)(YaTableSid sid,
                                           YaTableKeyEvent event);
        int (*keygetkeyindex)(YaTableSid sid);
        YaTableKeyEvent(*keygetkeyevent)(YaTableSid sid);
        boolean(*processkey)(YaTableSid sid, KeySym sym, unsigned int state);

        int (*configgetint)(YaTableSid sid, char* option);
        boolean(*configgetbool)(YaTableSid sid, char* option);
        char* (*configgetstring)(YaTableSid sid, char* option);
        char(*configgetchar)(YaTableSid sid, char* option);

        boolean(*configsetint)(YaTableSid sid, char* option, int value);
        boolean(*configsetbool)(YaTableSid sid, char* option, boolean value);
        boolean(*configsetstring)(YaTableSid sid, char* option, char* value);
        boolean(*configsetchar)(YaTableSid sid, char* option, char value);

    } YaTableAPI;

#define YATABLE_API __attribute__ ((visibility("default")))
    YATABLE_API YaTableAPI* YaTableGetAPIs();

#ifdef __cplusplus
}
#endif

#endif
