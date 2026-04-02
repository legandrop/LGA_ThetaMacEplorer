cmake_minimum_required(VERSION 3.16)
project(ThetaMacExplorer VERSION 1.0.0 LANGUAGES CXX OBJCXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum OS X deployment version")
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(MACOS_SDK_PATH)
        set(CMAKE_OSX_SYSROOT "${MACOS_SDK_PATH}" CACHE PATH "macOS SDK path")
        message(STATUS "macOS SDK path: ${MACOS_SDK_PATH}")
    endif()
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64")
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for OS X")
    else()
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for OS X")
    endif()
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isysroot ${CMAKE_OSX_SYSROOT}")
    set(CMAKE_OBJCXX_FLAGS "${CMAKE_OBJCXX_FLAGS} -isysroot ${CMAKE_OSX_SYSROOT}")
endif()

set(CMAKE_BUILD_TYPE Debug)
if(APPLE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_OBJCXX_FLAGS_DEBUG "${CMAKE_OBJCXX_FLAGS_DEBUG} -g -O0")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.5.3/mingw_64")
else()
    set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.5.3/macos")
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets Concurrent REQUIRED)

# ---- C++ Sources ----
set(SOURCES
    src/main.cpp
    src/services/ThetaCameraService.cpp
    src/services/DownloadManager.cpp
    src/services/HdrGroupingService.cpp
    src/ui/MainWindow.cpp
    src/ui/ThumbnailGridWidget.cpp
    src/ui/ThumbnailTileWidget.cpp
    src/ui/PreviewPanel.cpp
    src/ui/FileMetadataPanel.cpp
    src/ui/ConfirmDeleteDialog.cpp
    src/utils/Logger.cpp
    src/utils/ColorUtils.cpp
)

# ---- Objective-C++ bridge: SKIP_AUTOMOC prevents moc from processing .mm ----
set(OBJCXX_SOURCES
    src/bridge/ThetaBridge.mm
)
set_source_files_properties(${OBJCXX_SOURCES}
    PROPERTIES
        COMPILE_FLAGS "-x objective-c++ -fobjc-arc"
        SKIP_AUTOMOC ON
)

set(HEADERS
    include/thetaexplorer/ColorUtils.h
    include/thetaexplorer/CameraFileInfo.h
    include/thetaexplorer/MediaAssetGroup.h
    include/thetaexplorer/ThetaBridge.h
    include/thetaexplorer/ThetaCameraService.h
    include/thetaexplorer/HdrGroupingService.h
    include/thetaexplorer/MainWindow.h
    include/thetaexplorer/ThumbnailGridWidget.h
    include/thetaexplorer/ThumbnailTileWidget.h
    include/thetaexplorer/PreviewPanel.h
    include/thetaexplorer/FileMetadataPanel.h
    include/thetaexplorer/DownloadManager.h
    include/thetaexplorer/ConfirmDeleteDialog.h
    include/thetaexplorer/Logger.h
)

set(RESOURCES resources/resources.qrc)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${OBJCXX_SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework ImageCaptureCore"
        "-framework Foundation"
        "-framework AppKit"
        "-framework CoreFoundation"
        "-framework CoreGraphics"
    )
    target_link_options(${PROJECT_NAME} PRIVATE "LINKER:-weak_framework,AGL")

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "ThetaMacExplorer"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_IDENTIFIER "com.lga.thetamacexplorer"
        MACOSX_BUNDLE_EXECUTABLE_NAME "ThetaMacExplorer"
        MACOSX_BUNDLE_ICON_FILE "ThetaExplorer.icns"
    )

    # Copiar el icono al bundle (Contents/Resources/)
    set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/ThetaExplorer.icns")
    set_source_files_properties("${APP_ICON}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(${PROJECT_NAME} PRIVATE "${APP_ICON}")
endif()
