INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/)
SET(LIBYATABLE_SRC mem.c yatable.c yatable_api.c yatable_commit.c
    yatable_context.c yatable_dict.c yatable_key.c yatable_phrase.c
    yatable_config.c)
ADD_LIBRARY(yatable SHARED ${LIBYATABLE_SRC})
TARGET_LINK_LIBRARIES(yatable ${SQLite3_LIBRARIES})
SET_TARGET_PROPERTIES(yatable PROPERTIES VERSION ${LIBYATABLE_VERSION}
    SOVERSION ${LIBYATABLE_SOVERSION})
INSTALL(TARGETS yatable DESTINATION ${prefix}/lib)
