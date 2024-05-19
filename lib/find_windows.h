#pragma once

// TODO fully support windows 11
//
// Author:   Brandon Kirincich
// Version:  1.1
// Date:     14 May, 2024
//
// Based on:
//
// Author:   Jonathan Blow
// Version:  1
// Date:     31 August, 2018
// https://opensource.org/licenses/MIT

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>

#include <stdint.h>
#include <io.h>

struct FindResult {
    int windows_sdk_version = 0;

    wchar_t *windows_sdk_root = nullptr;
    wchar_t *windows_sdk_um_library_path = nullptr;
    wchar_t *windows_sdk_ucrt_library_path = nullptr;

    wchar_t *vs_exe_path = nullptr;
    wchar_t *vs_library_path = nullptr;
};

FindResult find_visual_studio_and_windows_sdk();

inline void free_resources(FindResult *result) {
    free(result->windows_sdk_root);
    free(result->windows_sdk_um_library_path);
    free(result->windows_sdk_ucrt_library_path);
    free(result->vs_exe_path);
    free(result->vs_library_path);
}

#define CONCAT_INTERNAL(x, y) x##y
#define CONCAT(x, y) CONCAT_INTERNAL(x, y)

template <typename T>
struct ExitScope {
    T lambda;
    ExitScope(T lambda) : lambda(lambda) {}
    ~ExitScope() {
        lambda();
    }
    ExitScope(const ExitScope &);

private:
    ExitScope &operator=(const ExitScope &);
};

class ExitScopeHelp {
public:
    template <typename T>
    ExitScope<T> operator+(T t) {
        return t;
    }
};

#define MSC_DEFER                                                              \
    const auto &CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()

struct DECLSPEC_UUID("B41463C3-8866-43B5-BC33-2B0676F7F42E")
    DECLSPEC_NOVTABLE ISetupInstance : public IUnknown {
    STDMETHOD(GetInstanceId)(_Out_ BSTR *pbstrInstanceId) = 0;
    STDMETHOD(GetInstallDate)(_Out_ LPFILETIME pInstallDate) = 0;
    STDMETHOD(GetInstallationName)(_Out_ BSTR *pbstrInstallationName) = 0;
    STDMETHOD(GetInstallationPath)(_Out_ BSTR *pbstrInstallationPath) = 0;
    STDMETHOD(GetInstallationVersion)(_Out_ BSTR *pbstrInstallationVersion) = 0;
    STDMETHOD(GetDisplayName)(_In_ LCID lcid, _Out_ BSTR *pbstrDisplayName) = 0;
    STDMETHOD(GetDescription)(_In_ LCID lcid, _Out_ BSTR *pbstrDescription) = 0;
    STDMETHOD(ResolvePath)
    (_In_opt_z_ LPCOLESTR pwszRelativePath, _Out_ BSTR *pbstrAbsolutePath) = 0;
};

struct DECLSPEC_UUID("6380BCFF-41D3-4B2E-8B2E-BF8A6810C848")
    DECLSPEC_NOVTABLE IEnumSetupInstances : public IUnknown {
    STDMETHOD(Next)
    (_In_ ULONG celt,
     _Out_writes_to_(celt, *pceltFetched) ISetupInstance **rgelt,
     _Out_opt_ _Deref_out_range_(0, celt) ULONG *pceltFetched) = 0;
    STDMETHOD(Skip)(_In_ ULONG celt) = 0;
    STDMETHOD(Reset)(void) = 0;
    STDMETHOD(Clone)(_Deref_out_opt_ IEnumSetupInstances **ppenum) = 0;
};

struct DECLSPEC_UUID("42843719-DB4C-46C2-8E7C-64F1816EFD5B")
    DECLSPEC_NOVTABLE ISetupConfiguration : public IUnknown {
    STDMETHOD(EnumInstances)(_Out_ IEnumSetupInstances **ppEnumInstances) = 0;
    STDMETHOD(GetInstanceForCurrentProcess)
    (_Out_ ISetupInstance **ppInstance) = 0;
    STDMETHOD(GetInstanceForPath)
    (_In_z_ LPCWSTR wzPath, _Out_ ISetupInstance **ppInstance) = 0;
};

struct Version_Data {
    int32_t best_version[4]; // For Windows 8 versions, only two of these
                             // numbers are used.
    wchar_t *best_name;
};

inline bool os_file_exists(wchar_t *name) {
    // @Robustness: What flags do we really want to check here?

    auto attrib = GetFileAttributesW(name);
    if (attrib == INVALID_FILE_ATTRIBUTES)
        return false;
    if (attrib & FILE_ATTRIBUTE_DIRECTORY)
        return false;

    return true;
}

inline wchar_t *concat(wchar_t const *a, wchar_t const *b,
                       wchar_t const *c = nullptr, wchar_t const *d = nullptr) {
    // Concatenate up to 4 wide strings together. Allocated with malloc.

    auto len_a = wcslen(a);
    auto len_b = wcslen(b);

    auto len_c = 0;
    if (c)
        len_c = (int)wcslen(c);

    auto len_d = 0;
    if (d)
        len_d = (int)wcslen(d);

    wchar_t *result =
        (wchar_t *)malloc((len_a + len_b + len_c + len_d + 1) * 2);
    memcpy(result, a, len_a * 2);
    memcpy(result + len_a, b, len_b * 2);

    if (c)
        memcpy(result + len_a + len_b, c, len_c * 2);
    if (d)
        memcpy(result + len_a + len_b + len_c, d, len_d * 2);

    result[len_a + len_b + len_c + len_d] = 0;

    return result;
}

typedef void (*Visit_Proc_W)(wchar_t *short_name, wchar_t *full_name,
                             Version_Data *data);
inline bool visit_files_w(wchar_t *dir_name, Version_Data *data,
                          Visit_Proc_W proc) {

    // Visit everything in one folder (non-recursively). If it's a directory
    // that doesn't start with ".", call the visit proc on it. The visit proc
    // will see if the filename conforms to the expected versioning pattern.

    auto wildcard_name = concat(dir_name, L"\\*");
    MSC_DEFER {
        free(wildcard_name);
    };

    WIN32_FIND_DATAW find_data;
    auto handle = FindFirstFileW(wildcard_name, &find_data);
    if (handle == INVALID_HANDLE_VALUE)
        return false;

    while (true) {
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            (find_data.cFileName[0] != '.')) {
            auto full_name = concat(dir_name, L"\\", find_data.cFileName);
            MSC_DEFER {
                free(full_name);
            };

            proc(find_data.cFileName, full_name, data);
        }

        auto success = FindNextFileW(handle, &find_data);
        if (!success)
            break;
    }

    FindClose(handle);

    return true;
}

inline wchar_t *find_windows_kit_root(HKEY key, wchar_t const *version) {
    // Given a key to an already opened registry entry,
    // get the value stored under the 'version' subkey.

    DWORD required_length;
    auto rc =
        RegQueryValueExW(key, version, NULL, NULL, NULL, &required_length);
    if (rc != 0)
        return NULL;

    DWORD length =
        required_length + 2; // The +2 is for the maybe optional zero later on.
                             // Probably we are over-allocating.
    wchar_t *value = (wchar_t *)malloc(length);
    if (!value)
        return NULL;

    rc =
        RegQueryValueExW(key, version, NULL, NULL, (LPBYTE)value,
                         &length); // We know that version is zero-terminated...
    if (rc != 0)
        return NULL;

    // The documentation says that if the string for some reason was not stored
    // with zero-termination, we need to manually terminate it.

    if (value[length]) {
        value[length + 1] = 0;
    }

    return value;
}

inline void win10_best(wchar_t *short_name, wchar_t *full_name,
                       Version_Data *data) {
    // Find the Windows 10 subdirectory with the highest version number.

    int i0, i1, i2, i3;
    auto success = swscanf_s(short_name, L"%d.%d.%d.%d", &i0, &i1, &i2, &i3);
    if (success < 4)
        return;

    if (i0 < data->best_version[0])
        return;
    else if (i0 == data->best_version[0]) {
        if (i1 < data->best_version[1])
            return;
        else if (i1 == data->best_version[1]) {
            if (i2 < data->best_version[2])
                return;
            else if (i2 == data->best_version[2]) {
                if (i3 < data->best_version[3])
                    return;
            }
        }
    }

    // we have to copy_string and free here because visit_files free's the
    // full_name string after we execute this function, so Win*_Data would
    // contain an invalid pointer.
    if (data->best_name)
        free(data->best_name);
    data->best_name = _wcsdup(full_name);

    if (data->best_name) {
        data->best_version[0] = i0;
        data->best_version[1] = i1;
        data->best_version[2] = i2;
        data->best_version[3] = i3;
    }
}

inline void win8_best(wchar_t *short_name, wchar_t *full_name,
                      Version_Data *data) {
    // Find the Windows 8 subdirectory with the highest version number.

    int i0, i1;
    auto success = swscanf_s(short_name, L"winv%d.%d", &i0, &i1);
    if (success < 2)
        return;

    if (i0 < data->best_version[0])
        return;
    else if (i0 == data->best_version[0]) {
        if (i1 < data->best_version[1])
            return;
    }

    // we have to copy_string and free here because visit_files free's the
    // full_name string after we execute this function, so Win*_Data would
    // contain an invalid pointer.
    if (data->best_name)
        free(data->best_name);
    data->best_name = _wcsdup(full_name);

    if (data->best_name) {
        data->best_version[0] = i0;
        data->best_version[1] = i1;
    }
}

inline void find_windows_kit_root(FindResult *result) {
    // Information about the Windows 10 and Windows 8 development kits
    // is stored in the same place in the registry. We open a key
    // to that place, first checking preferntially for a Windows 10 kit,
    // then, if that's not found, a Windows 8 kit.

    HKEY main_key;

    auto rc = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots", 0,
        KEY_QUERY_VALUE | KEY_WOW64_32KEY | KEY_ENUMERATE_SUB_KEYS, &main_key);
    if (rc != S_OK)
        return;
    MSC_DEFER {
        RegCloseKey(main_key);
    };

    // Look for a Windows 10 entry.
    auto windows10_root = find_windows_kit_root(main_key, L"KitsRoot10");

    if (windows10_root) {
        MSC_DEFER {
            free(windows10_root);
        };
        Version_Data data = {0};
        auto windows10_lib = concat(windows10_root, L"Lib");
        MSC_DEFER {
            free(windows10_lib);
        };

        visit_files_w(windows10_lib, &data, win10_best);
        if (data.best_name) {
            result->windows_sdk_version = 10;
            result->windows_sdk_root = data.best_name;
            return;
        }
    }

    // Look for a Windows 8 entry.
    auto windows8_root = find_windows_kit_root(main_key, L"KitsRoot81");

    if (windows8_root) {
        MSC_DEFER {
            free(windows8_root);
        };

        auto windows8_lib = concat(windows8_root, L"Lib");
        MSC_DEFER {
            free(windows8_lib);
        };

        Version_Data data = {0};
        visit_files_w(windows8_lib, &data, win8_best);
        if (data.best_name) {
            result->windows_sdk_version = 8;
            result->windows_sdk_root = data.best_name;
            return;
        }
    }

    // If we get here, we failed to find anything.
}

inline void
find_visual_studio_by_fighting_through_microsoft_craziness(FindResult *result) {
    auto rc = CoInitialize(NULL);
    // "Subsequent valid calls return false." So ignore false.
    // if rc != S_OK  return false;

    GUID my_uid = {0x42843719,
                   0xDB4C,
                   0x46C2,
                   {0x8E, 0x7C, 0x64, 0xF1, 0x81, 0x6E, 0xFD, 0x5B}};
    GUID CLSID_SetupConfiguration = {
        0x177F0C4A,
        0x1CD3,
        0x4DE7,
        {0xA3, 0x2C, 0x71, 0xDB, 0xBB, 0x9F, 0xA3, 0x6D}};

    ISetupConfiguration *config = NULL;
    auto hr = CoCreateInstance(CLSID_SetupConfiguration, NULL,
                               CLSCTX_INPROC_SERVER, my_uid, (void **)&config);
    if (hr != 0)
        return;
    MSC_DEFER {
        config->Release();
    };

    IEnumSetupInstances *instances = NULL;
    hr = config->EnumInstances(&instances);
    if (hr != 0)
        return;
    if (!instances)
        return;
    MSC_DEFER {
        instances->Release();
    };

    while (1) {
        ULONG found = 0;
        ISetupInstance *instance = NULL;
        auto hr = instances->Next(1, &instance, &found);
        if (hr != S_OK)
            break;

        MSC_DEFER {
            instance->Release();
        };

        BSTR bstr_inst_path;
        hr = instance->GetInstallationPath(&bstr_inst_path);
        if (hr != S_OK)
            continue;
        MSC_DEFER {
            SysFreeString(bstr_inst_path);
        };

        auto tools_filename = concat(
            bstr_inst_path,
            L"\\VC\\Auxiliary\\Build\\Microsoft.VCToolsVersion.default.txt");
        MSC_DEFER {
            free(tools_filename);
        };

        FILE *f = nullptr;
        auto open_result = _wfopen_s(&f, tools_filename, L"rt");
        if (open_result != 0)
            continue;
        if (!f)
            continue;
        MSC_DEFER {
            fclose(f);
        };

        LARGE_INTEGER tools_file_size;
        auto file_handle = (HANDLE)_get_osfhandle(_fileno(f));
        BOOL success = GetFileSizeEx(file_handle, &tools_file_size);
        if (!success)
            continue;

        auto version_bytes =
            (tools_file_size.QuadPart + 1) *
            2; // Warning: This multiplication by 2 presumes there is no
               // variable-length encoding in the wchars (wacky characters in
               // the file could betray this expectation).
        wchar_t *version = (wchar_t *)malloc(version_bytes);
        MSC_DEFER {
            free(version);
        };

        auto read_result = fgetws(version, (int)version_bytes, f);
        if (!read_result)
            continue;

        auto version_tail = wcschr(version, '\n');
        if (version_tail)
            *version_tail = 0; // Stomp the data

        auto library_path = concat(bstr_inst_path, L"\\VC\\Tools\\MSVC\\",
                                   version, L"\\lib\\x64");
        auto library_file =
            concat(library_path,
                   L"\\vcruntime.lib"); // @Speed: Could have library_path point
                                        // to this string, with a smaller count,
                                        // to save on memory flailing!

        if (os_file_exists(library_file)) {
            auto link_exe_path = concat(bstr_inst_path, L"\\VC\\Tools\\MSVC\\",
                                        version, L"\\bin\\Hostx64\\x64");
            result->vs_exe_path = link_exe_path;
            result->vs_library_path = library_path;
            return;
        }

        /*
           Ryan Saunderson said:
           "Clang uses the 'SetupInstance->GetInstallationVersion' /
           ISetupHelper->ParseVersion to find the newest version and then reads
           the tools file to define the tools path - which is definitely better
           than what i did."

           So... @Incomplete: Should probably pick the newest version...
        */
    }

    // If we get here, we didn't find Visual Studio 2017. Try earlier versions.

    HKEY vs7_key;
    rc = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                       "SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7", 0,
                       KEY_QUERY_VALUE | KEY_WOW64_32KEY, &vs7_key);

    if (rc != S_OK)
        return;
    MSC_DEFER {
        RegCloseKey(vs7_key);
    };

    // Hardcoded search for 4 prior Visual Studio versions. Is there something
    // better to do here?
    wchar_t const *versions[] = {L"14.0", L"12.0", L"11.0", L"10.0"};
    const int NUM_VERSIONS = sizeof(versions) / sizeof(versions[0]);

    for (int i = 0; i < NUM_VERSIONS; i++) {
        auto v = versions[i];

        DWORD dw_type;
        DWORD cb_data;

        auto rc = RegQueryValueExW(vs7_key, v, NULL, &dw_type, NULL, &cb_data);
        if ((rc == ERROR_FILE_NOT_FOUND) || (dw_type != REG_SZ)) {
            continue;
        }

        auto buffer = (wchar_t *)malloc(cb_data);
        if (!buffer)
            return;
        MSC_DEFER {
            free(buffer);
        };

        rc = RegQueryValueExW(vs7_key, v, NULL, NULL, (LPBYTE)buffer, &cb_data);
        if (rc != 0)
            continue;

        // @Robustness: Do the zero-termination thing suggested in the
        // RegQueryValue docs?

        auto lib_path = concat(buffer, L"VC\\Lib\\amd64");

        // Check to see whether a vcruntime.lib actually exists here.
        auto vcruntime_filename = concat(lib_path, L"\\vcruntime.lib");
        MSC_DEFER {
            free(vcruntime_filename);
        };

        if (os_file_exists(vcruntime_filename)) {
            result->vs_exe_path = concat(buffer, L"VC\\bin");
            result->vs_library_path = lib_path;
            return;
        }

        free(lib_path);
    }

    // If we get here, we failed to find anything.
}

inline FindResult find_visual_studio_and_windows_sdk() {
    FindResult result;

    find_windows_kit_root(&result);

    if (result.windows_sdk_root) {
        result.windows_sdk_um_library_path =
            concat(result.windows_sdk_root, L"\\um\\x64");
        result.windows_sdk_ucrt_library_path =
            concat(result.windows_sdk_root, L"\\ucrt\\x64");
    }

    find_visual_studio_by_fighting_through_microsoft_craziness(&result);

    return result;
}
