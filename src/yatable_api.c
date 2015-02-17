#include "yatable_api.h"
#include "yatable.h"
#include "yatable_commit.h"
#include "yatable_context.h"
#include "yatable_dict.h"
#include "yatable_key.h"
#include "yatable_config.h"
#include "yatable_phrase.h"
#include "yatable_private.h"

YATABLE_API YaTableAPI* YaTableGetAPIs()
{
    static YaTableAPI api = {0};

    api.yamalloc = &yamalloc;
    api.yafree = &yafree;
    api.startsession = &YaTableStartSession;
    api.cleansession = &YaTableCleanSession;
    api.getcommitbysid = &YaTableGetCommitBySid;
    api.getyatableinfobysid = &YaTableGetYaTableInfoBySid;
    api.updatecommitdata = &YaTableUpdateCommitData;

    api.commitclean = &YaTableCommitClean;
    api.commitgetcommitstr = &YaTableCommitGetCommitStr;
    api.commitgetaliasstr = &YaTableCommitGetAliasStr;
    api.commitgetprevstr = &YaTableCommitGetPrevStr;
    api.commitgetnextstr = &YaTableCommitGetNextStr;
    api.commitgenword = &YaTableCommitGenWord;

    api.contextcleancontext = &YaTableContextCleanContext;
    api.contextgetcontext = &YaTableContextGetContext;
    api.contextgetprevpage = &YaTableContextGetPrevPage;
    api.contextgetnextpage = &YaTableContextGetNextPage;
    api.contextselectprev = &YaTableContextSelectPrev;
    api.contextselectnext = &YaTableContextSelectNext;
    api.contextgetselectedofpage = &YaTableContextGetSelectedOfPage;
    api.contextgetselectedoflist = &YaTableContextGetSelectedOfList;

    api.dictchangewordweight = &YaTableDictChangeWordWeight;
    api.removephrase = &YaTablePhraseRemoveUserPhrase;

    api.keygetcommitstrbykeyevent = &YaTableKeyGetCommitstrByKeyEvent;
    api.keygetkeyindex = &YaTableKeyGetKeyIndex;
    api.keygetkeyevent = &YaTableKeyGetKeyEvent;
    api.processkey = &YaTableProcessKey;

    api.configopen = &YaTableConfigOpen;
    api.configclose = &YaTableConfigClose;

    api.configgetint = &YaTableConfigGetINT;
    api.configgetbool = &YaTableConfigGetBOOL;
    api.configgetstring = &YaTableConfigGetSTRING;
    api.configgetchar = &YaTableConfigGetCHAR;

    api.configsetint = &YaTableConfigSetINT;
    api.configsetbool = &YaTableConfigSetBOOL;
    api.configsetstring = &YaTableConfigSetSTRING;
    api.configsetchar = &YaTableConfigSetCHAR;

    return &api;
}

