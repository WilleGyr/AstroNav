cmake_minimum_required(VERSION 3.16)
project(SimpleShape LANGUAGES CXX)

# Enable automatic Qt processing
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)

# Find all required Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
    Multimedia
    3DCore
    3DRender
    3DExtras
    3DInput
    3DLogic
)

# Add executable with all source files
add_executable(SimpleShape
    main.cpp
    music.cpp
    databasehandler.cpp
    starcreator.cpp
    qtmanager.cpp
    cameramanager.cpp
    firstpersoncameracontroller.cpp
    thirdpersoncameracontroller.cpp

    resources.qrc

    logindialog.cpp logindialog.ui
    registerdialog.cpp registerdialog.ui
    infobox.cpp infobox.ui
    activitybox.cpp activitybox.ui

    starcreator.h
    cameramanager.h
    qtmanager.h
    activitybox.h
    infobox.h
    registerdialog.h
    music.h
    firstpersoncameracontroller.h
    thirdpersoncameracontroller.h
    databasehandler.h
    logindialog.h
    includeqt.h
    includepr.h
)

# Link all Qt modules
target_link_libraries(SimpleShape PRIVATE
    # Core Qt modules
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Multimedia

    # Qt3D modules
    Qt6::3DCore      # For QEntity, QTransform
    Qt6::3DRender    # For textures and materials
    Qt6::3DExtras    # For QTextureMaterial and meshes
    Qt6::3DInput     # For camera controls
    Qt6::3DLogic     # For frame updates
)

# Installation settings (keep your existing)
include(GNUInstallDirs)
install(TARGETS SimpleShape
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
