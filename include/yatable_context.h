#ifndef YATABLE_CONTEXT_H
#define YATABLE_CONTEXT_H
#include "yatable_def.h"

void YaTableContextCleanContext(YaTableContext* context);
YaTableContext* YaTableContextGetContext(YaTableSid sid, char* procstr,
        boolean matchonce);
YaTableCandInfo* YaTableContextGetPrevPage(YaTableContext* context,
        size_t select);
YaTableCandInfo* YaTableContextGetNextPage(YaTableContext* context,
        size_t select);
YaTableCandInfo* YaTableContextSelectPrev(YaTableContext* context);
YaTableCandInfo* YaTableContextSelectNext(YaTableContext* context);
YaTableCandInfo* YaTableContextGetSelectedOfPage(YaTableContext* context);
YaTableCandInfo* YaTableContextGetSelectedOfList(YaTableContext* context);

#endif

