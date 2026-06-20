#include "processor.h"

// --- 1. TagRenamerProcessor ---
TagRenamerProcessor::TagRenamerProcessor(const char* pattern) : m_pattern(pattern) {}

void TagRenamerProcessor::Process(pfc::string_base& text, const metadb_handle_ptr& item) {
    if (m_pattern.is_empty()) return;

    titleformat_object::ptr script;
    titleformat_compiler::get()->compile_safe(script, m_pattern);

    item->format_title(nullptr, text, script, nullptr);
}

// --- 2. TransliteratorProcessor ---
// Базовый словарь. Сюда можно будет дописать любые нужные символы.
const std::unordered_map<std::string, std::string> TransliteratorProcessor::m_map = {
    {"é", "e"}, {"è", "e"}, {"ê", "e"}, {"ë", "e"},
    {"á", "a"}, {"à", "a"}, {"ä", "a"}, {"å", "a"},
    {"ó", "o"}, {"ò", "o"}, {"ö", "o"}, {"ø", "o"},
    {"í", "i"}, {"ì", "i"}, {"ï", "i"},
    {"ú", "u"}, {"ù", "u"}, {"ü", "u"},
    {"ñ", "n"}, {"ß", "ss"}, {"ç", "c"}
};

TransliteratorProcessor::TransliteratorProcessor(const char* replacement) : m_replacement(replacement) {}

void TransliteratorProcessor::Process(pfc::string_base& text, const metadb_handle_ptr& item) {
    std::string str = text.get_ptr();

    for (const auto& pair : m_map) {
        size_t pos = 0;
        while ((pos = str.find(pair.first, pos)) != std::string::npos) {
            str.replace(pos, pair.first.length(), pair.second);
            pos += pair.second.length();
        }
    }

    // TODO: Добавить логику замены остальных неизвестных символов на m_replacement
    text = str.c_str();
}

// --- 3. CaseConverterProcessor ---
const std::unordered_set<std::string> CaseConverterProcessor::m_stop_words = {
    "a", "an", "the", "and", "but", "or", "for", "nor", "on", "at", "to", "from", "by", "in", "of"
};

CaseConverterProcessor::CaseConverterProcessor(CaseMode mode) : m_mode(mode) {}

void CaseConverterProcessor::Process(pfc::string_base& text, const metadb_handle_ptr& item) {
    // В SDK foobar2000 есть встроенные функции для базовой смены регистра
    if (m_mode == CaseMode::LowerCase) {
        text = pfc::stringToLower(text).c_str();
    }
    else if (m_mode == CaseMode::AllCaps) {
        text = pfc::stringToUpper(text).c_str();
    }
    // TODO: Дописать реализацию для TitleCase, SmartTitleCase и SentenceCase
}