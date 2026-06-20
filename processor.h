#pragma once
#include <foobar2000/SDK/foobar2000.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

// Базовый интерфейс обработчика
class INameProcessor {
public:
    virtual ~INameProcessor() = default;
    virtual void Process(pfc::string_base& text, const metadb_handle_ptr& item) = 0;
};

// 1. Обработчик тегов
class TagRenamerProcessor : public INameProcessor {
    pfc::string8 m_pattern;
public:
    TagRenamerProcessor(const char* pattern);
    void Process(pfc::string_base& text, const metadb_handle_ptr& item) override;
};

// 2. Обработчик транслитерации
class TransliteratorProcessor : public INameProcessor {
    pfc::string8 m_replacement;
    static const std::unordered_map<std::string, std::string> m_map;
public:
    TransliteratorProcessor(const char* replacement = "");
    void Process(pfc::string_base& text, const metadb_handle_ptr& item) override;
};

// Режимы регистра
enum class CaseMode {
    AllCaps,
    LowerCase,
    TitleCase,
    SmartTitleCase,
    SentenceCase
};

// 3. Обработчик регистра
class CaseConverterProcessor : public INameProcessor {
    CaseMode m_mode;
    static const std::unordered_set<std::string> m_stop_words;
public:
    CaseConverterProcessor(CaseMode mode);
    void Process(pfc::string_base& text, const metadb_handle_ptr& item) override;
};