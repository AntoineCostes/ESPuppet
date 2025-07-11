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
    FileManager::printFilesInDirectory("/", 1);
}

bool FileManager::exists(String filePath)
{
    return LittleFS.exists(filePath);
}

File FileManager::openFile(String filePath, bool write)
{
    if (!filePath.startsWith("/")) filePath = "/" + filePath;

    Serial.println("[FM] open " +filePath);
    
    if (!write && !LittleFS.exists(filePath))  
    {
        Serial.println("[FM]" +filePath + "does not exist !");
        return File();
    }
    return LittleFS.open(filePath.c_str(), write?"w":"r");
}

void FileManager::printFilesInDirectory(String dirname, uint8_t levels)
{
    if (!dirname.startsWith("/"))
    dirname = "/" + dirname;

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
                Serial.println("\t [DIR] " + String(file.name()));
                if (levels)
                {
                    FileManager::printFilesInDirectory(file.path(), levels - 1);
                }
            }
            else
            {
                String fileName = String(file.name());
                if (strcmp(dirname.c_str(), "/")) Serial.print("\t"); // indent if file is in dir
                Serial.print("\t");
                Serial.println(fileName + " (" + String(file.size()) + " bytes)");
            }
            file = root.openNextFile();
        }
    }
}



String FileManager::getCurrentConfigName()
{
    Preferences prefs;
    prefs.begin("ESPuppet");
    String configFileName = prefs.getString("config", "default"); // TODO test default config
    prefs.end();
    return configFileName;
}

void FileManager::setNewConfig(String configName)
{
    Preferences prefs;
    prefs.begin("ESPuppet");
    prefs.putString("config", configName);
    prefs.end();
}

File FileManager::openCurrentConfig()
{
    return FileManager::openFile("/"+String(ARDUINO_BOARD)+"/"+FileManager::getCurrentConfigName()+".json");
}

std::vector<String> FileManager::getConfigNames()
{
    std::vector<String> fileNameList;
    
    File root = LittleFS.open("/"+String(ARDUINO_BOARD), "r");

    if (!root) Serial.println("[FM] Failed to open directory");
    else if (!root.isDirectory()) Serial.println("[FM] Not a directory");
    else 
    {
        File file = root.openNextFile();
        while (file)
        {
            String fileName = String(file.name());
            
            if (fileName.endsWith(".json")) fileNameList.emplace_back(fileName.substring(0, fileName.length() - 5));
            file = root.openNextFile();
        }
    }
    return fileNameList;
}
