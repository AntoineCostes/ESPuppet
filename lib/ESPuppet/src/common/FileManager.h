#pragma once
#include "util/Includes.h"

class FileManager 
{
    public:
        FileManager();
        static void init();

        static void printFilesInDirectory(String dirname, uint8_t levels);
        static File openFile(String fileName, bool write = false);
        static bool exists(String fileName);

        static std::vector<String> getConfigNames();
        static String getCurrentConfigName();
        static File openCurrentConfig();
        static void setNewConfig(String configName);
};