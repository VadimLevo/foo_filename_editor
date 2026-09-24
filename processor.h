#pragma once
#include <foobar2000/SDK/foobar2000.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

class INameProcessor {
public:
    virtual ~INameProcessor() = default;
    virtual void Process(pfc::string_base& text, const metadb_handle_ptr& item) = 0;
};

class TagRenamerProcessor : public INameProcessor {
    pfc::string8 m_pattern;
public:
    explicit TagRenamerProcessor(const char* pattern);
    void Process(pfc::string_base& text, const metadb_handle_ptr& item) override;
};

class TransliteratorProcessor : public INameProcessor {
    pfc::string8 m_replacement;
public:
    explicit TransliteratorProcessor(const char* replacement = "");
    void Process(pfc::string_base& text, const metadb_handle_ptr& item) override;
};

// [REFACTOR] CaseConverterProcessor удалён — вся логика регистра в editor_dialog.cpp
// (было дублирование логики между processor.cpp и editor_dialog.cpp)