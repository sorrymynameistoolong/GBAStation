# GBAStation Android Scoped Storage 审计

**审计范围：** Android `targetSdk` 36 的数据目录、AndroidManifest 权限、ROM 导入与原生文件访问路径。
**结论：** 修复后，GBAStation 的运行时数据和 ROM 导入流程遵循 Android 的 Scoped Storage 模型：应用仅写入自身的应用专属目录；对于下载、SD 卡和云盘中的用户文件，应用通过 Storage Access Framework（SAF）让用户显式选择后复制到应用目录。项目不声明“所有文件访问”、不依赖 legacy 外部存储，也不请求不必要的共享媒体权限。

## 合规性判断

| 审计项目 | 修复后实现 | 判断 |
| --- | --- | --- |
| 目标平台 | `targetSdk = 36`。 | 必须遵循 Scoped Storage；`requestLegacyExternalStorage` 不可作为兼容性方案。[2] |
| 持久运行时数据 | 配置、数据库、存档、截图、缓存、BIOS 与导入 ROM 位于 SDL 返回的应用专属外部文件目录；该卷不可写时回退到 SDL 的应用内部偏好目录。 | 符合。应用专属目录无需广泛存储权限。[1] |
| 写入失败路径 | 外部卷状态不含可写位时不再回退到 `/`，而是使用应用内部目录。 | 符合。避免在无权限的系统根目录创建数据。 |
| 清单权限 | 已移除 `requestLegacyExternalStorage`、`READ_EXTERNAL_STORAGE`、`WRITE_EXTERNAL_STORAGE`、`READ_MEDIA_IMAGES` 和 `READ_MEDIA_VIDEO`。 | 符合最小权限原则；这些权限不是应用专属目录或 SAF 导入所必需的。[1] [2] |
| ROM 导入 | 使用 `ACTION_OPEN_DOCUMENT` 多选系统选择器。用户选择的 `content://` 文件会被复制到应用的 `roms/` 目录；不会保留或扫描未授权的共享存储路径。 | 符合。SAF 不需要广泛存储权限，且由用户授予选择范围。[3] |
| 原生文件浏览 | Android 下本地文件选择器从应用的 `roms/` 目录开始。 | 符合。不会把 `/` 或 `Android/data` 当作可遍历的用户存储根。 |
| Switch 专用路径 | Android 不写入 `sdmc:` 或 `.nro` 外置核心默认值，并隐藏 NRO 路径设置。 | 符合平台隔离目标，且防止产生无效文件路径。 |

> Android 的应用专属文件在用户卸载应用时会被删除。该行为符合平台模型，但意味着用户应在卸载或清除应用数据前导出并备份存档、设置与游戏库数据库。[1]

## 存储模型

默认数据根目录由 SDL 的 `getExternalFilesDir(null)` 对应目录提供，典型形式为：

```text
/storage/emulated/0/Android/data/com.beiklive.gbastation/files/GBAStation/
```

该目录下的 `roms/`、`saves/`、`screenshots/`、`config/` 和 `data/` 都只供 GBAStation 使用。当外部应用专属卷不可写时，应用会退回到内部应用文件目录；这保证存档和配置不会被写入公共根目录，但会使文件仅能通过应用内系统选择器和数据备份工具访问。

ROM 并不直接留在用户通过 SAF 选择的下载目录、SD 卡或云盘 URI 上。应用读取一次由用户授予的 URI，将内容复制到自己的 `roms/` 目录，然后原生模拟器始终读取应用拥有的普通文件路径。这样既避免了原生库长期保存或遍历共享存储 URI 的复杂权限问题，也使后续扫描和运行不依赖临时 URI 授权。[3]

## 仍需注意的产品边界

本项目**没有**申请 `MANAGE_EXTERNAL_STORAGE`。这意味着它不能作为设备级文件管理器遍历所有共享文件，也不能自行访问其他应用的 `Android/data` 或 `Android/obb`。这不是缺失的权限，而是对模拟器前端更合适的隐私边界；用户可通过系统文件选择器选择需要导入的 ROM。[2] [3]

Android 11 及以后，即使使用 SAF，系统也会限制存储卷根目录、可靠 SD 卡根目录、下载目录树以及 `Android/data` 和 `Android/obb` 的选择范围。因此用户指南不要求用户将 ROM 放到 `Android/data`，而是要求通过“从设备导入 ROM”选择可访问的文件，再由应用复制到自己的目录。[2] [3]

## 验证清单

在 Android 11 或更高版本的真机或模拟器上，应验证以下行为：

| 验证步骤 | 预期结果 |
| --- | --- |
| 安装并首次启动 | 应用创建自己的数据目录，不弹出“所有文件”或媒体权限请求。 |
| 在数据管理页选择“从设备导入 ROM” | 打开 Android 系统文件选择器，可选择一个或多个文件。 |
| 选择 ROM | 文件复制到 `roms/`；应用提示导入结果。 |
| 扫描 `roms/` | 已支持平台的 ROM 出现在游戏库并可启动。 |
| 卸载前备份 | `saves/`、`config/` 和 `data/` 可由用户备份；卸载后应用专属数据会被系统清除。 |
| 外部卷不可用 | 应用仍能在内部应用目录保存配置和存档，不尝试写入 `/`。 |

## 参考资料

[1]: https://developer.android.com/training/data-storage/app-specific "Android Developers: Access app-specific files"
[2]: https://developer.android.com/about/versions/11/privacy/storage "Android Developers: Storage updates in Android 11"
[3]: https://developer.android.com/training/data-storage/shared/documents-files "Android Developers: Access documents and other files from shared storage"
