#include "core/SteamGridDb.hpp"

namespace beiklive::steamgriddb
{
namespace
{
    constexpr const char* kUnavailable =
        "SteamGridDB online artwork is not available in the Android package.";

    template <typename T>
    Result<T> unavailable()
    {
        Result<T> result;
        result.networkError = true;
        result.error = kUnavailable;
        return result;
    }
}

std::string rootDirectory() { return {}; }
std::string apiKeyPath() { return {}; }
std::string cacheDirectory() { return {}; }
bool hasApiKey() { return false; }
std::string loadApiKey() { return {}; }

bool saveApiKey(const std::string&, std::string* error)
{
    if (error) *error = kUnavailable;
    return false;
}

Result<bool> validateApiKey(const std::string&)
{
    return unavailable<bool>();
}

bool clearCache(std::string* error)
{
    if (error) *error = kUnavailable;
    return false;
}

Result<std::vector<SearchGame>> searchGames(
    const std::vector<std::string>&, const std::atomic<bool>*)
{
    return unavailable<std::vector<SearchGame>>();
}

Result<AssetGroups> fetchAllAssets(
    const std::vector<SearchGame>&, int, const std::atomic<bool>*)
{
    return unavailable<AssetGroups>();
}

std::vector<Asset> applyFilters(const std::vector<Asset>& source, const Filters&)
{
    return source;
}

bool ensureAssetCached(Asset&, bool, std::string* error, const std::atomic<bool>*)
{
    if (error) *error = kUnavailable;
    return false;
}

bool saveAssetAsCover(const Asset&, const GameEntry&, std::string& outputPath,
                      std::string* error)
{
    outputPath.clear();
    if (error) *error = kUnavailable;
    return false;
}

const char* assetTypeName(AssetType type)
{
    switch (type) {
        case AssetType::Grids: return "grid";
        case AssetType::Heroes: return "hero";
        case AssetType::Logos: return "logo";
        case AssetType::Icons: return "icon";
        default: return "asset";
    }
}

} // namespace beiklive::steamgriddb
