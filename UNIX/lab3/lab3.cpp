#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <openssl/sha.h>
#include <cstring>
#include <memory>

namespace fs = std::filesystem;


std::string calculateSHA1(const fs::path& filepath) 
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file) 
    {
        throw std::runtime_error("Не удалось открыть файл: " + filepath.string());
    }

    SHA_CTX shaContext;
    SHA1_Init(&shaContext);

    char buffer[1024 * 1024]; 
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) 
    {
        SHA1_Update(&shaContext, buffer, file.gcount());
    }

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash, &shaContext);

    std::string result;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) 
    {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", hash[i]);
        result += hex;
    }

    return result;
}

void replaceWithHardLinks(const std::map<std::string, std::vector<fs::path>>& hashGroups) 
{
    for (const auto& [hash, files] : hashGroups) 
    {
        if (files.size() < 2) 
        {
            continue; 
        }

        const fs::path& originalFile = files[0];

        for (size_t i = 1; i < files.size(); i++) 
        {
            const fs::path& duplicateFile = files[i];
            
            try 
            {
                fs::remove(duplicateFile);
                fs::create_hard_link(originalFile, duplicateFile);
                
                std::cout << "Создана ссылка: " << duplicateFile << " -> " << originalFile << std::endl;
            } 
            catch (const fs::filesystem_error& e) 
            {
                std::cerr << "Ошибка при создании ссылки для " << duplicateFile << ": " << e.what() << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[]) 
{

    fs::path directory = argv[1];
    
    if (!fs::exists(directory) || !fs::is_directory(directory)) 
    {
        std::cerr << "Указанный путь не существует" << std::endl;
        return 1;
    }

    std::map<std::string, std::vector<fs::path>> hashGroups;

    try 
    {        
        for (const auto& entry : fs::recursive_directory_iterator(directory))
         {
            if (fs::is_regular_file(entry.path())) 
            {
                try 
                {
                    std::cout << "Обработка файла: " << entry.path() << std::endl;
                    
                    std::string hash = calculateSHA1(entry.path());
                    
                    hashGroups[hash].push_back(entry.path());
                    
                    std::cout << " Хэш: " << hash << std::endl;
                } 
                catch (const std::exception& e)
                {
                    std::cerr << "Ошибка обработки файла " << entry.path() << ": " << e.what() << std::endl;
                }
            }
        }

        std::cout << "\nНайдено " << hashGroups.size() << " уникальных хэшей" << std::endl;
        
        replaceWithHardLinks(hashGroups);
    } 
    catch (const std::exception& e)
    {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}