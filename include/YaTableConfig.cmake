if(YaTable_INCLUDE_DIR AND YaTable_LIBRARIES)
    set(YaTable_FIND_QUIETLY TRUE)
endif(YaTable_INCLUDE_DIR AND YaTable_LIBRARIES)

include(FindPkgConfig)
PKG_CHECK_MODULES(PC_YaTable yatable)

find_path(YaTable_INCLUDE_DIR
          NAMES yatable_api.h
          HINTS ${PC_YaTable_INCLUDEDIR}
         )

find_library(YaTable_LIBRARIES
             NAMES yatable
             HINTS ${PC_YaTable_LIBDIR}
            )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(YaTable DEFAULT_MSG
                                  YaTable_LIBRARIES YaTable_INCLUDE_DIR)
