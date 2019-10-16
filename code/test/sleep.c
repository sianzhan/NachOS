#include "syscall.h"
main()
    {
        int a;
        for (a = 100; a > 98; a--)
        {
            Sleep(3733);
            PrintInt(a);
        }
        Sleep(10000);
    }
