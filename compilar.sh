#!/bin/bash

# ThetaMacExplorer build script
# Workaround for Qt 6.5.x on macOS / Apple Silicon: force Rosetta
if [ "$(uname -m)" = "arm64" ]; then
    echo "ARM64 detected. Restarting under Rosetta (x86_64) for Qt compatibility..."
    exec arch -x86_64 "$0" "$@"
fi

APP_NAME="ThetaMacExplorer"
QT_PATH="$HOME/Qt/6.5.3/macos"

echo "=== Building $APP_NAME ==="

# Kill existing process
pkill -f "$APP_NAME" 2>/dev/null && echo "Killed running instance." || true
sleep 0.5

# Create build dir
mkdir -p build
cd build

# El requerimiento de -framework AGL de Qt6 (via WrapOpenGL::WrapOpenGL) se
# neutraliza directamente en CMakeLists.txt, asi que no hace falta dummy AGL.

# Configure
cmake .. \
    -G "Unix Makefiles" \
    -DCMAKE_PREFIX_PATH="$QT_PATH" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
    -DCMAKE_OSX_ARCHITECTURES="x86_64"

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed."
    exit 1
fi

# Build
cmake --build . -- -j$(sysctl -n hw.logicalcpu)
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed."
    exit 1
fi

cd ..

# Deploy Qt dependencies
echo "Running macdeployqt..."
export PATH="$QT_PATH/bin:$PATH"

# Create local bin for otool-classic workaround (Xcode 16+)
mkdir -p build/local_bin
if [ ! -f "build/local_bin/otool-classic" ]; then
    OTOOL_PATH=$(xcrun -f otool)
    ln -sf "$OTOOL_PATH" "build/local_bin/otool-classic"
fi
export PATH="$PWD/build/local_bin:$PATH"

if [ ! -d "build/$APP_NAME.app/Contents/Frameworks" ] || [ ! -f "build/$APP_NAME.app/Contents/PlugIns/platforms/libqcocoa.dylib" ]; then
    "$QT_PATH/bin/macdeployqt" "build/$APP_NAME.app"

    # Manual platform plugin copy if macdeployqt missed it
    if [ ! -f "build/$APP_NAME.app/Contents/PlugIns/platforms/libqcocoa.dylib" ]; then
        mkdir -p "build/$APP_NAME.app/Contents/PlugIns/platforms"
        cp "$QT_PATH/plugins/platforms/"* "build/$APP_NAME.app/Contents/PlugIns/platforms/" 2>/dev/null || true
    fi

    # Image format plugins (jpeg, png for thumbnails)
    mkdir -p "build/$APP_NAME.app/Contents/PlugIns/imageformats"
    for fmt in libqjpeg.dylib libqpng.dylib libqtiff.dylib libqwebp.dylib; do
        [ -f "$QT_PATH/plugins/imageformats/$fmt" ] && \
            cp "$QT_PATH/plugins/imageformats/$fmt" "build/$APP_NAME.app/Contents/PlugIns/imageformats/" || true
    done
fi

echo ""
echo "=== Build complete. Launching $APP_NAME... ==="
echo ""
open "build/$APP_NAME.app"
