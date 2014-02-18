CONFIGURE_FILE(yatable.pc.in ${PROJECT_BINARY_DIR}/yatable.pc)
INSTALL(FILES ${PROJECT_BINARY_DIR}/yatable.pc DESTINATION
        ${libs_prefix}/pkgconfig)
INSTALL(FILES YaTableConfig.cmake DESTINATION ${prefix}/share/cmake/YaTable)
INSTALL(FILES yatable_api.h yatable_def.h DESTINATION ${prefix}/include)
