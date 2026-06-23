#include <foobar2000/SDK/foobar2000.h>

// Обязательная декларация информации о плагине
DECLARE_COMPONENT_VERSION(
    "File Name Editor",
    "1.1",
    "Advanced file renamer with transliteration and case conversion.\n"
);

// Защита от переименования файла 
VALIDATE_COMPONENT_FILENAME("foo_filename_editor.dll");