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
        boolean(*removephrase)(YaTableContext* context);

        char* (*keygetcommitstrbykeyevent)(YaTableSid sid,
                                           YaTableKeyEvent event);
        int (*keygetkeyindex)(YaTableSid sid);
        YaTableKeyEvent(*keygetkeyevent)(YaTableSid sid);
        boolean(*processkey)(YaTableSid sid, KeySym sym, unsigned int state);

        YaTableConfig(*configopen)(char* userdata);
        void (*configclose)(YaTableConfig cfg);

        int (*configgetint)(YaTableConfig cfg, char* option);
        boolean(*configgetbool)(YaTableConfig cfg, char* option);
        char* (*configgetstring)(YaTableConfig cfg, char* option);
        char(*configgetchar)(YaTableConfig cfg, char* option);

        boolean(*configsetint)(YaTableConfig cfg, char* option, int value);
        boolean(*configsetbool)(YaTableConfig cfg, char* option, boolean value);
        boolean(*configsetstring)(YaTableConfig cfg, char* option, char* value);
        boolean(*configsetchar)(YaTableConfig cfg, char* option, char value);

    } YaTableAPI;

#define YATABLE_API __attribute__ ((visibility("default")))
    YATABLE_API YaTableAPI* YaTableGetAPIs();

#ifdef __cplusplus
}
#endif

#endif
