#include "FileManager.h"

FileManager::FileManager()
{
}

void FileManager::init()
{
    if (!LittleFS.begin())
    {
        Serial.println("[FM] [Error] could not mount LittleFS");
        return;
    } 
    Serial.println("[FM] LittleFS initalized. Listing files:");
    printFilesInDirectory("/", 1);
}


File FileManager::openFile(String filePath)
{
    if (!filePath.startsWith("/"))
    filePath = "/" + filePath;
    Serial.println("[FM] open " +filePath);
    
    if (!LittleFS.exists(filePath))  Serial.println("[FM]" +filePath + "does not exist !");

    if (!LittleFS.exists(filePath)) return File();
    return LittleFS.open(filePath.c_str(), "r");
}

std::vector<String> FileManager::getFileNameList(const char *extension)
{
    std::vector<String> fileNameList;
    
    File root = LittleFS.open("/", "r");

    if (!root) Serial.println("[FM] Failed to open directory");
    else if (!root.isDirectory()) Serial.println("[FM] Not a directory");
    else 
    {
        File file = root.openNextFile();
        while (file)
        {
            String fileName = String(file.name());
            Serial.println("\t" + fileName + " (" + String(file.size()) + " bytes)");
            
            if (fileName.endsWith(extension)) fileNameList.emplace_back(fileName.substring(0, fileName.length() - 4));
            file = root.openNextFile();
        }
    }
    return fileNameList;
}

void FileManager::printFilesInDirectory(const char *dirname, uint8_t levels)
{
    std::vector<String> fileNameList;
    
    File root = LittleFS.open(dirname, "r");

    if (!root) Serial.println("[FM] Failed to open directory");
    else if (!root.isDirectory()) Serial.println("[FM] Not a directory");
    else 
    {
        File file = root.openNextFile();
        while (file)
        {
            if (file.isDirectory())
            {
                Serial.println("[DIR] " + String(file.name()));
                if (levels)
                {
                    printFilesInDirectory(file.path(), levels - 1);
                }
            }
            else
            {
                String fileName = String(file.name());
                Serial.println("\t" + fileName + " (" + String(file.size()) + " bytes)");
            }
            file = root.openNextFile();
        }
    }
}

