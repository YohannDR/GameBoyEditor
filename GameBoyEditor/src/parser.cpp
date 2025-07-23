#include "parser.hpp"

#include <fstream>
#include <iostream>

#include "application.hpp"

bool_t Parser::ParseProject()
{
    const std::filesystem::path path = Application::projectPath;
    const std::filesystem::path dataPath = path / "src/data";

    for (const std::filesystem::directory_entry& dirEntry : std::filesystem::recursive_directory_iterator(dataPath))
    {
        // Skip folders
        if (dirEntry.is_directory())
            continue;

        const std::filesystem::path& file = dirEntry.path();
        // Skip non .c file
        if (file.extension() != ".c")
            continue;

        if (!ParseFileContents(file))
            return false;
    }

    return true;
}

bool_t Parser::ParseFileContents(const std::filesystem::path& filePath)
{
    std::ifstream file;
    uint8_t type;
    int32_t width = 0;
    int32_t height = 0;

    file.open(filePath);

    if (file.bad())
        return false;

    std::string line;
    while (file)
    {
        std::getline(file, line);

        if (!line.contains("const u8"))
            continue;

        const size_t idx = line.find('[');
        const std::string symbolName = line.substr(sizeof("const u8"), idx - sizeof("const u8"));

        if (symbolName.contains("Graphics"))
        {
            std::getline(file, line);
            std::getline(file, line);
            type = 0;
        }
        else if (symbolName.contains("Tilemap"))
        {
            std::getline(file, line);
            (void)sscanf_s(line.c_str(), "%d, %d", &width, &height);
            
            std::getline(file, line);
            type = 1;
        }
        else if (symbolName.contains("Clipdata"))
        {
            std::getline(file, line);
            (void)sscanf_s(line.c_str(), "%d, %d", &width, &height);
            std::getline(file, line);
            type = 2;
        }
        else
        {
            continue;
        }

        std::vector<uint8_t> data;

        while (true)
        {
            std::getline(file, line);
            if (line[0] == '}')
                break;

            if (type == 0)
            {
                int32_t buffer[16];
                (void)sscanf_s(line.c_str(), "%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",
                    &buffer[0],  &buffer[1],  &buffer[2],  &buffer[3],
                    &buffer[4],  &buffer[5],  &buffer[6],  &buffer[7],
                    &buffer[8],  &buffer[9],  &buffer[10], &buffer[11],
                    &buffer[12], &buffer[13], &buffer[14], &buffer[15]
                );
                data.append_range(buffer);
            }
            else if (type == 1 || type == 2)
            {
                int32_t buffer[20];
                (void)sscanf_s(line.c_str(), "%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",
                    &buffer[0],  &buffer[1],  &buffer[2],  &buffer[3],  &buffer[4],
                    &buffer[5],  &buffer[6],  &buffer[7],  &buffer[8],  &buffer[9],
                    &buffer[10], &buffer[11], &buffer[12], &buffer[13], &buffer[14],
                    &buffer[15], &buffer[16], &buffer[17], &buffer[18], &buffer[19]
                );
                data.append_range(buffer);
            }
        }

        if (type == 0)
        {
            graphics[symbolName] = data;
        }
        else if (type == 1)
        {
            tilemaps[symbolName] = Tilemap{ width, height, data };
        }
        else if (type == 1)
        {
            // m_Clipdata[symbolName] = { width, height, data };
        }
    }

    file.close();
    return true;
}
