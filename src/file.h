/* date = January 2nd 2026 11:36 pm */

#ifndef FILE_H
#define FILE_H

#include "log.h"

//  ========================================================================
// NOTE: File I/O
//  ========================================================================

long long GetTimestamp(char * file)
{
    struct stat fileStat = {};
    stat(file, &fileStat);
    return fileStat.st_mtime;
}

bool8 FileExists(char * filePath)
{
    SM_ASSERT(filePath, "No file path provided!");
    
    auto file = fopen(filePath, "rb");
    if (!file)
    {
        return false;
    }
    fclose(file);
    
    return true;    
}

long GetFileSize(char * filePath)
{
    SM_ASSERT(filePath, "No file path provided!");
    
    long fileSize = 0;
    
    auto file = fopen(filePath, "rb");
    if (!file)
    {
        SM_ERROR("Failed to open file: %s", filePath);
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    fclose(file);
    
    return fileSize;
}

// NOTE: Reads a file into a supplied buffer. We manage our own memory and therefore want more contorl over where it is allocated
char * read_file(char * filePath, int * fileSize, char * buffer)
{
    SM_ASSERT(filePath, "No file pth provided!");
    SM_ASSERT(fileSize, "No file size provided!");
    SM_ASSERT(buffer,   "No buffer provided!");
    
    *fileSize = 0;
    
    auto file = fopen(filePath, "rb");
    if (!file)
    {
        SM_ERROR("Failed to open file: %s", filePath);
        return nullptr;
    }
    
    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    memset(buffer, 0, *fileSize + 1);
    fread(buffer, sizeof(char), *fileSize, file);
    
    fclose(file);
    
    return buffer;
    
}

char * read_file(char *filePath, int * fileSize, BumpAllocator * ba)
{
    char * file = nullptr;
    long fileSize2 = GetFileSize(filePath);
    if (fileSize2)
    {
        char * buffer = BumpAlloc(ba, fileSize2 + 1);
        file = read_file(filePath, fileSize, buffer);
    }
    
    return file;
    
}

void write_file(char * filePath, char * buffer, int size)
{
    SM_ASSERT(filePath, "No file path provided!");
    SM_ASSERT(buffer,   "No buffer provided!");
    
    auto file = fopen(filePath, "wb");
    if (!file)
    {
        SM_ERROR("Failed to open file: %s", filePath);
        return;
    }
    
    fwrite(buffer, sizeof(char), size, file);
    fclose(file);
}

bool8 copy_file(char * fileName, char * outputName, char * buffer)
{
    int fileSize = 0;
    char * data = read_file(fileName, &fileSize, buffer);
    
    auto outputFile = fopen(outputName, "wb");
    if (!outputFile)
    {
        SM_ERROR("Failed to open file: %s", outputName);
        return false;
    }
    
    size_t result = fwrite(data, sizeof(char), fileSize, outputFile);
    if (!result)
    {
        SM_ERROR("Failed to open file: %s", outputName);
        return false;
    }
    
    fclose(outputFile);
    
    return true;
}


bool8 copy_file(char * fileName, char * outputName, BumpAllocator * ba)
{
    
    char * file = nullptr;
    long fileSize = GetFileSize(fileName);
    if (fileSize)
    {
        char * buffer = BumpAlloc(ba, fileSize + 1);
        return copy_file(fileName, outputName, buffer);
    }
    
    return false;
    
}


#endif //FILE_H
