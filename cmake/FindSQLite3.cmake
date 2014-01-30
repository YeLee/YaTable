if(SQLite3_INCLUDE_DIR AND SQLite3_LIBRARIES)
    set(SQLite3_FIND_QUIETLY TRUE)
endif(SQLite3_INCLUDE_DIR AND SQLite3_LIBRARIES)

include(FindPkgConfig)
PKG_CHECK_MODULES(PC_SQLite3 sqlite3)

find_path(SQLite3_INCLUDE_DIR
          NAMES sqlite3.h
          HINTS $ {PC_SQLite3_INCLUDEDIR}
         )

find_library(SQLite3_LIBRARIES
             NAMES sqlite3
             HINTS $ {PC_SQLite3_LIBDIR}
            )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SQLite3 DEFAULT_MSG
                                  SQLite3_LIBRARIES SQLite3_INCLUDE_DIR)
