#include "render.hpp"
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>
#include <iostream>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

void WindowClass::Draw(std::string_view label)
{
    constexpr static auto windowFlags = ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoScrollbar;

    static auto windowSize =
        ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
    constexpr static auto windowPos = ImVec2(0.0F, 0.0F);

    ImGui::SetNextWindowContentSize(windowSize);
    ImGui::SetNextWindowPos(windowPos);

    ImGui::Begin(label.data(), nullptr, windowFlags);

    DrawMenu();
    ImGui::Separator();
    ImGui::BeginChild("Scrollable Text", ImVec2(0, 500), windowFlags);
    DrawContent();
    ImGui::Text(" ");
    ImGui::EndChild();
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - (0.3f)*ImGui::GetWindowHeight());
    DrawFilter();
    ImGui::Text(" ");
    DrawActions();
    // ImGui::SameLine(); // the next text in fn and prev are in same line
    ImGui::End();
}

void WindowClass::DrawMenu()
{
    if (ImGui::Button("Go Up"))
    { // Imgui::Button return true if button is click
        if (currentPath.has_parent_path())
        {
            currentPath = currentPath.parent_path();
        }
    }

    ImGui::SameLine();

    std::string currentPathString = currentPath.string();
    std::string OutputText = "Current Directory : " + currentPathString;

    ImGui::Text(OutputText.data());
}

void WindowClass::DrawActions()
{
    if (fs::is_directory(selectedEntry))
    {
        ImGui::Text("Selected Dir : %s", selectedEntry.string().c_str());
    }
    else if (fs::is_regular_file(selectedEntry))
    {
        ImGui::Text("Selected file : %s", selectedEntry.string().c_str());
    }
    else
    {
        ImGui::Text("Nothing is selected");
    }

    ImGui::Text(" ");

    if (fs::is_regular_file(selectedEntry) && ImGui::Button("Open"))
    {
        openFileWithDefaultEditor();
    }

    ImGui::SameLine();

    if (ImGui::Button("Rename"))
    {
        renameDialogOpen = true;
        ImGui::OpenPopup(
            "Rename File"); // for pop up fn , you need the same name as it is here
    }

    ImGui::SameLine();

    if (ImGui::Button("Delete"))
    {
        deleteDialogOpen = true;
        ImGui::OpenPopup("Delete File");
    }
    renameFilePopUp();
    deleteFilePopUp();
}

void WindowClass::DrawContent()
{
    for (const auto &entry : fs::directory_iterator(currentPath))
    {

        const bool isSelected = entry.path() == selectedEntry;
        const bool is_Directory = entry.is_directory();
        const bool is_file = entry.is_regular_file();
        std::string entryName = entry.path().filename().string();

        if (is_Directory)
        {
            entryName = "[D] : " + entryName;
        }
        else if (is_file)
        {
            entryName = "[F] : " + entryName;
        }

        if (ImGui::Selectable(entryName.data(), isSelected))
        {

            if (is_Directory)
            {
                currentPath /=
                    entry.path(); // /= overload for + and add file name
            }

            selectedEntry = entry.path();
        }
    }
}

void WindowClass::DrawFilter()
{
    static char extension_filter[16] = {"\0"};

    ImGui::Text("Filter by extension");
    ImGui::SameLine();
    ImGui::InputText("###input filter",
                     extension_filter,
                     sizeof(extension_filter));

    if (std::strlen(extension_filter) == 0)
        return;

    size_t filtered_file_count{0};

    for (const auto &entry : fs::directory_iterator(currentPath))
    {

        if (!fs::is_regular_file(entry))
            continue;

        if (entry.path().extension().string() == extension_filter)
            ++filtered_file_count;
    }
    std::string temp = std::to_string(filtered_file_count);
    ImGui::Text(temp.data());
}

void WindowClass::openFileWithDefaultEditor()
{
#ifdef _WIN32
    const auto command =
        "start \"\" \"" + selectedEntry.string() +"\"";
        // \ is a escape character it allows special character in string otherwise terminated
#elif __APPLE__
    const auto command = "open \"" + selectedEntry.string() + "\"";
#else
    const auto command = "xdg-open \"" + selectedEntry.string() + "\"";
#endif

    std::system(command.c_str());
    return;
}

void WindowClass::renameFilePopUp()
{
    if (ImGui::BeginPopupModal("Rename File", &renameDialogOpen))
    {
        static char buffer_name[512] = {'\0'};

        ImGui::Text("Enter file name");
        ImGui::InputText("###input file name",
                         buffer_name,
                         sizeof(buffer_name));

        auto new_path =
            selectedEntry
                .parent_path(); // / here overload class for concat path
        new_path /= buffer_name;

        //ImGui::Text(new_path.string().data());
        if (ImGui::Button("Rename"))
        {


            if (renameFile(selectedEntry, new_path))
            {
                selectedEntry = new_path;
                memset(buffer_name, '\0', sizeof(buffer_name));
                renameDialogOpen = false;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("cancel"))
        {
            renameDialogOpen = false;
        }
        ImGui::EndPopup();
    }
}

void WindowClass::deleteFilePopUp()
{
    if (ImGui::BeginPopupModal("Delete File", &deleteDialogOpen))
    {
        ImGui::Text("Are you sure ? ");

        if (ImGui::Button("Yes"))
        {
            if (deleteFile(selectedEntry))
            {
                selectedEntry.clear();
            }
            deleteDialogOpen = false;
        }

        if (ImGui::Button("cancel"))
        {
            deleteDialogOpen = false;
        }
        ImGui::EndPopup();
    }
}

bool WindowClass::renameFile(const fs::path &old_path, const fs::path &new_path)
{
    try
    {
        fs::rename(old_path, new_path);

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        std::cerr << old_path << " " << new_path << std::endl;
        return false;
    }
}

bool WindowClass::deleteFile(const fs::path &path)
{
    try
    {
        fs::remove(path);
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
}

void render(WindowClass &window_obj)
{
    window_obj.Draw("Label");
}
