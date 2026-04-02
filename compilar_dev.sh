#!/bin/bash

set -u

if [ "$(uname -m)" = "arm64" ]; then
    echo "ARM64 detected. Restarting under Rosetta (x86_64) for Qt compatibility..."
    exec arch -x86_64 "$0" "$@"
fi

APP_NAME="ThetaMacExplorer"
QT_PATH="$HOME/Qt/6.5.3/macos"
BUILD_DIR="build"
APP_BUNDLE="$BUILD_DIR/$APP_NAME.app"
APP_BIN="$APP_BUNDLE/Contents/MacOS/$APP_NAME"

show_help() {
    echo "Uso: $0 [--force-clean] [--parallel N] [--no-deploy] [--no-run]"
    echo ""
    echo "Opciones:"
    echo "  --force-clean    Borra build y reconfigura desde cero"
    echo "  --parallel N     Usa N núcleos para compilar"
    echo "  --no-deploy      Salta macdeployqt y copias de plugins"
    echo "  --no-run         Compila pero no lanza la app"
}

FORCE_CLEAN=false
NO_DEPLOY=false
NO_RUN=false
PARALLEL_CORES=$(sysctl -n hw.logicalcpu)

while [[ $# -gt 0 ]]; do
    case "$1" in
        --force-clean)
            FORCE_CLEAN=true
            shift
            ;;
        --parallel)
            PARALLEL_CORES="$2"
            shift 2
            ;;
        --no-deploy)
            NO_DEPLOY=true
            shift
            ;;
        --no-run)
            NO_RUN=true
            shift
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            echo "Opción desconocida: $1"
            show_help
            exit 1
            ;;
    esac
done

echo "=== Fast dev build for $APP_NAME ($PARALLEL_CORES cores) ==="

if [ "$FORCE_CLEAN" = "true" ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

pkill -f "$APP_NAME" 2>/dev/null || true
sleep 0.3

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

EXTRA_CMAKE_ARGS=()
if [ ! -f "/System/Library/Frameworks/AGL.framework/Versions/A/AGL" ]; then
    if [ ! -f "AGL.framework/Versions/A/AGL" ]; then
        echo "Creating dummy AGL framework..."
        mkdir -p AGL.framework/Versions/A
        echo "void _aglDummy(){}" > agl_dummy.c
        clang -dynamiclib agl_dummy.c -o AGL.framework/Versions/A/AGL \
            -install_name /System/Library/Frameworks/AGL.framework/Versions/A/AGL \
            -arch x86_64
        ln -sf A AGL.framework/Versions/Current
        ln -sf Versions/Current/AGL AGL.framework/AGL
        rm agl_dummy.c
    fi
    EXTRA_CMAKE_ARGS+=("-DCMAKE_EXE_LINKER_FLAGS=-F$PWD")
fi

CMAKE_FLAGS=(
    -G "Unix Makefiles"
    -DCMAKE_PREFIX_PATH="$QT_PATH"
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
    -DCMAKE_OSX_ARCHITECTURES="x86_64"
    -DCMAKE_BUILD_TYPE=Debug
    -DQt6_DIR="$QT_PATH/lib/cmake/Qt6"
    -DCMAKE_CXX_FLAGS_DEBUG="-g -O0 -Wno-unused-parameter"
    "${EXTRA_CMAKE_ARGS[@]}"
)

if [ ! -f "CMakeCache.txt" ] || [ "$FORCE_CLEAN" = "true" ]; then
    echo "Configuring CMake..."
    cmake .. "${CMAKE_FLAGS[@]}" || exit 1
fi

echo "Building..."
cmake --build . --parallel "$PARALLEL_CORES" || exit 1

cd .. || exit 1

if [ "$NO_DEPLOY" = "false" ]; then
    export PATH="$QT_PATH/bin:$PATH"

    mkdir -p "$BUILD_DIR/local_bin"
    if [ ! -f "$BUILD_DIR/local_bin/otool-classic" ]; then
        OTOOL_PATH=$(xcrun -f otool)
        ln -sf "$OTOOL_PATH" "$BUILD_DIR/local_bin/otool-classic"
    fi
    export PATH="$PWD/$BUILD_DIR/local_bin:$PATH"

    NEEDS_DEPLOY=false
    if [ ! -d "$APP_BUNDLE/Contents/Frameworks" ]; then
        NEEDS_DEPLOY=true
    elif [ ! -f "$APP_BUNDLE/Contents/PlugIns/platforms/libqcocoa.dylib" ]; then
        NEEDS_DEPLOY=true
    elif [ ! -f "$APP_BUNDLE/Contents/PlugIns/imageformats/libqjpeg.dylib" ]; then
        NEEDS_DEPLOY=true
    fi

    if [ "$NEEDS_DEPLOY" = "true" ]; then
        echo "Deploying minimal Qt runtime..."
        "$QT_PATH/bin/macdeployqt" "$APP_BUNDLE" || exit 1

        if [ ! -f "$APP_BUNDLE/Contents/PlugIns/platforms/libqcocoa.dylib" ]; then
            mkdir -p "$APP_BUNDLE/Contents/PlugIns/platforms"
            cp "$QT_PATH/plugins/platforms/"* "$APP_BUNDLE/Contents/PlugIns/platforms/" 2>/dev/null || true
        fi

        mkdir -p "$APP_BUNDLE/Contents/PlugIns/imageformats"
        for fmt in libqjpeg.dylib libqpng.dylib libqtiff.dylib libqwebp.dylib; do
            [ -f "$QT_PATH/plugins/imageformats/$fmt" ] && \
                cp "$QT_PATH/plugins/imageformats/$fmt" "$APP_BUNDLE/Contents/PlugIns/imageformats/" || true
        done
    else
        echo "Qt runtime already present. Skipping deploy."
    fi
else
    echo "Skipping deploy (--no-deploy)"
fi

echo "Build complete."

if [ "$NO_RUN" = "true" ]; then
    exit 0
fi

if [ ! -x "$APP_BIN" ]; then
    echo "ERROR: Missing app executable at $APP_BIN"
    exit 1
fi

echo "Launching $APP_NAME..."
exec "$APP_BIN"
