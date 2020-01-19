#pragma once

#ifdef _WIN32
#include <windows.h>
static void *dlopen(const char *filename, int flag) {
    wchar_t wpath[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wpath, MAX_PATH);
    return LoadLibraryW(wpath);
}
#define dlclose(n) FreeLibrary((HMODULE)(n))
#define dlsym(n,p) GetProcAddress((HMODULE)(n),(p))
#define RTLD_LAZY 0
#else
#include <dlfcn.h>
#endif
