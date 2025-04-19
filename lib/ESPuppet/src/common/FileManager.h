#pragma once
#include "util/Includes.h"

class FileManager 
{
    public:
        FileManager();
        void init();
        std::vector<String> getFileNameList(const char *extension);
        void printFilesInDirectory(const char *dirname, uint8_t levels);
        File openFile(String fileName);
};