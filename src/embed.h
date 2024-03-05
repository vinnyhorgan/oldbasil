#ifndef EMBED_H
#define EMBED_H

typedef struct File
{
    char *name;
    char *source;
} File;

int build(const char *selfPath, const char *path);
void checkEmbedded(const char *selfPath, File **files, int *count);
void freeEmbedded(File *files, int count);

#endif
