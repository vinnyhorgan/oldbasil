#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *readFile(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        printf("Error opening file: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        printf("Error allocating memory\n");
        fclose(file);
        return NULL;
    }

    if (fread(buffer, 1, fileSize, file) != fileSize)
    {
        printf("Error reading file: %s\n", path);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[fileSize] = '\0';

    fclose(file);

    return buffer;
}

mfb_key stringToKey(const char *str)
{
    if (!str)
        return KB_KEY_UNKNOWN;

    int len = (int)strlen(str);
    if (len == 1)
    {
        int keyCode = tolower((unsigned char)str[0]);

        switch (keyCode)
        {
        case '\'':
            return KB_KEY_APOSTROPHE;
        case ',':
            return KB_KEY_COMMA;
        case '-':
            return KB_KEY_MINUS;
        case '.':
            return KB_KEY_PERIOD;
        case '/':
            return KB_KEY_SLASH;
        case '0':
            return KB_KEY_0;
        case '1':
            return KB_KEY_1;
        case '2':
            return KB_KEY_2;
        case '3':
            return KB_KEY_3;
        case '4':
            return KB_KEY_4;
        case '5':
            return KB_KEY_5;
        case '6':
            return KB_KEY_6;
        case '7':
            return KB_KEY_7;
        case '8':
            return KB_KEY_8;
        case '9':
            return KB_KEY_9;
        case ';':
            return KB_KEY_SEMICOLON;
        case '=':
            return KB_KEY_EQUAL;
        case 'a':
            return KB_KEY_A;
        case 'b':
            return KB_KEY_B;
        case 'c':
            return KB_KEY_C;
        case 'd':
            return KB_KEY_D;
        case 'e':
            return KB_KEY_E;
        case 'f':
            return KB_KEY_F;
        case 'g':
            return KB_KEY_G;
        case 'h':
            return KB_KEY_H;
        case 'i':
            return KB_KEY_I;
        case 'j':
            return KB_KEY_J;
        case 'k':
            return KB_KEY_K;
        case 'l':
            return KB_KEY_L;
        case 'm':
            return KB_KEY_M;
        case 'n':
            return KB_KEY_N;
        case 'o':
            return KB_KEY_O;
        case 'p':
            return KB_KEY_P;
        case 'q':
            return KB_KEY_Q;
        case 'r':
            return KB_KEY_R;
        case 's':
            return KB_KEY_S;
        case 't':
            return KB_KEY_T;
        case 'u':
            return KB_KEY_U;
        case 'v':
            return KB_KEY_V;
        case 'w':
            return KB_KEY_W;
        case 'x':
            return KB_KEY_X;
        case 'y':
            return KB_KEY_Y;
        case 'z':
            return KB_KEY_Z;
        case '[':
            return KB_KEY_LEFT_BRACKET;
        case '\\':
            return KB_KEY_BACKSLASH;
        case ']':
            return KB_KEY_RIGHT_BRACKET;
        case '`':
            return KB_KEY_GRAVE_ACCENT;
        default:
            return KB_KEY_UNKNOWN;
        }
    }

    if (strcmp(str, "space") == 0)
        return KB_KEY_SPACE;
    if (strcmp(str, "world1") == 0)
        return KB_KEY_WORLD_1;
    if (strcmp(str, "world2") == 0)
        return KB_KEY_WORLD_2;
    if (strcmp(str, "escape") == 0)
        return KB_KEY_ESCAPE;
    if (strcmp(str, "enter") == 0)
        return KB_KEY_ENTER;
    if (strcmp(str, "tab") == 0)
        return KB_KEY_TAB;
    if (strcmp(str, "backspace") == 0)
        return KB_KEY_BACKSPACE;
    if (strcmp(str, "insert") == 0)
        return KB_KEY_INSERT;
    if (strcmp(str, "delete") == 0)
        return KB_KEY_DELETE;
    if (strcmp(str, "right") == 0)
        return KB_KEY_RIGHT;
    if (strcmp(str, "left") == 0)
        return KB_KEY_LEFT;
    if (strcmp(str, "down") == 0)
        return KB_KEY_DOWN;
    if (strcmp(str, "up") == 0)
        return KB_KEY_UP;
    if (strcmp(str, "pageUp") == 0)
        return KB_KEY_PAGE_UP;
    if (strcmp(str, "pageDown") == 0)
        return KB_KEY_PAGE_DOWN;
    if (strcmp(str, "home") == 0)
        return KB_KEY_HOME;
    if (strcmp(str, "end") == 0)
        return KB_KEY_END;
    if (strcmp(str, "capsLock") == 0)
        return KB_KEY_CAPS_LOCK;
    if (strcmp(str, "scrollLock") == 0)
        return KB_KEY_SCROLL_LOCK;
    if (strcmp(str, "numLock") == 0)
        return KB_KEY_NUM_LOCK;
    if (strcmp(str, "printScreen") == 0)
        return KB_KEY_PRINT_SCREEN;
    if (strcmp(str, "pause") == 0)
        return KB_KEY_PAUSE;
    if (strcmp(str, "kpDecimal") == 0)
        return KB_KEY_KP_DECIMAL;
    if (strcmp(str, "kpDivide") == 0)
        return KB_KEY_KP_DIVIDE;
    if (strcmp(str, "kpMultiply") == 0)
        return KB_KEY_KP_MULTIPLY;
    if (strcmp(str, "kpSubtract") == 0)
        return KB_KEY_KP_SUBTRACT;
    if (strcmp(str, "kpAdd") == 0)
        return KB_KEY_KP_ADD;
    if (strcmp(str, "kpEnter") == 0)
        return KB_KEY_KP_ENTER;
    if (strcmp(str, "kpEqual") == 0)
        return KB_KEY_KP_EQUAL;
    if (strcmp(str, "leftShift") == 0)
        return KB_KEY_LEFT_SHIFT;
    if (strcmp(str, "leftControl") == 0)
        return KB_KEY_LEFT_CONTROL;
    if (strcmp(str, "leftAlt") == 0)
        return KB_KEY_LEFT_ALT;
    if (strcmp(str, "leftSuper") == 0)
        return KB_KEY_LEFT_SUPER;
    if (strcmp(str, "rightShift") == 0)
        return KB_KEY_RIGHT_SHIFT;
    if (strcmp(str, "rightControl") == 0)
        return KB_KEY_RIGHT_CONTROL;
    if (strcmp(str, "rightAlt") == 0)
        return KB_KEY_RIGHT_ALT;
    if (strcmp(str, "rightSuper") == 0)
        return KB_KEY_RIGHT_SUPER;
    if (strcmp(str, "menu") == 0)
        return KB_KEY_MENU;

    for (int i = 1; i <= 25; ++i)
    {
        char fKey[10];
        snprintf(fKey, sizeof(fKey), "f%d", i);

        if (strcmp(str, fKey) == 0)
            return KB_KEY_F1 + i - 1;
    }

    for (int i = 0; i <= 9; ++i)
    {
        char kpKey[10];
        snprintf(kpKey, sizeof(kpKey), "kp%d", i);

        if (strcmp(str, kpKey) == 0)
            return KB_KEY_KP_0 + i;
    }

    return KB_KEY_UNKNOWN;
}
