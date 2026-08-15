#!/usr/bin/env bash
# Build GBAStation Android APKs with Gradle, CMake, SDL2 and the Android NDK.
#
# Usage:
#   ./androidbuild.sh [debug|release]
#
# Required environment:
#   ANDROID_SDK_ROOT (or ANDROID_HOME) pointing to an SDK that contains:
#     platform android-36, build-tools 36.0.0, CMake 3.22.1 and NDK 28.2.13676358.
#
# Set GBASTATION_ANDROID_ABIS to a comma-separated subset (for example,
# arm64-v8a) to speed up a local validation build. By default, both arm64-v8a
# and armeabi-v7a APK libraries are packaged.
#
# Release signing is optional. When all four values below are supplied, the
# release artifact is signed by Gradle; otherwise the normal unsigned release
# APK is emitted for local signing:
#   GBASTATION_KEYSTORE, GBASTATION_STORE_PASSWORD,
#   GBASTATION_KEY_ALIAS, GBASTATION_KEY_PASSWORD
set -euo pipefail

VARIANT="${1:-debug}"
case "${VARIANT}" in
    debug|release) ;;
    *)
        echo "Usage: $0 [debug|release]" >&2
        exit 64
        ;;
esac

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ANDROID_PROJECT="${SCRIPT_DIR}/android-project"
JNI_DIR="${ANDROID_PROJECT}/app/jni"
SDK_ROOT="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
LIBROMFS_SOURCE="${SCRIPT_DIR}/third_party/borealis/library/lib/extern/libromfs/generator"
LIBROMFS_BUILD="${SCRIPT_DIR}/build_libromfs_generator"
LIBROMFS_BINARY="${SCRIPT_DIR}/libromfs-generator"
DIST_DIR="${SCRIPT_DIR}/dist/android"

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "ERROR: required command '$1' was not found in PATH." >&2
        exit 127
    fi
}

require_command cmake
require_command ninja
require_command java

if [[ -z "${SDK_ROOT}" || ! -d "${SDK_ROOT}" ]]; then
    cat >&2 <<'EOF'
ERROR: ANDROID_SDK_ROOT (or ANDROID_HOME) must point to a valid Android SDK.
Install the following packages before building:
  platforms;android-36
  build-tools;36.0.0
  cmake;3.22.1
  ndk;28.2.13676358
EOF
    exit 2
fi

export ANDROID_SDK_ROOT="${SDK_ROOT}"
export ANDROID_HOME="${SDK_ROOT}"

printf '[1/5] Building libromfs-generator...\n'
rm -rf "${LIBROMFS_BUILD}"
cmake -S "${LIBROMFS_SOURCE}" -B "${LIBROMFS_BUILD}" -G Ninja
cmake --build "${LIBROMFS_BUILD}" --parallel
install -m 0755 "${LIBROMFS_BUILD}/libromfs-generator" "${LIBROMFS_BINARY}"
rm -rf "${LIBROMFS_BUILD}"

printf '[2/5] Creating JNI source links...\n'
mkdir -p "${JNI_DIR}"
ln -sfn ../../../third_party/borealis/library/lib/extern/SDL "${JNI_DIR}/SDL"
ln -sfn ../../.. "${JNI_DIR}/borealis"

printf '[3/5] Validating Android toolchain...\n'
for required_path in \
    "${SDK_ROOT}/platforms/android-36" \
    "${SDK_ROOT}/build-tools/36.0.0" \
    "${SDK_ROOT}/cmake/3.22.1" \
    "${SDK_ROOT}/ndk/28.2.13676358"; do
    if [[ ! -d "${required_path}" ]]; then
        echo "ERROR: missing Android SDK component: ${required_path}" >&2
        exit 2
    fi
done
if [[ ! -x "${JNI_DIR}/borealis/libromfs-generator" ]]; then
    echo "ERROR: libromfs-generator was not linked into the JNI source tree." >&2
    exit 2
fi

printf '[4/5] Building %s APK...\n' "${VARIANT}"
cd "${ANDROID_PROJECT}"
chmod +x ./gradlew
GRADLE_ARGS=(--no-daemon)
if [[ -n "${GBASTATION_ANDROID_ABIS:-}" ]]; then
    GRADLE_ARGS+=("-PgbastationAbis=${GBASTATION_ANDROID_ABIS}")
fi

if [[ "${VARIANT}" == "release" ]]; then
    ./gradlew "${GRADLE_ARGS[@]}" assembleRelease
else
    ./gradlew "${GRADLE_ARGS[@]}" assembleDebug
fi

printf '[5/5] Collecting artifacts...\n'
mkdir -p "${DIST_DIR}"
VERSION_NAME="$(sed -n "s/.*versionName = '\([^']*\)'.*/\1/p" app/build.gradle | head -n 1)"
[[ -n "${VERSION_NAME}" ]] || VERSION_NAME="dev"
APK_DIR="${ANDROID_PROJECT}/app/build/outputs/apk/${VARIANT}"
mapfile -t APK_FILES < <(find "${APK_DIR}" -maxdepth 1 -type f -name '*.apk' -print | sort)
if [[ "${#APK_FILES[@]}" -eq 0 ]]; then
    echo "ERROR: Gradle completed but produced no ${VARIANT} APK." >&2
    exit 3
fi

for apk in "${APK_FILES[@]}"; do
    base_name="$(basename "${apk}" .apk)"
    output="${DIST_DIR}/GBAStation-v${VERSION_NAME}-${base_name}.apk"
    cp -f "${apk}" "${output}"
    printf '  %s\n' "${output}"
done

if [[ "${VARIANT}" == "release" && -z "${GBASTATION_KEYSTORE:-}" ]]; then
    echo "NOTE: The release APK is unsigned. Set GBASTATION_KEYSTORE and related signing variables for a signed release." >&2
fi
