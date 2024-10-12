#pragma once

#include <filesystem>
#include <imgui.h>
#include <string_view>

namespace fs = std::filesystem;

class WindowClass
{
public:
    WindowClass() : currentPath{fs::current_path()},selectedEntry(fs::path{})
    {
    }
    void Draw(std::string_view label);

private:
    void DrawMenu();
    void DrawContent();
    void DrawActions();
    void DrawFilter();

private:
    fs::path currentPath;
    fs::path selectedEntry;

private:
    void openFileWithDefaultEditor();
    void renameFilePopUp();
    void deleteFilePopUp();
    bool renameFile(const fs::path &old_path,const fs::path &new_path);
    bool deleteFile(const fs::path &path);

private:
    bool renameDialogOpen = false;
    bool deleteDialogOpen = false;
};

void render(WindowClass &window_obj);
