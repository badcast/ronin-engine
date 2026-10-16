include(GNUInstallDirs)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/RoninEngine.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/RoninEngine.pc @ONLY)

install(TARGETS RoninEngine
    EXPORT RoninEngineTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT RoninEngineTargets
    FILE RoninEngineTargets.cmake
    NAMESPACE RoninEngine::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RoninEngine
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/RoninEngine)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RoninEngine.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
