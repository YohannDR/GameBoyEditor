#include "application.hpp"

#include <filesystem>
#include <iostream>

#include "parser.hpp"
#include "ui.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// ReSharper disable CppClangTidyConcurrencyMtUnsafe

bool_t Application::Init()
{
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(800, 600, "Game Boy Editor", nullptr, nullptr);

    glfwMakeContextCurrent(m_Window);
    glfwShowWindow(m_Window);

    gladLoadGL();

    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.Fonts->AddFontDefault();

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    Ui::Init();

    return true;
}

void Application::Update()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        PreLoop();

        Ui::MainMenuBar();
        Ui::DrawWindows();

        PostLoop();
    }
}

void Application::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
        
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

bool_t Application::TryParseProject()
{
    const std::filesystem::path path = projectPath;

    // Check is absolute path
    if (!path.is_absolute())
        return false;

    // Check is folder path
    if (!std::filesystem::is_directory(path))
        return false;

    // Verify if the makefile exists, that should be enough to verify it's the correct folder
    if (!std::filesystem::exists(path / "MakeFile"))
        return false;

    if (!Parser::ParseProject())
        return false;

    m_ProjectLoaded = true;
    return true;
}

void Application::BuildRom(const bool_t launch)
{
    // Windows is kind of dumb, in order to cd to a folder in another disk, we need to input the disk alone first
    const std::string disk = std::string(projectPath.c_str(), 2);

    // Goto correct disk, cd to project path, then run make -j in WSL
    const std::string command = std::format("{} && cd {} && wsl make -j", disk, projectPath);
    const int32_t result = system(command.c_str());

    if (result == 0 && launch)
    {
        // If the build succeeded, we may launch the ROM
        // TODO don't hardcode rom name
        const std::string romPath = projectPath + "/obj/Proto.gb";
        system(romPath.c_str());
    }
}

void Application::PreLoop()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Application::PostLoop()
{
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    GLFWwindow* ctxBackup = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(ctxBackup);

    glfwPollEvents();
    glfwSwapBuffers(m_Window);
}
