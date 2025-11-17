#!/bin/bash

set -euo pipefail

ARCH="${1:-x86_64}"
BUILD_DIR="build"
APP_PATH="${BUILD_DIR}/iDescriptor.app"

echo "Starting deployment for architecture: ${ARCH}"

# Deploy Qt dependencies
echo "Deploying Qt dependencies..."
macdeployqt "${APP_PATH}" -qmldir=qml -verbose=2

# Bundle GStreamer
echo "Bundling GStreamer..."
GST_PLUGIN_DIR="${APP_PATH}/Contents/Frameworks/gstreamer"
mkdir -p "${GST_PLUGIN_DIR}"

PLUGINS=(
  "libgstapp"
  "libgstaudioconvert"
  "libgstautodetect"
  "libgstavi"
  "libgstcoreelements"
  "libgstlevel"
  "libgstlibav"
  "libgstosxaudio"
  "libgstplayback"
  "libgstvolume"
)

for plugin in "${PLUGINS[@]}"; do
  cp "$(brew --prefix gstreamer)/lib/gstreamer-1.0/${plugin}.dylib" "${GST_PLUGIN_DIR}/"
done

cp "$(brew --prefix gstreamer)/libexec/gstreamer-1.0/gst-plugin-scanner" "${APP_PATH}/Contents/Frameworks/"

# Bundle libjxl_cms
echo "Bundling libjxl_cms..."
cp "$(brew --prefix)/lib/libjxl_cms.0.11.dylib" "${APP_PATH}/Contents/Frameworks/"
install_name_tool -id "@rpath/libjxl_cms.0.11.dylib" "${APP_PATH}/Contents/Frameworks/libjxl_cms.0.11.dylib"
install_name_tool -change "$(brew --prefix)/lib/libjxl_cms.0.11.dylib" "@rpath/libjxl_cms.0.11.dylib" "${APP_PATH}/Contents/Frameworks/libjxl.0.11.dylib"

# Add rpath to main executable
echo "Adding rpath to main executable..."
install_name_tool -add_rpath "@executable_path/../Frameworks" "${APP_PATH}/Contents/MacOS/iDescriptor"

# Fix GStreamer library paths
echo "Fixing GStreamer library paths..."
FRAMEWORKS_DIR="${APP_PATH}/Contents/Frameworks"
BREW_PREFIX="$(brew --prefix)"

# Copy GStreamer core libraries
GST_LIBS=(
  "libgstreamer-1.0.0.dylib"
  "libgstbase-1.0.0.dylib"
  "libgstaudio-1.0.0.dylib"
  "libgstvideo-1.0.0.dylib"
  "libgstapp-1.0.0.dylib"
  "libgstpbutils-1.0.0.dylib"
  "libgsttag-1.0.0.dylib"
  "libgstriff-1.0.0.dylib"
  "libgstcodecparsers-1.0.0.dylib"
  "libglib-2.0.0.dylib"
  "libgobject-2.0.0.dylib"
  "libgmodule-2.0.0.dylib"
  "libgio-2.0.0.dylib"
  "libgthread-2.0.0.dylib"
)

for lib in "${GST_LIBS[@]}"; do
  if [ -f "${BREW_PREFIX}/lib/${lib}" ]; then
    cp "${BREW_PREFIX}/lib/${lib}" "${FRAMEWORKS_DIR}/"
    install_name_tool -id "@rpath/${lib}" "${FRAMEWORKS_DIR}/${lib}"
    echo "Copied and fixed ID for ${lib}"
  fi
done

# For some reason libavfilter sometimes doesnt get copied by macdeployqt 
FFMPEG_LIB_DIR="$(brew --prefix ffmpeg)/lib"
cp "${FFMPEG_LIB_DIR}"/libavfilter.*.dylib "${FRAMEWORKS_DIR}/"

# Copy additional audio-related libraries that are missing
echo "Bundling additional audio libraries..."
ADDITIONAL_LIBS=(
  "libarchive.13.dylib"
  "libass.9.dylib" 
  "libb2.1.dylib"
  "libfribidi.0.dylib"
  "libgif.dylib"
  "libgraphite2.3.dylib"
  "libharfbuzz.0.dylib"
  "libjpeg.8.dylib"
  "liblcms2.2.dylib"
  "libleptonica.6.dylib"
  "liblz4.1.dylib"
  "librubberband.3.dylib"
  "libsamplerate.0.dylib"
  "libtesseract.5.dylib"
  "libtiff.6.dylib"
  "libunibreak.6.dylib"
  "libvidstab.1.2.dylib"
  "libzimg.2.dylib"
)

for lib in "${ADDITIONAL_LIBS[@]}"; do
  if [ -f "${BREW_PREFIX}/lib/${lib}" ]; then
    cp "${BREW_PREFIX}/lib/${lib}" "${FRAMEWORKS_DIR}/"
    install_name_tool -id "@rpath/${lib}" "${FRAMEWORKS_DIR}/${lib}"
  else
    echo "Warning: ${lib} not found, skipping..."
  fi
done

# Fix dependencies in all GStreamer plugins
echo "Fixing GStreamer plugin dependencies..."
for plugin in "${GST_PLUGIN_DIR}"/*.dylib; do
  echo "Fixing plugin: $(basename "${plugin}")"
  
  # Get all dependencies and fix them
  otool -L "${plugin}" | grep -E "${BREW_PREFIX}" | awk '{print $1}' | while read dep; do
    depname=$(basename "${dep}")
    echo "  Changing ${depname}"
    install_name_tool -change "${dep}" "@rpath/${depname}" "${plugin}" 2>/dev/null || true
  done
done

# Fix dependencies in GStreamer core libraries themselves
echo "Fixing GStreamer core library dependencies..."
for lib in "${FRAMEWORKS_DIR}"/libgst*.dylib "${FRAMEWORKS_DIR}"/libglib*.dylib "${FRAMEWORKS_DIR}"/libgobject*.dylib "${FRAMEWORKS_DIR}"/libgmodule*.dylib "${FRAMEWORKS_DIR}"/libgio*.dylib "${FRAMEWORKS_DIR}"/libgthread*.dylib; do
  if [ -f "${lib}" ]; then
    echo "Fixing library: $(basename "${lib}")"
    
    otool -L "${lib}" | grep -E "${BREW_PREFIX}" | awk '{print $1}' | while read dep; do
      depname=$(basename "${dep}")
      echo "  Changing ${depname}"
      install_name_tool -change "${dep}" "@rpath/${depname}" "${lib}" 2>/dev/null || true
    done
  fi
done

# Create DMG
echo "Creating DMG..."
create-dmg \
  --volname "iDescriptor" \
  --volicon "resources/icons/app-icon/icon.icns" \
  --window-pos 200 120 \
  --window-size 600 400 \
  --icon-size 100 \
  --icon "iDescriptor.app" 175 190 \
  --hide-extension "iDescriptor.app" \
  --app-drop-link 425 190 \
  "${BUILD_DIR}/iDescriptor-macOS-${ARCH}.dmg" \
  "${APP_PATH}"

echo "Deployment complete for architecture: ${ARCH}"
