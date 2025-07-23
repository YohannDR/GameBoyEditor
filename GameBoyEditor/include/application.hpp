#pragma once

#include <string>

#include "core.hpp"

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;

class Application
{
    STATIC_CLASS(Application)

public:
    _NODISCARD static bool_t Init();
    static void Update();
    static void Shutdown();

    _NODISCARD static bool_t TryParseProject();
    static void BuildRom(bool_t launch);

    _NODISCARD static bool_t IsProjectLoaded() { return m_ProjectLoaded; }

public:
    static inline std::string projectPath;

private:
    static void PreLoop();
    static void PostLoop();

    _NODISCARD static bool_t ParseProject();

private:
    static inline GLFWwindow* m_Window = nullptr;
    static inline bool_t m_ProjectLoaded = false;
};
