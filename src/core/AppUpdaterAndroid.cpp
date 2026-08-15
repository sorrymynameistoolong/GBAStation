#include "core/AppUpdater.hpp"

namespace beiklive {

AppUpdater& AppUpdater::instance() {
    static AppUpdater updater;
    return updater;
}

void AppUpdater::check() {
    (void)checkSync();
}

bool AppUpdater::checkSync() {
    m_aborted.store(false);
    m_downloadedData.clear();
    m_info = {};
    return false;
}

bool AppUpdater::download(std::function<bool(size_t, size_t)> onProgress) {
    (void)onProgress;
    m_aborted.store(false);
    m_downloadedData.clear();
    return false;
}

bool AppUpdater::install() {
    return false;
}

bool AppUpdater::finishInstall() {
    return false;
}

} // namespace beiklive
