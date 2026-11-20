// ---------------------------------------------------------------------------
// HZTest 单文件版启动器
//
// 把 HZTest.exe + Qt 运行库（Qt5Core/Gui/Widgets/Network、platforms 插件、
// MSVC 运行库）打包进自身的 RCDATA 资源里；运行时释放到
// %LOCALAPPDATA%\HZTest\runtime 并从那里启动真正的程序。
//
// 数据目录（hztest.ini / candidates.dat / records.dat / practice.dat /
// server.log）通过环境变量 HZTEST_DATA_DIR 指到本 exe 所在目录，
// 因此“单个 exe + 同目录数据文件”即可拷到别的电脑使用。
// ---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <string>
#include <vector>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")

static std::wstring ExeDir()
{
    wchar_t buf[1024];
    DWORD n = GetModuleFileNameW(NULL, buf, 1024);
    std::wstring s(buf, n);
    size_t p = s.find_last_of(L"\\/");
    return p == std::wstring::npos ? std::wstring(L".") : s.substr(0, p);
}

static std::wstring LocalAppData()
{
    wchar_t buf[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, buf)))
        return buf;
    return ExeDir();
}

static bool EnsureDir(const std::wstring &dir)
{
    if (dir.empty())
        return false;
    DWORD attr = GetFileAttributesW(dir.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES)
        return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
    size_t pos = dir.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        EnsureDir(dir.substr(0, pos));
    return CreateDirectoryW(dir.c_str(), NULL) != 0;
}

static std::wstring Widen(const char *s, int len)
{
    int n = MultiByteToWideChar(CP_UTF8, 0, s, len, NULL, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s, len, &w[0], n);
    return w;
}

static bool WriteAll(const std::wstring &path, const unsigned char *data, DWORD size)
{
    EnsureDir(path.substr(0, path.find_last_of(L"\\/")));
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    DWORD written = 0;
    BOOL ok = WriteFile(h, data, size, &written, NULL);
    CloseHandle(h);
    return ok != FALSE && written == size;
}

static bool IsWritable(const std::wstring &dir)
{
    std::wstring probe = dir + L"\\.hztest_write_probe";
    HANDLE h = CreateFileW(probe.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    CloseHandle(h);
    DeleteFileW(probe.c_str());
    return true;
}

static bool ReadStamp(const std::wstring &path, unsigned int *stamp)
{
    FILE *f = _wfopen(path.c_str(), L"rb");
    if (!f)
        return false;
    bool ok = fread(stamp, sizeof(unsigned int), 1, f) == 1;
    fclose(f);
    return ok;
}

static void WriteStamp(const std::wstring &path, unsigned int stamp)
{
    FILE *f = _wfopen(path.c_str(), L"wb");
    if (f) {
        fwrite(&stamp, sizeof(unsigned int), 1, f);
        fclose(f);
    }
}

// 把资源里的文件全部释放到 target 目录
static bool ExtractPayload(const std::wstring &target)
{
    HRSRC hr = FindResourceW(NULL, L"PAYLOAD", RT_RCDATA);
    if (!hr)
        return false;
    DWORD size = SizeofResource(NULL, hr);
    HGLOBAL hg = LoadResource(NULL, hr);
    const unsigned char *p = static_cast<const unsigned char *>(LockResource(hg));
    if (!p || size < 9)
        return false;
    if (memcmp(p, "HZPK1", 5) != 0)
        return false;
    p += 5;
    DWORD remain = size - 5;

    unsigned int count = *reinterpret_cast<const unsigned int *>(p);
    p += 4;
    remain -= 4;

    for (unsigned int i = 0; i < count && remain > 6; ++i) {
        unsigned short nlen = *reinterpret_cast<const unsigned short *>(p);
        p += 2;
        remain -= 2;
        if (nlen == 0 || remain < (DWORD)nlen + 4)
            break;
        std::wstring rel = Widen(reinterpret_cast<const char *>(p), nlen);
        p += nlen;
        remain -= nlen;

        unsigned int fsize = *reinterpret_cast<const unsigned int *>(p);
        p += 4;
        remain -= 4;
        if (remain < fsize)
            break;

        for (size_t k = 0; k < rel.size(); ++k) {
            if (rel[k] == L'/')
                rel[k] = L'\\';
        }
        // 写不进去（例如另一个实例正占用这些 DLL）就跳过，继续用已有文件
        WriteAll(target + L"\\" + rel, p, fsize);

        p += fsize;
        remain -= fsize;
    }
    return true;
}

static int Run()
{
    const std::wstring exeDir = ExeDir();
    const std::wstring runtime = LocalAppData() + L"\\HZTest\\runtime";
    EnsureDir(runtime);

    // 资源大小当作版本号：程序更新后资源大小变化，就会重新释放
    HRSRC hr = FindResourceW(NULL, L"PAYLOAD", RT_RCDATA);
    const unsigned int stamp = hr ? SizeofResource(NULL, hr) : 0;
    const std::wstring stampFile = runtime + L"\\payload.stamp";
    unsigned int oldStamp = 0;
    const bool hasStamp = ReadStamp(stampFile, &oldStamp);
    const std::wstring appExe = runtime + L"\\HZTest.exe";

    if (!hasStamp || oldStamp != stamp
            || GetFileAttributesW(appExe.c_str()) == INVALID_FILE_ATTRIBUTES) {
        if (ExtractPayload(runtime))
            WriteStamp(stampFile, stamp);
    }

    if (GetFileAttributesW(appExe.c_str()) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(NULL, L"程序文件释放失败，无法启动。\n请检查磁盘空间或杀毒软件是否拦截。",
                    L"HZTest", MB_ICONERROR);
        return 1;
    }

    // 数据目录：优先放在 exe 旁边（绿色版），不可写则退回 LocalAppData
    std::wstring dataDir = exeDir;
    if (!IsWritable(dataDir)) {
        dataDir = LocalAppData() + L"\\HZTest\\data";
        EnsureDir(dataDir);
    }
    SetEnvironmentVariableW(L"HZTEST_DATA_DIR", dataDir.c_str());

    std::wstring cmd = L"\"" + appExe + L"\"";
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(appExe.c_str(), &cmd[0], NULL, NULL, FALSE, 0, NULL,
                        runtime.c_str(), &si, &pi)) {
        MessageBoxW(NULL, L"启动失败。", L"HZTest", MB_ICONERROR);
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return static_cast<int>(code);
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    return Run();
}
