// Pack a directory into the HZPK1 archive used by the portable launcher.
// usage: packer.exe <dir> <out.bin>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>

static std::vector<std::pair<std::string, std::string> > g_files; // (relative, full)

static std::string Utf8(const std::wstring &w)
{
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], n, NULL, NULL);
    return s;
}

static void Walk(const std::wstring &root, const std::wstring &dir)
{
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE)
        return;
    do {
        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
            continue;
        std::wstring full = dir + L"\\" + fd.cFileName;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            Walk(root, full);
        } else {
            std::wstring rel = full.substr(root.size() + 1);
            std::replace(rel.begin(), rel.end(), L'\\', L'/');
            g_files.push_back(std::make_pair(Utf8(rel), Utf8(full)));
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

int wmain(int argc, wchar_t **argv)
{
    if (argc < 3) {
        wprintf(L"usage: packer <dir> <out.bin>\n");
        return 1;
    }
    std::wstring root = argv[1];
    Walk(root, root);
    std::sort(g_files.begin(), g_files.end());

    FILE *f = _wfopen(argv[2], L"wb");
    if (!f) {
        wprintf(L"cannot write output\n");
        return 1;
    }
    fwrite("HZPK1", 1, 5, f);
    unsigned int count = (unsigned int)g_files.size();
    fwrite(&count, 4, 1, f);

    for (size_t i = 0; i < g_files.size(); ++i) {
        const std::string &rel = g_files[i].first;
        FILE *in = fopen(g_files[i].second.c_str(), "rb");
        if (!in)
            continue;
        fseek(in, 0, SEEK_END);
        long size = ftell(in);
        fseek(in, 0, SEEK_SET);
        std::vector<char> buf(size > 0 ? size : 0);
        if (size > 0)
            fread(&buf[0], 1, size, in);
        fclose(in);

        unsigned short nlen = (unsigned short)rel.size();
        fwrite(&nlen, 2, 1, f);
        fwrite(rel.data(), 1, nlen, f);
        unsigned int fsize = (unsigned int)size;
        fwrite(&fsize, 4, 1, f);
        if (size > 0)
            fwrite(&buf[0], 1, size, f);
        printf("  + %s (%ld bytes)\n", rel.c_str(), size);
    }
    fclose(f);
    printf("packed %u files\n", count);
    return 0;
}
