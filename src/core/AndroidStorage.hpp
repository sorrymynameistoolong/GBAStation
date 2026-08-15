#pragma once

namespace beiklive::android_storage
{

// Opens Android's system document picker for one or more user-selected ROM
// files. Selected files are copied into the application's own ROM directory,
// so native emulation code only ever receives app-owned filesystem paths.
bool requestRomImport();

} // namespace beiklive::android_storage
