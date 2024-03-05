#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/wren.h"

#ifdef _WIN32
#include "lib/dirent.h"
#else
#include <sys/stat.h>
#include <dirent.h>
#endif

#include "api.h"
#include "embed.h"
#include "util.h"

char basePath[MAX_PATH_SIZE];

static File *embedded = NULL;
static int count = 0;

static void wrenWrite(WrenVM *vm, const char *text)
{
    printf("%s", text);
}

static void wrenError(WrenVM *vm, WrenErrorType type, const char *module, int line, const char *message)
{
    switch (type)
    {
    case WREN_ERROR_COMPILE:
        printf("[%s line %d] %s\n", module, line, message);
        break;
    case WREN_ERROR_RUNTIME:
        printf("%s\n", message);
        break;
    case WREN_ERROR_STACK_TRACE:
        printf("[%s line %d] in %s\n", module, line, message);
        break;
    }
}

static void onComplete(WrenVM *vm, const char *name, WrenLoadModuleResult result)
{
    if (result.source)
        free((void *)result.source);
}

static WrenLoadModuleResult wrenLoadModule(WrenVM *vm, const char *name)
{
    WrenLoadModuleResult result = {0};

    if (strcmp(name, "meta") == 0 || strcmp(name, "random") == 0)
        return result;

    if (strcmp(name, "basil") == 0)
    {
        result.source = wrenApi;
        return result;
    }

    char fullPath[MAX_PATH_SIZE];
    snprintf(fullPath, MAX_PATH_SIZE, "%s/%s.wren", basePath, name);

    if (count > 0)
    {
        for (int i = 0; i < count; i++)
        {
            if (strcmp(fullPath, embedded[i].name) == 0)
            {
                result.source = embedded[i].source;
                break;
            }
        }
    }
    else
    {
        result.source = readFile(fullPath);
        result.onComplete = onComplete;
    }

    return result;
}

static WrenForeignClassMethods wrenBindForeignClass(WrenVM *vm, const char *module, const char *className)
{
    WrenForeignClassMethods methods = {0};

    if (strcmp(className, "Bitmap") == 0)
    {
        methods.allocate = bitmapAllocate;
        methods.finalize = bitmapFinalize;
    }
    else if (strcmp(className, "Font") == 0)
    {
        methods.allocate = fontAllocate;
        methods.finalize = fontFinalize;
    }
    else if (strcmp(className, "Pixel") == 0)
    {
        methods.allocate = pixelAllocate;
    }
    else if (strcmp(className, "Timer") == 0)
    {
        methods.allocate = timerAllocate;
        methods.finalize = timerFinalize;
    }
    else if (strcmp(className, "Window") == 0)
    {
        methods.allocate = windowAllocate;
    }

    return methods;
}

static WrenForeignMethodFn wrenBindForeignMethod(WrenVM *vm, const char *module, const char *className, bool isStatic, const char *signature)
{
    if (!isStatic)
    {
        if (strcmp(className, "Bitmap") == 0)
        {
            if (strcmp(signature, "init create(_,_)") == 0)
                return bitmapCreate;
            if (strcmp(signature, "init create(_)") == 0)
                return bitmapCreate2;
            if (strcmp(signature, "destroy()") == 0)
                return bitmapDestroy;
            if (strcmp(signature, "save(_)") == 0)
                return bitmapSave;
            if (strcmp(signature, "width") == 0)
                return bitmapWidth;
            if (strcmp(signature, "height") == 0)
                return bitmapHeight;
            if (strcmp(signature, "get(_,_,_)") == 0)
                return bitmapGet;
            if (strcmp(signature, "set(_,_,_)") == 0)
                return bitmapSet;
            if (strcmp(signature, "clear()") == 0)
                return bitmapClear;
            if (strcmp(signature, "clear(_)") == 0)
                return bitmapClear2;
            if (strcmp(signature, "rectangle(_,_,_,_,_)") == 0)
                return bitmapRectangle;
            if (strcmp(signature, "blit(_,_,_)") == 0)
                return bitmapBlit;
            if (strcmp(signature, "blit(_,_,_,_)") == 0)
                return bitmapBlit2;
            if (strcmp(signature, "blitRec(_,_,_,_,_,_,_)") == 0)
                return bitmapBlitRec;
            if (strcmp(signature, "blitRec(_,_,_,_,_,_,_,_)") == 0)
                return bitmapBlitRec2;
            if (strcmp(signature, "text(_,_,_,_)") == 0)
                return bitmapText;
        }
        else if (strcmp(className, "Font") == 0)
        {
            if (strcmp(signature, "init create(_,_,_)") == 0)
                return fontCreate;
            if (strcmp(signature, "destroy()") == 0)
                return fontDestroy;
        }
        else if (strcmp(className, "Pixel") == 0)
        {
            if (strcmp(signature, "init new(_,_,_,_)") == 0)
                return pixelNew;
            if (strcmp(signature, "init new(_,_,_)") == 0)
                return pixelNew2;
            if (strcmp(signature, "r") == 0)
                return pixelR;
            if (strcmp(signature, "g") == 0)
                return pixelG;
            if (strcmp(signature, "b") == 0)
                return pixelB;
            if (strcmp(signature, "a") == 0)
                return pixelA;
            if (strcmp(signature, "toString") == 0)
                return pixelToString;
        }
        else if (strcmp(className, "Timer") == 0)
        {
            if (strcmp(signature, "init create()") == 0)
                return timerCreate;
            if (strcmp(signature, "destroy()") == 0)
                return timerDestroy;
            if (strcmp(signature, "reset()") == 0)
                return timerReset;
            if (strcmp(signature, "now") == 0)
                return timerNow;
            if (strcmp(signature, "delta") == 0)
                return timerDelta;
        }
        else if (strcmp(className, "Window") == 0)
        {
            if (strcmp(signature, "init create(_,_,_,_)") == 0)
                return windowCreate;
            if (strcmp(signature, "init create(_,_,_)") == 0)
                return windowCreate2;
            if (strcmp(signature, "update(_)") == 0)
                return windowUpdate;
            if (strcmp(signature, "close()") == 0)
                return windowClose;
            if (strcmp(signature, "keyDown(_)") == 0)
                return windowKeyDown;
            if (strcmp(signature, "keyPressed(_)") == 0)
                return windowKeyPressed;
            if (strcmp(signature, "buttonDown(_)") == 0)
                return windowButtonDown;
            if (strcmp(signature, "buttonPressed(_)") == 0)
                return windowButtonPressed;
            if (strcmp(signature, "closed") == 0)
                return windowClosed;
            if (strcmp(signature, "scaleX") == 0)
                return windowScaleX;
            if (strcmp(signature, "scaleY") == 0)
                return windowScaleY;
            if (strcmp(signature, "active") == 0)
                return windowActive;
            if (strcmp(signature, "width") == 0)
                return windowWidth;
            if (strcmp(signature, "height") == 0)
                return windowHeight;
            if (strcmp(signature, "mouseX") == 0)
                return windowMouseX;
            if (strcmp(signature, "mouseY") == 0)
                return windowMouseY;
            if (strcmp(signature, "scrollX") == 0)
                return windowScrollX;
            if (strcmp(signature, "scrollY") == 0)
                return windowScrollY;
            if (strcmp(signature, "targetFps") == 0)
                return windowTargetFps;
            if (strcmp(signature, "targetFps=(_)") == 0)
                return windowTargetFpsSet;
        }
    }
    else
    {
        if (strcmp(className, "OS") == 0)
        {
            if (strcmp(signature, "name") == 0)
                return osName;
            if (strcmp(signature, "basilVersion") == 0)
                return osBasilVersion;
            if (strcmp(signature, "args") == 0)
                return osArgs;
            if (strcmp(signature, "readLine()") == 0)
                return osReadLine;
        }
    }

    return NULL;
}

int main(int argc, char **argv)
{
    setArgs(argc, argv);

    checkEmbedded(argv[0], &embedded, &count);

    if (count > 0)
    {
        snprintf(basePath, MAX_PATH_SIZE, ".");

        WrenConfiguration config;
        wrenInitConfiguration(&config);

        config.writeFn = wrenWrite;
        config.errorFn = wrenError;
        config.loadModuleFn = wrenLoadModule;
        config.bindForeignClassFn = wrenBindForeignClass;
        config.bindForeignMethodFn = wrenBindForeignMethod;

        WrenVM *vm = wrenNewVM(&config);

        for (int i = 0; i < count; i++)
        {
            if (strcmp(embedded[i].name, "main.wren") == 0)
                wrenInterpret(vm, embedded[i].name, embedded[i].source);
        }

        freeEmbedded(embedded, count);
        wrenFreeVM(vm);

        return 0;
    }

    if (argc < 2)
    {
        printf("Usage:\n");
        printf("\tbasil [file|dir] [arguments...]\n");
        printf("\tbasil build [dir]\n");
        printf("\tbasil version\n");
        return 1;
    }

    if (argc == 2 && strcmp(argv[1], "version") == 0)
    {
        printf("basil %s\n", BASIL_VERSION);
        return 0;
    }
    else if (strcmp(argv[1], "build") == 0)
    {
        if (argc != 3)
        {
            printf("Folder to build not specified\n");
            return 1;
        }

        return build(argv[0], argv[2]);
    }

    struct stat st;
    if (stat(argv[1], &st) != 0)
    {
        printf("Error opening: %s\n", argv[1]);
        return 1;
    }

    char *source;

    if (S_ISDIR(st.st_mode))
    {
        DIR *dir = opendir(argv[1]);
        if (dir == NULL)
        {
            printf("Error opening directory: %s\n", argv[1]);
            return 1;
        }

        struct dirent *entry;
        bool found = false;
        char path[MAX_PATH_SIZE];

        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, "main.wren") == 0)
            {
                found = true;
                snprintf(path, MAX_PATH_SIZE, "%s/%s", argv[1], entry->d_name);
                break;
            }
        }

        closedir(dir);

        if (!found)
        {
            printf("The directory does not contain a main.wren file\n");
            return 1;
        }

        strncpy(basePath, argv[1], MAX_PATH_SIZE - 1);
        basePath[MAX_PATH_SIZE - 1] = '\0';

        source = readFile(path);
        if (source == NULL)
            return 1;
    }
    else
    {
        int pathLen = (int)strlen(argv[1]);
        int extLen = (int)strlen(".wren");

        if (pathLen < extLen || strcmp(argv[1] + pathLen - extLen, ".wren") != 0)
        {
            printf("The file must be a .wren file\n");
            return 1;
        }

        strncpy(basePath, argv[1], MAX_PATH_SIZE - 1);
        basePath[MAX_PATH_SIZE - 1] = '\0';

        const char *lastSlash = strrchr(basePath, '/');
        const char *lastBackslash = strrchr(basePath, '\\');
        const char *lastSeparator = (lastSlash > lastBackslash) ? lastSlash : lastBackslash;

        if (lastSeparator != NULL)
            basePath[lastSeparator - basePath + 1] = '\0';

        source = readFile(argv[1]);
        if (source == NULL)
            return 1;
    }

    WrenConfiguration config;
    wrenInitConfiguration(&config);

    config.writeFn = wrenWrite;
    config.errorFn = wrenError;
    config.loadModuleFn = wrenLoadModule;
    config.bindForeignClassFn = wrenBindForeignClass;
    config.bindForeignMethodFn = wrenBindForeignMethod;

    WrenVM *vm = wrenNewVM(&config);

    wrenInterpret(vm, argv[1], source);

    free(source);
    wrenFreeVM(vm);

    return 0;
}
