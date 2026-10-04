#include "processor.h"
#include <windows.h>
#include <vector>
#include <string>

// --- 1. TagRenamerProcessor ---
TagRenamerProcessor::TagRenamerProcessor(const char* pattern) : m_pattern(pattern) {}

void TagRenamerProcessor::Process(pfc::string_base& text, const metadb_handle_ptr& item) {
    if (m_pattern.is_empty()) return;

    titleformat_object::ptr script;
    titleformat_compiler::get()->compile_safe(script, m_pattern);

    item->format_title(nullptr, text, script, nullptr);
}

// --- 2. TransliteratorProcessor ---

static wchar_t HandleSpecialLatinChar(wchar_t ch, std::wstring& out) {
    switch (ch) {
        // Венгерские двойные акуты
    case L'\u0151': return L'o';
    case L'\u0150': return L'O';
    case L'\u0171': return L'u';
    case L'\u0170': return L'U';

        // Скандинавские и немецкие
    case L'\u00DF': out += L"ss"; return 0;
    case L'\u00E6': out += L"ae"; return 0;
    case L'\u00C6': out += L"AE"; return 0;
    case L'\u00F8': return L'o';
    case L'\u00D8': return L'O';
    case L'\u00E5': return L'a';
    case L'\u00C5': return L'A';

        // Исландские / староанглийские
    case L'\u00F0': return L'd';
    case L'\u00D0': return L'D';
    case L'\u00FE': out += L"th"; return 0;
    case L'\u00DE': out += L"TH"; return 0;

        // Польские / славянские
    case L'\u0142': return L'l';
    case L'\u0141': return L'L';
    case L'\u0111': return L'd';
    case L'\u0110': return L'D';

    default: return ch;
    }
}

TransliteratorProcessor::TransliteratorProcessor(const char* replacement)
    : m_replacement(replacement) {
}

void TransliteratorProcessor::Process(pfc::string_base& text, const metadb_handle_ptr& item) {
    if (text.is_empty()) return;

    int wlen = MultiByteToWideChar(CP_UTF8, 0, text.get_ptr(), -1, nullptr, 0);
    if (wlen <= 1) return;

    std::wstring wsrc(wlen - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.get_ptr(), -1, &wsrc[0], wlen);

    int reqSize = FoldStringW(MAP_COMPOSITE, wsrc.c_str(),
        static_cast<int>(wsrc.length()), nullptr, 0);
    std::wstring decomposed;
    if (reqSize > 0) {
        decomposed.resize(reqSize);
        FoldStringW(MAP_COMPOSITE, wsrc.c_str(),
            static_cast<int>(wsrc.length()), &decomposed[0], reqSize);
    }
    else {
        decomposed = wsrc;
    }

    std::wstring cleaned;
    cleaned.reserve(decomposed.length());

    for (wchar_t ch : decomposed) {
        // [ИСПРАВЛЕНИЕ] Не вырезаем 0x0306 (Combining Breve для й/Й)
        // и 0x0308 (Combining Diaeresis для ё/Ё)
        if (ch >= 0x0300 && ch <= 0x036F) {
            if (ch == 0x0306 || ch == 0x0308) {  // чтобы оставить й и исправить ё: if (ch == 0x0306) {
                cleaned += ch;
            }
            continue;
        }

        if (ch < 0x20) continue;

        wchar_t res = HandleSpecialLatinChar(ch, cleaned);
        if (res != 0) {
            cleaned += res;
        }
    }

    // Собираем обратно (и + 0x0306 -> й, И + 0x0306 -> Й)
    int precomposedSize = FoldStringW(MAP_PRECOMPOSED, cleaned.c_str(),
        static_cast<int>(cleaned.length()), nullptr, 0);
    if (precomposedSize > 0) {
        std::wstring recomposed(precomposedSize, L'\0');
        FoldStringW(MAP_PRECOMPOSED, cleaned.c_str(),
            static_cast<int>(cleaned.length()), &recomposed[0], precomposedSize);
        cleaned = recomposed;
    }

    // Замена неизвестных символов (если задано)
    if (!m_replacement.is_empty()) {
        std::wstring repl;
        int rlen = MultiByteToWideChar(CP_UTF8, 0, m_replacement.get_ptr(), -1, nullptr, 0);
        if (rlen > 1) {
            repl.resize(rlen - 1);
            MultiByteToWideChar(CP_UTF8, 0, m_replacement.get_ptr(), -1, &repl[0], rlen);
        }
    }

    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, cleaned.c_str(),
        static_cast<int>(cleaned.length()),
        nullptr, 0, nullptr, nullptr);
    if (utf8Len > 0) {
        std::string resultUtf8(utf8Len, '\0');
        WideCharToMultiByte(CP_UTF8, 0, cleaned.c_str(),
            static_cast<int>(cleaned.length()),
            &resultUtf8[0], utf8Len, nullptr, nullptr);
        text = resultUtf8.c_str();
    }
}