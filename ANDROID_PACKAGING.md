# GBAStation Android Packaging

**Author:** Manus AI
**Status:** Verified on GitHub Actions
**Target:** Android API 24 and above, with `arm64-v8a` and `armeabi-v7a` native libraries

## Overview

This repository now includes a reproducible Android packaging path for GBAStation. The implementation builds the C++ frontend, SDL2, Borealis, the embedded emulator cores, and packaged resources through Gradle and the Android NDK. It produces one debug APK containing both supported ARM application binary interfaces (ABIs).

The build is intentionally constrained to a single native compile/link job. GBAStation embeds a large ROM filesystem and several emulator cores; serializing the native graph avoids memory exhaustion on ordinary development machines and GitHub-hosted runners. The packaging script also builds the host-side `libromfs-generator` before Gradle invokes the Android CMake project.

| Area | Implemented behavior |
| --- | --- |
| APK variants | `debug` and `release` are supported by `androidbuild.sh`. |
| Default ABIs | `arm64-v8a` and `armeabi-v7a`. |
| ABI override | Set `GBASTATION_ANDROID_ABIS` for an intentionally narrower local build. |
| Android baseline | `compileSdk` 36, `minSdk` 24, Build Tools 36.0.0. |
| Native toolchain | NDK 28.2.13676358 and CMake 3.22.1. |
| Java/Gradle toolchain | JDK 21, Android Gradle Plugin 9.3.1, Gradle 9.5.0. |
| CI output | A 30-day GitHub Actions artifact holding the APK and `SHA256SUMS`. |

> The checked-in workflow validates both APK library paths before uploading any artifact. A green workflow therefore confirms not merely that Gradle completed, but that the packaged APK contains `lib/arm64-v8a/libGBAStation.so` and `lib/armeabi-v7a/libGBAStation.so`.

## Build Entry Points

The project-level [`androidbuild.sh`](androidbuild.sh) is the authoritative build entry point. It validates the selected SDK/NDK toolchain, creates the JNI symlink layout expected by the Android module, builds the host `libromfs-generator`, passes the ABI selection into Gradle, and runs `assembleDebug` or `assembleRelease`.

```bash
# From the repository root.
./androidbuild.sh debug

# Optional: build only one ABI locally.
GBASTATION_ANDROID_ABIS=arm64-v8a ./androidbuild.sh debug
```

The default dual-ABI configuration is defined in [`android-project/app/build.gradle`](android-project/app/build.gradle). It accepts the `gbastationAbis` Gradle property emitted by the script, supplies Android API and NDK versions, and configures the `externalNativeBuild` CMake invocation.

## Core Configuration Changes

| File | Responsibility | Key Android-specific change |
| --- | --- | --- |
| [`androidbuild.sh`](androidbuild.sh) | Reproducible packaging entry point | Builds the host resource generator, prepares JNI links, supports debug/release variants and ABI overrides, then calls Gradle. |
| [`android-project/app/build.gradle`](android-project/app/build.gradle) | Android application module | Defines API 36, `minSdk` 24, NDK 28.2.13676358, the dual ABI default, and CMake integration. |
| [`android-project/app/jni/CMakeLists.txt`](android-project/app/jni/CMakeLists.txt) | Native Android graph | Builds SDL2/Borealis, locates the generated resource tool, serializes native compile/link work, and disables mGBA host SQLite discovery. |
| [`android-project/gradle.properties`](android-project/gradle.properties) | Gradle resource controls | Limits the Gradle JVM heap to 1536 MiB and uses no more than two Gradle workers to prevent build-memory pressure. |
| [`android-project/gradle/wrapper/gradle-wrapper.properties`](android-project/gradle/wrapper/gradle-wrapper.properties) | Gradle distribution pin | Pins Gradle 9.5.0. |
| [`android-project/app/src/main/AndroidManifest.xml`](android-project/app/src/main/AndroidManifest.xml) | Android manifest | Updates storage permission declarations and removes the incompatible legacy `extractNativeLibs` attribute. |
| [`android-project/app/src/main/java/com/beiklive/gbastation/GBAStationActivity.java`](android-project/app/src/main/java/com/beiklive/gbastation/GBAStationActivity.java) | Native launcher | Loads SDL2 followed by GBAStation’s native library. |
| [`.github/workflows/build-android.yml`](.github/workflows/build-android.yml) | Continuous integration | Installs the fixed JDK/SDK/NDK/CMake versions, creates the dual-ABI debug APK, checks its contents, writes checksums, and uploads the result. |

## Native Porting Changes

GBAStation was originally centered on the Nintendo Switch build. The Android port adds platform-safe implementations or conditional paths where the original desktop/Switch code relied on unavailable libraries or incompatible graphics APIs.

| Component | Relevant files | Android adaptation |
| --- | --- | --- |
| SDL integration | `third_party/SDL`, `android-project/app/jni/CMakeLists.txt` | Uses **SDL2 2.32.10**, which matches Borealis’s SDL2 API usage. |
| Update and artwork networking | `src/core/AppUpdaterAndroid.cpp`, `src/core/SteamGridDbAndroid.cpp`, `src/ui/page/AboutPage.cpp`, `CMakeLists.txt` | Uses Android stubs and excludes libcurl-dependent code paths. |
| Pico-8 video | `src/core/pico8/Pico8Video.cpp` | Uses NanoVG’s GLES2 texture path instead of the GL3-specific backend. |
| ROM filesystem | `third_party/borealis/library/lib/extern/libromfs/generator/source/main.cpp`, `third_party/borealis/library/lib/extern/libromfs/lib/CMakeLists.txt` | Generates separate resource-source chunks rather than a single very large C++ translation unit, reducing peak compiler memory use. |
| FFmpeg and libretro cross compilation | `third_party/CMakeLists.txt`, `third_party/configure_ffmpeg_minimal.sh` | Uses the NDK target triples and sysroot for Android cross compilation and relocatable core linkage. |
| Emulator-core compatibility | `src/emulator/genesis/CMakeLists.txt`, `third_party/snes9x/unzip/ioapi.c`, `third_party/nestopia/libretro/libretro.cpp` | Corrects Android paths/includes, POSIX file APIs, and Clang narrowing conversions. |

## CI Design and Verification

The workflow runs on Ubuntu 24.04 for pushes and pull requests, and can also be started manually. It pins the Android pieces instead of relying on the runner image’s mutable defaults. The sequence checks out the source, installs JDK 21 and the Android command-line tools, accepts licenses, installs API 36/Build Tools 36.0.0/NDK 28.2.13676358/CMake 3.22.1, runs the build script, inspects the APK’s native-library paths, and uploads the APK plus its checksum manifest.

The first hosted run exposed a cross-compilation defect that did not appear locally: CMake’s generic mGBA feature probe found the runner’s **host** SQLite installation, but the Android NDK sysroot does not contain `sqlite3.h`. The final fix, commit [`07b3abda`](https://github.com/sorrymynameistoolong/GBAStation/commit/07b3abdafaa09cf54e6e9a4e9ef146d32256ae5c), explicitly disables mGBA’s optional SQLite game-database feature for Android. This prevents host headers or libraries from entering the NDK build graph.

| Verification item | Result |
| --- | --- |
| Latest commit | [`07b3abda`](https://github.com/sorrymynameistoolong/GBAStation/commit/07b3abdafaa09cf54e6e9a4e9ef146d32256ae5c) — `fix(android): disable mGBA host SQLite discovery`. |
| Hosted workflow | [Run 31887929319](https://github.com/sorrymynameistoolong/GBAStation/actions/runs/31887929319) completed successfully. |
| Build step | **Success**; dual-ABI Gradle/CMake packaging completed at 2026-08-15 14:10 UTC. |
| APK content check | **Success**; confirmed `libGBAStation.so` for both `arm64-v8a` and `armeabi-v7a`. |
| Artifact upload | **Success**; `GBAStation-Android-debug-07b3abdafaa09cf54e6e9a4e9ef146d32256ae5c`, including the APK and `SHA256SUMS`. |
| Artifact retention | 30 days from the successful workflow run. |

The workflow artifact can be downloaded from the successful run page while its retention period remains active. The artifact archive’s GitHub-reported SHA-256 is `a4811bd5b4c06c53f8f7155b4bb575222dc07ae3b1b003fa62121d36bb0715c2`.

## References

[1]: https://github.com/sorrymynameistoolong/GBAStation "GBAStation fork"
[2]: https://github.com/sorrymynameistoolong/GBAStation/blob/main/.github/workflows/build-android.yml "Android build workflow"
[3]: https://github.com/sorrymynameistoolong/GBAStation/actions/runs/31887929319 "Successful GitHub Actions run 31887929319"
[4]: https://github.com/sorrymynameistoolong/GBAStation/commit/07b3abdafaa09cf54e6e9a4e9ef146d32256ae5c "mGBA SQLite Android cross-compilation fix"
