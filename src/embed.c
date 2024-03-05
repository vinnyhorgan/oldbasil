#include "embed.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "lib/dirent.h"
#else
#include <sys/stat.h>
#include <dirent.h>
#endif

#include "util.h"

static void loadRecursive(const char *path, File **files, int *count)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        printf("Error opening directory: %s\n", path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                char subdir[MAX_PATH_SIZE];
                snprintf(subdir, MAX_PATH_SIZE, "%s/%s", path, entry->d_name);
                loadRecursive(subdir, files, count);
            }
        }
        else if (strcmp(entry->d_name + strlen(entry->d_name) - 5, ".wren") == 0)
        {
            *files = (File *)realloc(*files, (*count + 1) * sizeof(File));
            if (*files == NULL)
            {
                printf("Error allocating memory\n");
                closedir(dir);
                return;
            }

            char fullPath[MAX_PATH_SIZE];
            snprintf(fullPath, MAX_PATH_SIZE, "%s/%s", path, entry->d_name);
            int len = (int)strlen(fullPath);

            (*files)[*count].name = (char *)malloc(len + 1);
            if ((*files)[*count].name == NULL)
            {
                printf("Error allocating memory\n");
                closedir(dir);
                return;
            }

            strncpy((*files)[*count].name, fullPath, len);
            (*files)[*count].name[len] = '\0';

            char *file = readFile(fullPath);
            if (file == NULL)
            {
                closedir(dir);
                return;
            }

            (*files)[*count].source = file;

            (*count)++;
        }
    }

    closedir(dir);
}

void checkEmbedded(const char *selfPath, File **files, int *count)
{
    FILE *self = fopen(selfPath, "rb");
    if (self == NULL)
    {
        printf("Error opening self\n");
        return;
    }

    int size;
    char magic[6];

    fseek(self, -9, SEEK_END);

    fread(&size, sizeof(int), 1, self);
    fread(magic, sizeof(char), 5, self);
    magic[5] = '\0';

    if (strcmp(magic, "BASIL") == 0)
    {
        char *buffer = (char *)malloc(size);
        if (buffer == NULL)
        {
            printf("Error allocating memory\n");
            fclose(self);
            return;
        }

        fseek(self, -9 - size, SEEK_END);
        fread(buffer, sizeof(char), size, self);

        fclose(self);

        int offset = 0;

        memcpy(&(*count), buffer, sizeof(int));
        offset += sizeof(int);

        *files = (File *)malloc(*count * sizeof(File));
        if (*files == NULL)
        {
            printf("Error allocating memory\n");
            free(buffer);
            return;
        }

        for (int i = 0; i < *count; i++)
        {
            int nameLen, sourceLen;

            memcpy(&nameLen, buffer + offset, sizeof(int));
            offset += sizeof(int);

            (*files)[i].name = (char *)malloc(nameLen + 1);
            memcpy((*files)[i].name, buffer + offset, nameLen);
            (*files)[i].name[nameLen] = '\0';
            offset += nameLen;

            memcpy(&sourceLen, buffer + offset, sizeof(int));
            offset += sizeof(int);

            (*files)[i].source = (char *)malloc(sourceLen + 1);
            memcpy((*files)[i].source, buffer + offset, sourceLen);
            (*files)[i].source[sourceLen] = '\0';
            offset += sourceLen;
        }

        free(buffer);
    }
}

int build(const char *selfPath, const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
    {
        printf("Error opening: %s\n", path);
        return 1;
    }

    if (!S_ISDIR(st.st_mode))
    {
        printf("The path is not a directory\n");
        return 1;
    }

    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        printf("Error opening directory: %s\n", path);
        return 1;
    }

    bool found = false;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, "main.wren") == 0)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        printf("The directory does not contain a main.wren file\n");
        closedir(dir);
        return 1;
    }

    File *files = NULL;
    int count = 0;

    loadRecursive(path, &files, &count);
    if (count == 0)
    {
        printf("Error loading files\n");
        return 1;
    }

    int rootLen = (int)strlen(path);

    for (int i = 0; i < count; i++)
    {
        if (strncmp(files[i].name, path, rootLen) == 0 && files[i].name[rootLen] == '/')
        {
            memmove(files[i].name, files[i].name + rootLen + 1, strlen(files[i].name + rootLen + 1) + 1);
        }
    }

    int totalSize = 0;

    totalSize += sizeof(int);

    for (int i = 0; i < count; i++)
    {
        totalSize += sizeof(int) * 2;
        totalSize += (int)strlen(files[i].name);
        totalSize += (int)strlen(files[i].source);
    }

    char *buffer = (char *)malloc(totalSize);
    if (buffer == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    int offset = 0;

    memcpy(buffer, &count, sizeof(int));
    offset += sizeof(int);

    for (int i = 0; i < count; i++)
    {
        int nameLen = (int)strlen(files[i].name);
        int sourceLen = (int)strlen(files[i].source);

        memcpy(buffer + offset, &nameLen, sizeof(int));
        offset += sizeof(int);

        memcpy(buffer + offset, files[i].name, nameLen);
        offset += nameLen;

        memcpy(buffer + offset, &sourceLen, sizeof(int));
        offset += sizeof(int);

        memcpy(buffer + offset, files[i].source, sourceLen);
        offset += sourceLen;
    }

    FILE *self = fopen(selfPath, "rb");
    if (self == NULL)
    {
        printf("Error opening self\n");
        freeEmbedded(files, count);
        free(buffer);
        return 1;
    }

    char outPath[MAX_PATH_SIZE];

#ifdef _WIN32
    snprintf(outPath, MAX_PATH_SIZE, "%s.exe", path);
#else
    snprintf(outPath, MAX_PATH_SIZE, "%s", path);
#endif

    FILE *output = fopen(outPath, "wb");
    if (output == NULL)
    {
        printf("Error opening output\n");
        freeEmbedded(files, count);
        free(buffer);
        fclose(self);
        return 1;
    }

    char copyBuffer[1024];
    size_t bytesRead;

    while ((bytesRead = fread(copyBuffer, 1, 1024, self)) > 0)
        fwrite(copyBuffer, 1, bytesRead, output);

    fwrite(buffer, 1, totalSize, output);

    fwrite(&totalSize, 1, sizeof(int), output);
    fwrite("BASIL", 1, 5, output);

    freeEmbedded(files, count);
    free(buffer);

    fclose(self);
    fclose(output);

    return 0;
}

void freeEmbedded(File *files, int count)
{
    if (files == NULL)
        return;

    for (int i = 0; i < count; i++)
    {
        free(files[i].name);
        free(files[i].source);
    }

    free(files);
}
