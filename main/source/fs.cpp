#include <fs.h>

#include <windows.h>
#include <shlobj.h>

#include <algorithm>
#include <filesystem>

namespace whale::fs
{

    std::optional<std::fs::path> getExecutablePath()
    {
        std::wstring exePath(MAX_PATH, '\0');
        if (GetModuleFileNameW(nullptr, exePath.data(), exePath.length()) == 0)
            return std::nullopt;

        return exePath;

    }

    
    bool openFileBrowser(DialogMode mode, const std::vector<nfdfilteritem_t> &validExtensions, const std::function<void(std::fs::path)> &callback, const std::string &defaultPath)
    {

        NFD::Init();

        nfdchar_t *outPath = nullptr;
        nfdresult_t result;
        switch (mode) {
            case DialogMode::Open:
                result = NFD::OpenDialog(outPath, validExtensions.data(), validExtensions.size(), defaultPath.c_str());
                break;
            case DialogMode::Save:
                result = NFD::SaveDialog(outPath, validExtensions.data(), validExtensions.size(), defaultPath.c_str());
                break;
            case DialogMode::Folder:
                result = NFD::PickFolder(outPath, defaultPath.c_str());
                break;
            default:
                break;
        }

        if (result == NFD_OKAY && outPath != nullptr) {
            callback(reinterpret_cast<char8_t*>(outPath));
            NFD::FreePath(outPath);
        }

        NFD::Quit();

        return result == NFD_OKAY;
    }

    bool openFileBrowser(DialogMode mode, const std::string &defaultPath)
    {

        NFD::Init();

        nfdchar_t *outPath = nullptr;
        nfdresult_t result;
        switch (mode) {
            case DialogMode::Open:
                result = NFD::OpenDialog(outPath, nullptr, 0, defaultPath.c_str());
                break;
            case DialogMode::Save:
                result = NFD::SaveDialog(outPath, nullptr, 0, defaultPath.c_str());
                break;
            case DialogMode::Folder:
                result = NFD::PickFolder(outPath, defaultPath.c_str());
                break;
            default:
                break;
        }

        NFD::Quit();

        return result == NFD_OKAY;
    }

    static std::vector<std::fs::path> getDataPaths()
    {
        std::vector<std::fs::path> paths;

        for (auto &path : paths)
            path = path / "whale";

        if (auto executablePath = fs::getExecutablePath(); executablePath.has_value())
            paths.push_back(executablePath->parent_path());

        return paths;
    }

    static std::vector<std::fs::path> getConfigPaths()
    {
        return getDataPaths();
    }

    constexpr std::vector<std::fs::path> appendPath(std::vector<std::fs::path> paths, const std::fs::path &folder)
    {
        for (auto &path : paths)
            path = path / folder;

        return paths;
    };

    std::vector<std::fs::path> getProjectPaths()
    {
        std::vector<std::fs::path> paths = getDataPaths();
        return paths;
    }

    std::vector<std::fs::path> getDefaultPaths(WhalePath path, bool listNonExisting)
    {
        std::vector<std::fs::path> result;

        switch (path)
        {
        case WhalePath::Constants:
            result = appendPath(getDataPaths(), "constants");
            break;
        case WhalePath::Config:
            result = appendPath(getConfigPaths(), "config");
            break;
        case WhalePath::Logs:
            result = appendPath(getConfigPaths(), "logs");
            break;
        }

        if (!listNonExisting)
        {
            result.erase(std::remove_if(result.begin(), result.end(), [](const auto &path)
                                        { return !fs::isDirectory(path); }),
                         result.end());
        }

        return result;
    }

    std::fs::path toShortPath(const std::fs::path &path)
    {
        size_t size = GetShortPathNameW(path.c_str(), nullptr, 0) * sizeof(TCHAR);
        if (size == 0)
            return path;

        std::wstring newPath(size, 0x00);
        GetShortPathNameW(path.c_str(), newPath.data(), newPath.size());

        return newPath;
    }

}
