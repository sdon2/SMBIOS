#include <windows.h>
#include <stdio.h>

#define SMBIOS_SIGNATURE 'RSMB'

struct RawSMBIOSData {
    BYTE Used20CallingMethod;
    BYTE SMBIOSMajorVersion;
    BYTE SMBIOSMinorVersion;
    BYTE DmiRevision;
    DWORD Length;
    BYTE SMBIOSTableData[];
};

struct SMBIOSHeader {
    BYTE Type;
    BYTE Length;
    WORD Handle;
};

// Helper to get string from SMBIOS string table (index-based)
static const char* GetString(const struct SMBIOSHeader* hdr, BYTE stringIndex) {
    if (stringIndex == 0) return "";
    const char* p = (const char*)hdr + hdr->Length;
    for (int i = 1; i < stringIndex && *p != '\0'; i++) {
        p += strlen(p) + 1;
    }
    return p;
}

// Exported function to get serial number
__declspec(dllexport) int GetBiosSerial(char* buffer, DWORD bufferSize) {
    DWORD size = GetSystemFirmwareTable(SMBIOS_SIGNATURE, 0, NULL, 0);
    if (size == 0) return 0;

    struct RawSMBIOSData* smbiosData = (struct RawSMBIOSData*)malloc(size);
    if (!smbiosData) return 0;

    if (GetSystemFirmwareTable(SMBIOS_SIGNATURE, 0, smbiosData, size) == 0) {
        free(smbiosData);
        return 0;
    }

    const BYTE* p = smbiosData->SMBIOSTableData;
    const BYTE* end = smbiosData->SMBIOSTableData + smbiosData->Length;
    int found = 0;

    while (p < end) {
        const struct SMBIOSHeader* hdr = (const struct SMBIOSHeader*)p;
        
        // Type 1 is System Information (contains System Serial Number)
        if (hdr->Type == 1 && hdr->Length >= 0x08) {
            const BYTE* dataBytes = (const BYTE*)hdr;
            BYTE serialIndex = dataBytes[0x07]; // Offset 7 is Serial Number string index in Type 1
            const char* serial = GetString(hdr, serialIndex);
            
            if (serial && *serial != '\0') {
                snprintf(buffer, bufferSize, "%s", serial);
                found = 1;
                break;
            }
        }
        
        // Move past header and strings (terminated by double null 0x00 0x00)
        p += hdr->Length;
        while (p < end && !(p[0] == 0 && p[1] == 0)) {
            p++;
        }
        p += 2; // skip the double null bytes
    }

    free(smbiosData);
    return found;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return TRUE;
}
