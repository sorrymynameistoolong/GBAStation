# GBAStation

![logo](./resources/icon/default.png)

GBAStation 是一个面向 **Nintendo Switch 与 Android** 的多核心模拟器前端。主程序负责游戏库、文件识别、配置管理、按键映射、资源管理和游戏启动；小型核心直接编译到主程序中，不依赖运行时加载 libretro 动态核心。

项目最初以 Switch 为主。Switch 版本采用“小型核心内置 + 大型核心独立 NRO”的架构：FC、SFC、MD、GBA、GB、GBC 等核心直接集成在主程序中；NDS、3DS、街机、DC、PSP 等较大型核心通过 `GBAStation*Stub.nro` 独立运行，退出后返回 `sdmc:/switch/GBAStation.nro`。这类 NRO 链式启动、NRO 路径配置和返回路径均为 **Switch 专有功能**，不会出现在 Android 的可用配置中。

Android 版本目前聚焦于已内置、可由 APK 直接运行的核心。应用数据、存档、截图、数据库和默认 ROM 目录均使用 Android 的应用专属外部存储目录，不再假定进程可写入文件系统根目录或 `sdmc:` 路径。

## 运行时支持范围

| 模拟平台 | Nintendo Switch | Android |
| --- | --- | --- |
| GB / GBC / GBA | 主程序内置核心 | 主程序内置核心 |
| FC | 主程序内置核心 | 主程序内置核心 |
| SFC | 主程序内置核心 | 主程序内置核心 |
| MD | 主程序内置核心 | 主程序内置核心 |
| NDS | `GBAStationNDSStub.nro` | 当前未随 APK 提供 |
| 3DS | `GBAStation3DSStub.nro` | 当前未随 APK 提供 |
| 街机 | `GBAStationFBNeoStub.nro` | 当前未随 APK 提供 |
| Dreamcast | `GBAStationFlycastStub.nro` | 当前未随 APK 提供 |
| PSP | `GBAStationPPSSPPStub.nro` | 当前未随 APK 提供 |
| PS1 / Saturn / GameCube / Wii | Switch 外置 NRO 工作流 | 当前未随 APK 提供 |

> Android 版本不会尝试打开或配置 `.nro` 文件，也不会显示外置核心的 NRO 路径和返回路径设置。将未支持平台的 ROM 加入游戏库不等于该平台可以在 Android 上启动。

## 主要功能

| 功能 | 说明 |
| --- | --- |
| 游戏库管理 | 自动扫描、最近游玩、收藏、搜索、分类筛选、拼音排序和批量删除。 |
| 文件识别 | 根据扩展名和平台规则识别游戏文件。 |
| 内置核心运行 | GB/GBC/GBA、FC、SFC 与 MD 可由 Switch 和 Android 主程序直接运行。 |
| Switch 链式调用 | Switch 版可从主程序启动独立核心 NRO，并在核心退出后返回主程序。 |
| 配置与按键映射 | 按平台独立配置，支持单键、多键组合和多组映射。 |
| GameDB 与封面 | 管理游戏数据库、封面和元数据。 |
| Web 管理 | 在局域网内上传 ROM、导入存档、修改封面并管理游戏库。 |
| 运行时功能 | 快进、即时存档、读档、金手指、画面和核心设置由已集成的核心提供。 |
| glslp 着色器 | 内置小核心支持 glslp 渲染链和着色器参数。 |

## Nintendo Switch SD 卡目录

Switch Release 包解压后应保持以下结构：

```text
sdmc:/switch/GBAStation.nro
sdmc:/GBAStation/core/GBAStationNDSStub.nro
sdmc:/GBAStation/core/GBAStation3DSStub.nro
sdmc:/GBAStation/core/GBAStationFBNeoStub.nro
sdmc:/GBAStation/core/GBAStationFlycastStub.nro
sdmc:/GBAStation/core/GBAStationPPSSPPStub.nro
```

## Android 数据目录与 ROM 导入

Android 运行时通过 SDL 获取应用专属外部文件目录，并在其下创建 `GBAStation` 工作目录。对于包名 `com.beiklive.gbastation`，典型路径如下；实际存储卷名称可能因设备不同而变化。

```text
/storage/emulated/0/Android/data/com.beiklive.gbastation/files/GBAStation/
├── roms/          # Android 文件选择器的默认目录
├── saves/         # 游戏存档与即时存档
├── screenshots/   # 截图
├── config/        # 配置与按键映射
├── data/          # 游戏库数据库
├── bios/          # 需要 BIOS 的核心文件
├── cache/
├── cheats/
└── shaders/
```

Android 的分区存储机制不保证应用能以原生文件浏览器方式遍历系统根目录。因此 Android 中的文件和目录选择器会从应用的 `roms/` 目录开始。请优先通过应用的 Web 管理功能、`adb push`，或能够访问应用专属目录的设备文件管理工具导入 ROM；不要依赖 `sdmc:`、`/GBAStation/core/*.nro` 或系统根目录作为 Android 的可写位置。

## 构建

### Nintendo Switch

需要 devkitPro / devkitA64 环境：

```bash
cd BeikLiveStation
bash switchbuild.sh
```

本地构建默认从相邻项目目录复制外部核心：

```text
../GBAStation_fbneo/GBAStationFBNeoStub.nro
../GBAStation_flycast/GBAStationFlycastStub.nro
../GBAStation_ppsspp/GBAStationPPSSPPStub.nro
../GBAStation_3DS/GBAStation3DSStub.nro
```

构建产物位于：

```text
build_switch/GBAStation.nro
build_switch/GBAStation/core/GBAStationNDSStub.nro
build_switch/GBAStation/core/GBAStation3DSStub.nro
build_switch/GBAStation/core/GBAStationFBNeoStub.nro
build_switch/GBAStation/core/GBAStationFlycastStub.nro
build_switch/GBAStation/core/GBAStationPPSSPPStub.nro
```

### Android

Android 打包要求 JDK 21、CMake 3.22.1、Ninja，以及包含 Android API 36、Build Tools 36.0.0 和 NDK 28.2.13676358 的 Android SDK。设置 `ANDROID_SDK_ROOT`（或 `ANDROID_HOME`）后，从仓库根目录执行：

```bash
# 默认构建同时包含 arm64-v8a 与 armeabi-v7a 的 Debug APK。
./androidbuild.sh debug

# 生成 Release APK；未提供签名变量时，产物保持未签名。
./androidbuild.sh release

# 可选：仅在本地验证一个 ABI，以缩短编译时间。
GBASTATION_ANDROID_ABIS=arm64-v8a ./androidbuild.sh debug
```

构建脚本会先编译主机端 `libromfs-generator`，再准备 JNI 源码链接、验证工具链、调用 Gradle，并将 APK 复制到 `dist/android/`。默认双 ABI Debug APK 包含 `lib/arm64-v8a/libGBAStation.so` 和 `lib/armeabi-v7a/libGBAStation.so`。Release 签名可选地由 `GBASTATION_KEYSTORE`、`GBASTATION_STORE_PASSWORD`、`GBASTATION_KEY_ALIAS` 与 `GBASTATION_KEY_PASSWORD` 四个环境变量提供。

仓库中的 [Android CI 工作流](.github/workflows/build-android.yml) 在 push、Pull Request 和手动触发时使用固定工具链构建 Debug APK，检查两个 ABI 的原生库路径，并上传 APK 与 `SHA256SUMS` 构件。更多打包设计和已验证的构建记录请见 [ANDROID_PACKAGING.md](ANDROID_PACKAGING.md)。

### Windows

桌面版本用于前端开发和资源调试，暂时无法运行外置核心：

```bat
cd BeikLiveStation
windowsbuild.bat
```

## 许可证

本项目以 [LICENSE](LICENSE) 文件中声明的许可证发布。主程序、独立核心、渲染后端和第三方依赖分别遵循各自的开源许可证。

## 支持作者

如果这个项目对你有帮助，欢迎 Star 项目、提交 Issue / PR，或通过下方二维码支持开发。

![pay](./assets/pay.png)

## 模拟器截图展示

![alt text](./assets/1.png)
![alt text](./assets/2.png)
![alt text](./assets/3.png)
![alt text](./assets/4.png)
