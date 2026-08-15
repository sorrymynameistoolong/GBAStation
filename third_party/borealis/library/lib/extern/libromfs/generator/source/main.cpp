#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

std::string replace(std::string string, const std::string& from, const std::string& to)
{
    if (!from.empty()) {
        for (size_t pos = 0; (pos = string.find(from, pos)) != std::string::npos; pos += to.size()) {
            string.replace(pos, from.size(), to);
        }
    }
    return string;
}

std::string toPathString(std::string string)
{
#if defined(_WIN32)
    string = replace(string, "\\", "/");
#endif
    return replace(string, "\"", "\\\"");
}

std::vector<std::byte> readFile(const fs::path& path)
{
    const auto size = fs::file_size(path);
    std::vector<std::byte> bytes(size);
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return {};
    }
    file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    bytes.resize(static_cast<size_t>(file.gcount()));
    return bytes;
}

std::string encodeBase64(const std::vector<std::byte>& bytes)
{
    static constexpr char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded;
    encoded.reserve(((bytes.size() + 2) / 3) * 4);
    for (size_t offset = 0; offset < bytes.size(); offset += 3) {
        const auto a = std::to_integer<unsigned char>(bytes[offset]);
        const auto b = offset + 1 < bytes.size() ? std::to_integer<unsigned char>(bytes[offset + 1]) : 0;
        const auto c = offset + 2 < bytes.size() ? std::to_integer<unsigned char>(bytes[offset + 2]) : 0;
        encoded.push_back(alphabet[(a >> 2) & 0x3f]);
        encoded.push_back(alphabet[((a & 0x03) << 4) | (b >> 4)]);
        encoded.push_back(offset + 1 < bytes.size() ? alphabet[((b & 0x0f) << 2) | (c >> 6)] : '=');
        encoded.push_back(offset + 2 < bytes.size() ? alphabet[c & 0x3f] : '=');
    }
    return encoded;
}

void writeStringLiteral(std::ofstream& output, const std::string& value)
{
    constexpr size_t lineWidth = 120;
    if (value.empty()) {
        output << "\"\"";
        return;
    }
    for (size_t offset = 0; offset < value.size(); offset += lineWidth) {
        output << "    \"" << value.substr(offset, lineWidth) << "\"\n";
    }
}

void writeResourceSource(const fs::path& outputDirectory, std::uint64_t identifier,
                         const std::string& base64)
{
    std::ofstream output(outputDirectory / ("libromfs_resource_" + std::to_string(identifier) + ".cpp"));
    output << "#include <cstddef>\n#include <cstdint>\n#include <vector>\n\n";
    output << "namespace {\n";
    output << "constexpr char kData[] =\n";
    writeStringLiteral(output, base64);
    output << ";\n\n";
    output << R"(int decodeBase64(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return 0;
}

std::vector<std::byte> decode()
{
    std::vector<std::byte> bytes;
    constexpr size_t length = sizeof(kData) - 1;
    bytes.reserve((length / 4) * 3);
    for (size_t i = 0; i < length; i += 4) {
        const int a = decodeBase64(kData[i]);
        const int b = decodeBase64(kData[i + 1]);
        const int c = kData[i + 2] == '=' ? 0 : decodeBase64(kData[i + 2]);
        const int d = kData[i + 3] == '=' ? 0 : decodeBase64(kData[i + 3]);
        bytes.push_back(static_cast<std::byte>((a << 2) | (b >> 4)));
        if (kData[i + 2] != '=') bytes.push_back(static_cast<std::byte>(((b & 0x0f) << 4) | (c >> 2)));
        if (kData[i + 3] != '=') bytes.push_back(static_cast<std::byte>(((c & 0x03) << 6) | d));
    }
    return bytes;
}
} // namespace

)";
    output << "std::vector<std::byte>& romfs_resource_" << identifier << "()\n{\n";
    output << "    static std::vector<std::byte> data = decode();\n";
    output << "    return data;\n}\n";
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::fprintf(stderr, "Usage: libromfs-generator <project-name> <resource-directory>\n");
        return 64;
    }

    const fs::path resourceRoot = fs::absolute(argv[2]);
    const fs::path outputDirectory = fs::current_path();
    std::ofstream output(outputDirectory / "libromfs_resources.cpp");
    if (!output) {
        std::fprintf(stderr, "Unable to create libromfs resource source.\n");
        return 1;
    }

    struct ResourceEntry {
        fs::path relativePath;
        std::uint64_t identifier;
    };
    std::vector<ResourceEntry> resources;
    std::uint64_t identifier = 0;

    std::printf("[libromfs] Resource Folder: %s\n", resourceRoot.string().c_str());
    for (const auto& entry : fs::recursive_directory_iterator(resourceRoot)) {
        if (!entry.is_regular_file() || entry.path().filename() == ".DS_Store") {
            continue;
        }
        const auto relativePath = fs::relative(entry.path(), resourceRoot);
        const auto bytes = readFile(entry.path());
        const auto encoded = encodeBase64(bytes);
        writeResourceSource(outputDirectory, identifier, encoded);
        resources.push_back({relativePath, identifier});
        ++identifier;
    }

    output << "#include <romfs/romfs.hpp>\n#include <cstddef>\n#include <filesystem>\n#include <map>\n#include <vector>\n\n";
    output << "namespace fs = std::filesystem;\n\n";
    for (const auto& resource : resources) {
        output << "std::vector<std::byte>& romfs_resource_" << resource.identifier << "();\n";
    }
    output << "\nconst std::map<fs::path, romfs::Resource>& RomFs_" << argv[1] << "_get_resources() {\n";
    output << "    static std::map<fs::path, romfs::Resource> resources = {\n";
    for (const auto& resource : resources) {
        output << "        { \"" << toPathString(resource.relativePath.string()) << "\", [] { auto& data = romfs_resource_"
               << resource.identifier << "(); return romfs::Resource({ data.data(), data.size() }); }() },\n";
    }
    output << "    };\n\n    return resources;\n}\n\n";
    output << "const std::vector<fs::path>& RomFs_" << argv[1] << "_get_paths() {\n";
    output << "    static std::vector<fs::path> paths = {\n";
    for (const auto& resource : resources) {
        output << "        \"" << toPathString(resource.relativePath.string()) << "\",\n";
    }
    output << "    };\n\n    return paths;\n}\n\n";
    output << "const std::string& RomFs_" << argv[1] << "_get_name() {\n";
    output << "    static std::string name = \"" << argv[1] << "\";\n";
    output << "    return name;\n}\n";

    return 0;
}
