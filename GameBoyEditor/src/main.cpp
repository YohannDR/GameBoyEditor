#include <cstdlib>

#include "application.hpp"

int main(int32_t, char_t*[])
{
    if (!Application::Init())
        return EXIT_FAILURE;

    Application::Update();
    Application::Shutdown();

    return EXIT_SUCCESS;
}
