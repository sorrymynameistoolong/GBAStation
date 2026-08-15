# GBAStation

![logo](./resources/icon/default.png)

GBAStation is a multi-core emulator frontend for **Nintendo Switch and Android**. The main application manages the game library, file detection, configuration, input mapping, resources, and game launch flow. Smaller emulator cores are compiled into the application, so they do not rely on runtime loading of libretro dynamic cores.

The project was originally Switch-first. Its Switch build uses a split model: FC, SFC, MD, GBA, GB, GBC, and similar smaller cores are integrated into the main application, while NDS, 3DS, arcade, DC, PSP, and other large cores run as standalone `GBAStation*Stub.nro` applications and return to `sdmc:/switch/GBAStation.nro` after exit. NRO chain launching, NRO-path configuration, and NRO return paths are **Switch-only features** and are not exposed as usable Android settings.

The Android build currently focuses on the integrated cores that can run directly from the APK. Application data, saves, screenshots, databases, and the default ROM directory are stored in Android's app-specific external-files directory; the runtime no longer assumes that it can write to the filesystem root or to an `sdmc:` path.

## Runtime Support

| Emulated platform | Nintendo Switch | Android |
| --- | --- | --- |
| GB / GBC / GBA | Built-in core | Built-in core |
| FC | Built-in core | Built-in core |
| SFC | Built-in core | Built-in core |
| MD | Built-in core | Built-in core |
| NDS | `GBAStationNDSStub.nro` | Not included in the APK yet |
| 3DS | `GBAStation3DSStub.nro` | Not included in the APK yet |
| Arcade | `GBAStationFBNeoStub.nro` | Not included in the APK yet |
| Dreamcast | `GBAStationFlycastStub.nro` | Not included in the APK yet |
| PSP | `GBAStationPPSSPPStub.nro` | Not included in the APK yet |
| PS1 / Saturn / GameCube / Wii | Switch standalone-NRO workflow | Not included in the APK yet |

> Android does not open or configure `.nro` files, and it does not display external-core NRO or return-path settings. Adding a ROM for an unsupported platform to the library does not make that platform runnable on Android.

## Key Features

| Feature | Description |
| --- | --- |
| Game library management | Automatic scanning, recently played games, favorites, search, category filters, pinyin sorting, and batch deletion. |
| File detection | Detects games from file extensions and platform rules. |
| Built-in core execution | GB/GBC/GBA, FC, SFC, and MD run directly in the Switch and Android main application. |
| Switch chain loading | The Switch build can launch a standalone core NRO and return to the main application after it exits. |
| Configuration and input mapping | Per-platform settings with support for individual buttons, combinations, and multiple mapping groups. |
| GameDB and covers | Manages the game database, cover artwork, and metadata. |
| Web management | Upload ROMs, import saves, edit covers, and manage the library over the local network. |
| Runtime functions | Fast-forward, save/load state, cheats, display options, and core options are provided by integrated cores. |
| glslp shaders | Integrated smaller cores support glslp rendering chains and shader parameters. |

## Nintendo Switch SD Card Layout

After extracting a Switch release, keep the following structure:

```text
sdmc:/switch/GBAStation.nro
sdmc:/GBAStation/core/GBAStationNDSStub.nro
sdmc:/GBAStation/core/GBAStation3DSStub.nro
sdmc:/GBAStation/core/GBAStationFBNeoStub.nro
sdmc:/GBAStation/core/GBAStationFlycastStub.nro
sdmc:/GBAStation/core/GBAStationPPSSPPStub.nro
```

## Android Data Directory and ROM Import

At runtime, the Android build asks SDL for the app-specific external-files directory and creates its `GBAStation` working directory beneath it. For package name `com.beiklive.gbastation`, a typical path is shown below; the storage-volume prefix can vary by device.

```text
/storage/emulated/0/Android/data/com.beiklive.gbastation/files/GBAStation/
├── roms/          # Default location opened by Android file/directory pickers
├── saves/         # Game saves and save states
├── screenshots/   # Screenshots
├── config/        # Configuration and input mappings
├── data/          # Game-library database
├── bios/          # BIOS files for cores that require them
├── cache/
├── cheats/
└── shaders/
```

Android scoped storage does not guarantee that a native file browser can enumerate the system root. Android file and directory pickers therefore begin at the app's writable `roms/` directory. Prefer the application's web-management feature, `adb push`, or a device file manager that can access app-specific directories when importing ROMs. Do not use `sdmc:`, `/GBAStation/core/*.nro`, or the system root as Android write locations.

## Build

### Nintendo Switch

The devkitPro / devkitA64 environment is required:

```bash
cd BeikLiveStation
bash switchbuild.sh
```

Local builds copy external cores from neighboring project directories by default:

```text
../GBAStation_fbneo/GBAStationFBNeoStub.nro
../GBAStation_flycast/GBAStationFlycastStub.nro
../GBAStation_ppsspp/GBAStationPPSSPPStub.nro
../GBAStation_3DS/GBAStation3DSStub.nro
```

Build artifacts are generated at:

```text
build_switch/GBAStation.nro
build_switch/GBAStation/core/GBAStationNDSStub.nro
build_switch/GBAStation/core/GBAStation3DSStub.nro
build_switch/GBAStation/core/GBAStationFBNeoStub.nro
build_switch/GBAStation/core/GBAStationFlycastStub.nro
build_switch/GBAStation/core/GBAStationPPSSPPStub.nro
```

### Android

Android packaging requires JDK 21, CMake 3.22.1, Ninja, and an Android SDK containing API 36, Build Tools 36.0.0, and NDK 28.2.13676358. Set `ANDROID_SDK_ROOT` (or `ANDROID_HOME`) and run the following from the repository root:

```bash
# The default Debug APK packages both arm64-v8a and armeabi-v7a.
./androidbuild.sh debug

# Build a Release APK. It remains unsigned unless signing variables are supplied.
./androidbuild.sh release

# Optional: validate a single ABI locally to shorten build time.
GBASTATION_ANDROID_ABIS=arm64-v8a ./androidbuild.sh debug
```

The script builds the host-side `libromfs-generator`, prepares JNI source links, validates the fixed toolchain, invokes Gradle, and copies APKs into `dist/android/`. The default dual-ABI Debug APK contains `lib/arm64-v8a/libGBAStation.so` and `lib/armeabi-v7a/libGBAStation.so`. Release signing is optional and uses `GBASTATION_KEYSTORE`, `GBASTATION_STORE_PASSWORD`, `GBASTATION_KEY_ALIAS`, and `GBASTATION_KEY_PASSWORD` when all four variables are provided.

The [Android CI workflow](.github/workflows/build-android.yml) uses the pinned toolchain for pushes, pull requests, and manual runs. It builds the Debug APK, validates the native-library path for both ABIs, and uploads the APK with `SHA256SUMS`. See [ANDROID_PACKAGING.md](ANDROID_PACKAGING.md) for package architecture and verified build records. Android users can follow [ANDROID_USER_GUIDE.md](ANDROID_USER_GUIDE.md); see [ANDROID_SCOPED_STORAGE.md](ANDROID_SCOPED_STORAGE.md) for the storage-permission, app-directory, and SAF-import audit.

### Windows

The desktop version is used for frontend development and resource debugging. It cannot run external cores yet:

```bat
cd BeikLiveStation
windowsbuild.bat
```

## License

This project is released under the license declared in the [LICENSE](LICENSE) file. The main application, standalone cores, rendering backends, and third-party dependencies are each governed by their respective open-source licenses.

## Support the Author

If this project helps you, feel free to Star the project, submit Issues / PRs, or support development through the QR code below.

![pay](./assets/pay.png)

## Emulator Screenshots

![alt text](./assets/1.png)
![alt text](./assets/2.png)
![alt text](./assets/3.png)
![alt text](./assets/4.png)
