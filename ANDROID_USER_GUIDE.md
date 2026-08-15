# GBAStation Android 简易上手指南

**适用版本：** Android APK，最低 Android 7.0（API 24）。
**适用人群：** 希望在 Android 手机、平板或 Android 掌机上管理并运行已支持 ROM 的用户。

## 1. 开始前请了解支持范围

GBAStation Android 版目前直接支持已经编入 APK 的 **GB、GBC、GBA、FC、SFC 和 MD** 核心。Nintendo Switch 版本所使用的 `.nro` 外置核心、`sdmc:` 路径、NDS、3DS、街机、Dreamcast、PSP、PS1、Saturn、GameCube 和 Wii 工作流当前不随 Android APK 提供。把这些未支持平台的 ROM 导入游戏库不会使其在 Android 上可运行。

| 你想做什么 | Android 版是否支持 | 正确做法 |
| --- | --- | --- |
| 运行 GB / GBC / GBA / FC / SFC / MD ROM | 支持 | 导入 ROM、设置扫描目录并扫描游戏库。 |
| 从下载、SD 卡或云盘选择 ROM | 支持 | 使用应用内的“从设备导入 ROM”，调用 Android 系统文件选择器。 |
| 直接浏览所有设备文件或 `Android/data` | 不支持，也不需要 | 由系统文件选择器授予所选文件访问权；不要要求“所有文件访问”。 |
| 使用 Switch `.nro` 外置核心 | 不支持 | 请使用 Switch 版本对应的 Release 包。 |

> 请只导入你有权使用的 ROM、BIOS 与游戏文件。GBAStation 不提供游戏内容或 BIOS 文件。

## 2. 第一次启动

安装并启动应用后，GBAStation 会初始化自己的配置、存档和游戏库目录。默认情况下，这些文件位于 Android 的**应用专属外部存储目录**，典型位置如下：

```text
/Android/data/com.beiklive.gbastation/files/GBAStation/
├── roms/          # 导入的 ROM
├── saves/         # 游戏存档和即时存档
├── screenshots/   # 截图
├── config/        # 应用设置和按键映射
├── data/          # 游戏库数据库
├── bios/          # 需要 BIOS 的核心文件
├── cheats/
├── shaders/
└── cache/
```

Android 设备和厂商文件管理器对 `Android/data` 的显示方式可能不同。这是 Android 分区存储的正常限制，不代表文件导入失败。若应用专属外部存储暂时不可写，例如外置卷不可用，应用会安全地回退到内部应用目录；这时仍可使用系统选择器导入 ROM，但目录不会在普通文件管理器中显示。

## 3. 导入 ROM

在应用中依次进入**数据管理**、**游戏库扫描**，然后选择**从设备导入 ROM**。Android 会显示系统文件选择器，你可以在下载目录、可移动存储、云盘提供商或其他系统允许的位置选择一个或多个 ROM。选择完成后，应用会把这些文件复制到自身的 `roms/` 目录，并显示导入结果。

这种导入方式遵循 Android 的分区存储模型：应用只读取你在系统选择器中明确选中的文件，然后在自己的目录中保存副本。它不会请求“所有文件访问”、相册读取权限或对其他应用私有目录的访问权。[1] [2]

导入完成后，仍需在同一个**游戏库扫描**页中为相应平台选择扫描目录。请选择 GBAStation 的 `roms/` 目录，然后点击**开始扫描**。如果你把不同平台的 ROM 分放在 `roms/` 的子文件夹中，请开启**扫描子目录**，或为各平台选择对应的子文件夹。扫描完成后，新游戏会出现在游戏库中。

| 任务 | 推荐步骤 |
| --- | --- |
| 导入少量 ROM | 使用“从设备导入 ROM”，选择文件后扫描 `roms/`。 |
| 导入大量 ROM | 可以分批选择多个文件；导入后按平台目录扫描。 |
| 从电脑传输 | 可使用 `adb push` 将文件放入应用专属 `roms/` 目录，然后在应用中扫描。 |
| 通过局域网管理 | 如已启用应用的 Web 管理功能，可通过该功能上传 ROM，再执行扫描。 |

## 4. 运行、存档与设置

扫描完成后，在游戏库中选择已支持平台的游戏即可启动。游戏的常规存档和即时存档保存在 `saves/`；截图保存在 `screenshots/`；按键映射和应用设置保存在 `config/`。请在退出游戏后等待应用完成存档写入，再关闭应用或重启设备。

Android 版不会显示 Switch 专有的外置核心 NRO 路径或“返回主程序路径”设置。这是为了避免在 Android 上生成不可运行的 `sdmc:` 或 `.nro` 配置。需要 BIOS 的功能应把相应文件放在应用的 `bios/` 目录，并在核心设置中确认路径。

## 5. 备份与卸载注意事项

应用专属存储中的文件会在卸载应用时被 Android 删除。因此，升级前通常无需手动迁移数据，但在**卸载、恢复出厂设置、更换设备或清除应用数据前**，请先备份 `saves/`、`config/` 和 `data/`。使用 USB、ADB 或设备文件管理工具复制这些目录时，请确认工具确实拥有对应用专属目录的访问能力。

> 不要把唯一存档只留在应用目录中。Android 官方将应用专属存储视为随应用生命周期清理的目录；希望独立于应用卸载长期保留的文件应由用户主动导出和备份。[1]

## 6. 常见问题

| 现象 | 处理建议 |
| --- | --- |
| 系统文件选择器没有显示某个位置 | Android 会限制系统根目录、`Android/data`、`Android/obb` 等位置；请在允许的位置选择 ROM，或先把文件放到下载目录、SD 卡或云盘。 |
| 已导入 ROM，但游戏库没有显示 | 进入“游戏库扫描”，为对应平台选择 `roms/` 或其子目录，再点击“开始扫描”。同时确认 ROM 的扩展名属于已支持平台。 |
| 导入时提示部分文件失败 | 检查设备剩余空间、文件是否仍可由其来源应用读取，以及文件是否被移动或删除。可重新在系统选择器中选择失败的文件。 |
| 普通文件管理器看不到 `Android/data` | 这是 Android 11 及以后常见限制。优先使用应用内导入、USB/ADB 或支持应用专属目录访问的工具。 |
| 卸载后存档不见了 | 应用专属目录会随卸载删除。请在卸载前备份 `saves/`、`config/` 和 `data/`。 |
| 想运行 PSP、Dreamcast 或 3DS | 这些平台当前不包含在 Android APK 中；请勿使用 Switch NRO 文件替代。 |

## 参考资料

[1]: https://developer.android.com/training/data-storage/app-specific "Android Developers: Access app-specific files"
[2]: https://developer.android.com/training/data-storage/shared/documents-files "Android Developers: Access documents and other files from shared storage"
[3]: https://developer.android.com/about/versions/11/privacy/storage "Android Developers: Storage updates in Android 11"
