#include <windows.h>

IDI_ICON1 ICON DISCARDABLE "logo_64x64.ico"


#define VER_FILEVERSION             1,0,0,0
#define VER_FILEVERSION_STR         "@PROJECT_VERSION@, @PROJECT_BITNESS@bit, @CMAKE_BUILD_TYPE@\0"

#define VER_PRODUCTVERSION          1,0,0,0
#define VER_PRODUCTVERSION_STR      "@PROJECT_VERSION@, @PROJECT_BITNESS@bit, @CMAKE_BUILD_TYPE@\0"

#define VER_COMPANYNAME_STR         "Jan Krieger"
#define VER_FILEDESCRIPTION_STR     "@PROJECT_LONGNAME@"
#define VER_INTERNALNAME_STR        "@PROJECT_LONGNAME@"
#define VER_LEGALCOPYRIGHT_STR      "@PROJECT_COPYRIGHT@"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "@PROJECT_LONGNAME@.exe"
#define VER_PRODUCTNAME_STR         "@PROJECT_LONGNAME@"


VS_VERSION_INFO VERSIONINFO
//FILEVERSION     VER_FILEVERSION
//PRODUCTVERSION  VER_PRODUCTVERSION
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904E4"
        BEGIN
            VALUE "CompanyName",        VER_COMPANYNAME_STR
            VALUE "FileDescription",    VER_FILEDESCRIPTION_STR
            VALUE "FileVersion",        VER_FILEVERSION_STR
            VALUE "InternalName",       VER_INTERNALNAME_STR
            VALUE "LegalCopyright",     VER_LEGALCOPYRIGHT_STR
            VALUE "LegalTrademarks1",   VER_LEGALTRADEMARKS1_STR
            VALUE "LegalTrademarks2",   VER_LEGALTRADEMARKS2_STR
            VALUE "OriginalFilename",   VER_ORIGINALFILENAME_STR
            VALUE "ProductName",        VER_PRODUCTNAME_STR
            VALUE "ProductVersion",     VER_PRODUCTVERSION_STR
        END
    END

    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1252
    END
END
