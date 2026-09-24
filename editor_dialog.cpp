#include "editor_dialog.h"
#include <atlstr.h>
#include <vector>
#include <memory>
#include <cwctype>
#include <set>
#include <regex>
#include <string>

// --- КОНФИГУРАЦИЯ ---
static const GUID guid_history = { 0x8f2d5e7a, 0xc143, 0x4b9a, { 0x82, 0x14, 0x9d, 0x3e, 0xc5, 0x7f, 0x2a, 0x11 } };
static const GUID guid_single = { 0x3c9b1a4f, 0x8d72, 0x4e6b, { 0x95, 0x20, 0xf1, 0x8c, 0x3d, 0x6a, 0x5b, 0x22 } };
static const GUID guid_va = { 0x5a1f8c3e, 0x2b9d, 0x47a1, { 0xa4, 0x6e, 0xc8, 0x15, 0x9b, 0x3f, 0x0d, 0x77 } };
static const GUID guid_case = { 0xc5fbeb29, 0x8371, 0x4986, { 0xab, 0x05, 0x6f, 0x92, 0x1e, 0x20, 0xcc, 0x88 } };

static const GUID guid_col0 = { 0x81b7a2e5, 0x4f3d, 0x4a92, { 0x87, 0xc1, 0x2b, 0x3f, 0x4e, 0x5d, 0x6a, 0x7c } };
static const GUID guid_col1 = { 0x92c8b3f6, 0x504e, 0x5b03, { 0x98, 0xd2, 0x3c, 0x40, 0x5f, 0x6e, 0x7b, 0x8d } };
static const GUID guid_col2 = { 0xa3d9c407, 0x615f, 0x6c14, { 0xa9, 0xe3, 0x4d, 0x51, 0x60, 0x7f, 0x8c, 0x9e } };

static const GUID guid_chk_tags = { 0xf1c2b3a4, 0xd5e6, 0x7f8a, { 0x9b, 0x0c, 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x6c } };
static const GUID guid_chk_translit = { 0xe2d3c4b5, 0xa6f7, 0x8a9b, { 0x0c, 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x6c, 0x7d } };
static const GUID guid_chk_case_chk = { 0xd3c4b5a6, 0xf7e8, 0x9b0c, { 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x6c, 0x7d, 0x8e } };

static const GUID guid_m3u_history = { 0xa1b2c3d4, 0xe5f6, 0x47a8, { 0x9b, 0x0c, 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x6c } };
static const GUID guid_m3u_default = { 0xb2c3d4e5, 0xf6a7, 0x48b9, { 0x0c, 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x6c, 0x7d } };
static const GUID guid_chk_m3u = { 0xc3d4e5f6, 0xa7b8, 0x49ca, { 0x1d, 0x2e, 0x3f, 0x4a, 0x5b, 0x6c, 0x7d, 0x8e } };
static const GUID guid_chk_extinf = { 0xd4e5f6a7, 0xb8c9, 0x4adb, { 0x2e, 0x3f, 0x4a, 0x5b, 0x6c, 0x7d, 0x8e, 0x9f } };
static const GUID guid_chk_overwr = { 0xe5f6a7b8, 0xc9d0, 0x4bec, { 0x3f, 0x4a, 0x5b, 0x6c, 0x7d, 0x8e, 0x9f, 0xa0 } };

cfg_string cfg_pattern_history(guid_history, "");
cfg_string cfg_default_single(guid_single, "%tracknumber% - %title%");
cfg_string cfg_default_va(guid_va, "%tracknumber% - %artist% - %title%");
cfg_int    cfg_default_case(guid_case, 0);

cfg_int cfg_col_width_0(guid_col0, 150);
cfg_int cfg_col_width_1(guid_col1, 150);
cfg_int cfg_col_width_2(guid_col2, 100);

cfg_int cfg_use_tags(guid_chk_tags, 1);
cfg_int cfg_use_translit(guid_chk_translit, 0);
cfg_int cfg_use_case(guid_chk_case_chk, 0);

cfg_string cfg_m3u_history(guid_m3u_history, "");
cfg_string cfg_m3u_default(guid_m3u_default, "%album%");
cfg_int cfg_use_m3u(guid_chk_m3u, 0);
cfg_int cfg_use_extinf(guid_chk_extinf, 1);
cfg_int cfg_use_overwrite(guid_chk_overwr, 0);

// =========================================================================
//  УТИЛИТЫ
// =========================================================================

struct CStringLess {
    // Регистронезависимое сравнение (Windows-семантика)
    bool operator()(const CString& a, const CString& b) const {
        return a.CompareNoCase(b) < 0;
    }
};

// [FIX] Безопасное чтение текста из контрола — без магической 256
static CString GetDlgItemTextSafe(HWND hDlg, int id) {
    CString s;
    HWND h = ::GetDlgItem(hDlg, id);
    if (!h) return s;
    int len = ::GetWindowTextLengthW(h);
    ::GetWindowTextW(h, s.GetBuffer(len + 1), len + 1);
    s.ReleaseBuffer(len);
    return s;
}

wchar_t SafeToUpper(wchar_t c) { wchar_t buf[2] = { c, 0 }; CharUpperW(buf); return buf[0]; }
wchar_t SafeToLower(wchar_t c) { wchar_t buf[2] = { c, 0 }; CharLowerW(buf); return buf[0]; }
bool SafeIsAlpha(wchar_t c) { return IsCharAlphaW(c) != 0; }

// [FIX] SanitizeFilename: reserved names, trailing dots/spaces, пустое имя
void SanitizeFilename(pfc::string8& name) {
    CString wName = pfc::stringcvt::string_os_from_utf8(name).get_ptr();

    const wchar_t* invalid = L"<>:\"/\\|?*";
    for (int i = 0; invalid[i] != L'\0'; i++) wName.Replace(invalid[i], L'_');

    // Управляющие символы 0x00-0x1F
    for (int i = 0; i < 0x20; ++i) wName.Remove((wchar_t)i);

    // Trailing dots/spaces
    while (!wName.IsEmpty()) {
        wchar_t last = wName[wName.GetLength() - 1];
        if (last == L'.' || last == L' ') wName.Delete(wName.GetLength() - 1);
        else break;
    }

    // Reserved DOS-имена
    static const wchar_t* kReserved[] = {
        L"CON", L"PRN", L"AUX", L"NUL",
        L"COM1", L"COM2", L"COM3", L"COM4", L"COM5",
        L"COM6", L"COM7", L"COM8", L"COM9",
        L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5",
        L"LPT6", L"LPT7", L"LPT8", L"LPT9"
    };
    CString upper = wName; upper.MakeUpper();
    for (auto* r : kReserved) {
        CString rw(r);
        if (upper == rw || upper.Left(rw.GetLength() + 1) == rw + L".") {
            wName = L"_" + wName;
            break;
        }
    }

    if (wName.IsEmpty()) wName = L"Unknown";

    name = pfc::stringcvt::string_utf8_from_os(wName).get_ptr();
}

// [FIX] regex — статический, компилируется один раз
bool IsRomanNumeral(const CString& str) {
    CString w = str;
    w.MakeUpper();  // [FIX] вместо CharUpperW(w.GetBuffer())

    if (w == L"MIX" || w == L"DIV" || w == L"DI" ||
        w == L"MI" || w == L"LI" || w == L"CIV") return false;

    static const std::wregex r(
        L"^(M{0,3})(CM|CD|D?C{0,3})(XC|XL|L?X{0,3})(IX|IV|V?I{0,3})$");
    return std::regex_match(w.GetString(), r) && w.GetLength() > 0;
}

// =========================================================================
//  LIGATURE PROCESSOR
// =========================================================================

class LigatureProcessor : public INameProcessor {
public:
    void Process(pfc::string_base& text, const metadb_handle_ptr& item) override {
        CString wName = pfc::stringcvt::string_os_from_utf8(text.get_ptr()).get_ptr();
        wName.Replace(L"\u00E6", L"ae"); wName.Replace(L"\u00C6", L"Ae");
        wName.Replace(L"\u0153", L"oe"); wName.Replace(L"\u0152", L"Oe");
        wName.Replace(L"\u00DF", L"ss");
        wName.Replace(L"\u00F0", L"d");  wName.Replace(L"\u00D0", L"D");
        wName.Replace(L"\u00FE", L"th"); wName.Replace(L"\u00DE", L"Th");
        wName.Replace(L"\u00D8", L"O");  wName.Replace(L"\u00F8", L"o");
        text = pfc::stringcvt::string_utf8_from_os(wName).get_ptr();
    }
};

// =========================================================================
//  CASE CONVERSION
// =========================================================================

void ApplyCaseConversion(pfc::string8& name, int mode) {
    if (mode < 0 || mode > 4 || name.is_empty()) return;
    CString wName = pfc::stringcvt::string_os_from_utf8(name).get_ptr();

    if (mode == 3) {           // ALL CAPS
        wName.MakeUpper();     // [FIX] вместо CharUpperW(GetBuffer())
    }
    else if (mode == 4) {      // lowercase
        wName.MakeLower();
    }
    else if (mode == 2) {      // Sentence case
        wName.MakeLower();
        for (int i = 0; i < wName.GetLength(); ++i) {
            if (SafeIsAlpha(wName[i])) {
                wName.SetAt(i, SafeToUpper(wName[i]));
                break;
            }
        }
    }
    else if (mode == 0 || mode == 1) {  // Title Case / Smart Title Case
        static const std::set<CStringW, CStringLess> smallWords = {
            L"a", L"an", L"the",
            L"and", L"but", L"or", L"nor", L"for", L"so", L"yet",
            L"as", L"at", L"by", L"in", L"of", L"on", L"to", L"up",
            L"off", L"out", L"into", L"over", L"with", L"from", L"via", L"near"
        };

        struct Token { CStringW text; bool isWord; };
        std::vector<Token> tokens;
        CStringW currentToken;
        bool inWord = false;
        int length = wName.GetLength();

        for (int i = 0; i < length; ++i) {
            wchar_t ch = wName[i];
            bool isAlphaNum = (IsCharAlphaNumericW(ch) != 0);
            if ((ch == L'\'' || ch == L'\u2019') && inWord &&
                i + 1 < length && IsCharAlphaNumericW(wName[i + 1])) {
                isAlphaNum = true;
            }

            if (isAlphaNum) {
                if (!inWord && !currentToken.IsEmpty()) {
                    tokens.push_back({ currentToken, false });
                    currentToken.Empty();
                }
                inWord = true;
                currentToken += ch;
            }
            else {
                if (inWord && !currentToken.IsEmpty()) {
                    tokens.push_back({ currentToken, true });
                    currentToken.Empty();
                }
                inWord = false;
                currentToken += ch;
            }
        }
        if (!currentToken.IsEmpty()) tokens.push_back({ currentToken, inWord });

        int firstWordIdx = -1, lastWordIdx = -1;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i].isWord) {
                if (firstWordIdx == -1) firstWordIdx = (int)i;
                lastWordIdx = (int)i;
            }
        }

        CStringW result;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (!tokens[i].isWord) { result += tokens[i].text; continue; }

            CStringW word = tokens[i].text;

            bool followsMajorPunctuation = false;
            if (i > 0) {
                CStringW prevSep = tokens[i - 1].text;
                if (prevSep.FindOneOf(L":;./([{\"\u201C'\u2018") != -1) {
                    followsMajorPunctuation = true;
                }
                // [FIX] дефис считается major только если вокруг пробелы
                if (prevSep.Find(L'-') != -1 && prevSep.Find(L' ') != -1) {
                    followsMajorPunctuation = true;
                }
            }

            bool isSmallWord = (mode == 1) && (smallWords.find(word) != smallWords.end());
            bool isMustCapitalize = ((int)i == firstWordIdx) ||
                ((int)i == lastWordIdx) ||
                followsMajorPunctuation;

            if (isSmallWord && !isMustCapitalize) {
                word.MakeLower();
            }
            else {
                word.MakeLower();
                if (word.GetLength() > 0) word.SetAt(0, SafeToUpper(word[0]));
            }
            result += word;
        }
        wName = result;
    }

    // Пост-обработка: римские цифры (кроме lowercase)
    if (mode != 4) {
        int len = wName.GetLength();
        int start = -1;
        for (int i = 0; i <= len; ++i) {
            wchar_t c = (i < len) ? wName[i] : L' ';
            if (SafeIsAlpha(c)) {
                if (start == -1) start = i;
            }
            else {
                if (start != -1) {
                    int wordLen = i - start;
                    CString word = wName.Mid(start, wordLen);
                    if (IsRomanNumeral(word)) {
                        word.MakeUpper();
                        for (int j = 0; j < wordLen; ++j)
                            wName.SetAt(start + j, word[j]);
                    }
                    start = -1;
                }
            }
        }
    }
    name = pfc::stringcvt::string_utf8_from_os(wName).get_ptr();
}

// =========================================================================
//  DIALOG
// =========================================================================

BOOL CEditorDialog::OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {
    m_list.Attach(GetDlgItem(IDC_LST_PREVIEW));
    m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_list.InsertColumn(0, _T("Original Name"), LVCFMT_LEFT, cfg_col_width_0.get_value());
    m_list.InsertColumn(1, _T("New Name"), LVCFMT_LEFT, cfg_col_width_1.get_value());
    m_list.InsertColumn(2, _T("Status"), LVCFMT_LEFT, cfg_col_width_2.get_value());

    for (t_size i = 0; i < m_items.get_count(); ++i) {
        const char* path = m_items[i]->get_path();
        pfc::string8 filename = pfc::string_filename_ext(path);
        m_list.InsertItem((int)i, pfc::stringcvt::string_os_from_utf8(filename));
        m_list.SetItemText((int)i, 1, _T("Waiting..."));
        m_list.SetItemText((int)i, 2, _T("Ready"));
    }

    CheckDlgButton(IDC_CHK_TAGS, cfg_use_tags.get_value() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_CHK_TRANSLIT, cfg_use_translit.get_value() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_CHK_CASE, cfg_use_case.get_value() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_CHK_SAVE_M3U, cfg_use_m3u.get_value() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_CHK_M3U_EXTINF, cfg_use_extinf.get_value() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_CHK_M3U_OVERWRITE, cfg_use_overwrite.get_value() ? BST_CHECKED : BST_UNCHECKED);

    CComboBox cbCase(GetDlgItem(IDC_CMB_CASE_MODE));
    cbCase.AddString(_T("Title Case"));
    cbCase.AddString(_T("Smart Title Case"));
    cbCase.AddString(_T("Sentence case"));
    cbCase.AddString(_T("ALL CAPS"));
    cbCase.AddString(_T("lowercase"));
    cbCase.SetCurSel(cfg_default_case.get_value());

    // --- Заполнение паттернов тегов ---
    CComboBox cbPattern(GetDlgItem(IDC_CMB_TAG_PATTERN));
    pfc::string8 history = cfg_pattern_history.get();
    if (history.is_empty()) {
        cbPattern.AddString(_T("%tracknumber% - %title%"));
        cbPattern.AddString(_T("%artist% - %title%"));
    }
    else {
        pfc::string8 current_token;
        for (t_size i = 0; i < history.length(); ++i) {
            if (history[i] == '|') {
                if (!current_token.is_empty())
                    cbPattern.AddString(pfc::stringcvt::string_os_from_utf8(current_token).get_ptr());
                current_token.reset();
            }
            else current_token.add_char(history[i]);
        }
        if (!current_token.is_empty())
            cbPattern.AddString(pfc::stringcvt::string_os_from_utf8(current_token).get_ptr());
    }

    // --- Заполнение паттернов M3U ---
    CComboBox cbM3U(GetDlgItem(IDC_CMB_M3U_TEMPLATE));
    pfc::string8 historyM3U = cfg_m3u_history.get();
    if (historyM3U.is_empty()) {
        cbM3U.AddString(_T("%album%"));
        cbM3U.AddString(_T("%artist% - %album%"));
    }
    else {
        pfc::string8 current_token;
        for (t_size i = 0; i < historyM3U.length(); ++i) {
            if (historyM3U[i] == '|') {
                if (!current_token.is_empty())
                    cbM3U.AddString(pfc::stringcvt::string_os_from_utf8(current_token).get_ptr());
                current_token.reset();
            }
            else current_token.add_char(historyM3U[i]);
        }
        if (!current_token.is_empty())
            cbM3U.AddString(pfc::stringcvt::string_os_from_utf8(current_token).get_ptr());
    }
    CString wM3UInitial = pfc::stringcvt::string_os_from_utf8(cfg_m3u_default.get()).get_ptr();
    cbM3U.SetWindowText(wM3UInitial);

    // --- Определение VA/Single ---
    bool is_va = false;
    if (m_items.get_count() > 0) {
        titleformat_object::ptr script_aa;
        titleformat_compiler::get()->compile_safe(script_aa, "$lower(%album artist%)");
        pfc::string8 album_artist;
        m_items[0]->format_title(nullptr, album_artist, script_aa, nullptr);

        if (strstr(album_artist.get_ptr(), "various") != nullptr ||
            strstr(album_artist.get_ptr(), "va") != nullptr) {
            is_va = true;
        }
        else if (m_items.get_count() > 1) {
            titleformat_object::ptr script_a;
            titleformat_compiler::get()->compile_safe(script_a, "%artist%");
            pfc::string8 first_artist;
            m_items[0]->format_title(nullptr, first_artist, script_a, nullptr);
            for (t_size i = 1; i < m_items.get_count(); ++i) {
                pfc::string8 next_artist;
                m_items[i]->format_title(nullptr, next_artist, script_a, nullptr);
                if (first_artist != next_artist) { is_va = true; break; }
            }
        }
    }

    pfc::string8 initial_pattern = is_va ? cfg_default_va.get() : cfg_default_single.get();
    CString wInitial = pfc::stringcvt::string_os_from_utf8(initial_pattern).get_ptr();
    cbPattern.SetWindowText(wInitial);

    // [FIX] Не сохраняем историю автоматически — только добавляем в UI
    if (cbPattern.FindStringExact(-1, wInitial) == CB_ERR) {
        cbPattern.AddString(wInitial);
    }

    COMBOBOXINFO cbi = { sizeof(COMBOBOXINFO) };
    ::GetComboBoxInfo(GetDlgItem(IDC_CMB_TAG_PATTERN), &cbi);
    m_edit.SubclassWindow(cbi.hwndItem);

    COMBOBOXINFO cbi2 = { sizeof(COMBOBOXINFO) };
    ::GetComboBoxInfo(GetDlgItem(IDC_CMB_M3U_TEMPLATE), &cbi2);
    m_edit_m3u.SubclassWindow(cbi2.hwndItem);

    UpdatePreview();
    return TRUE;
}

void CEditorDialog::OnDestroy() {
    cfg_col_width_0 = m_list.GetColumnWidth(0);
    cfg_col_width_1 = m_list.GetColumnWidth(1);
    cfg_col_width_2 = m_list.GetColumnWidth(2);

    cfg_use_tags = (IsDlgButtonChecked(IDC_CHK_TAGS) == BST_CHECKED) ? 1 : 0;
    cfg_use_translit = (IsDlgButtonChecked(IDC_CHK_TRANSLIT) == BST_CHECKED) ? 1 : 0;
    cfg_use_case = (IsDlgButtonChecked(IDC_CHK_CASE) == BST_CHECKED) ? 1 : 0;
    cfg_use_m3u = (IsDlgButtonChecked(IDC_CHK_SAVE_M3U) == BST_CHECKED) ? 1 : 0;
    cfg_use_extinf = (IsDlgButtonChecked(IDC_CHK_M3U_EXTINF) == BST_CHECKED) ? 1 : 0;
    cfg_use_overwrite = (IsDlgButtonChecked(IDC_CHK_M3U_OVERWRITE) == BST_CHECKED) ? 1 : 0;

    // [NEW] Сохраняем историю при закрытии — все добавления/удаления фиксируются
    SaveHistory();
    SaveM3UHistory();

    cfg_m3u_default = pfc::stringcvt::string_utf8_from_os(
        GetDlgItemTextSafe(m_hWnd, IDC_CMB_M3U_TEMPLATE)).get_ptr();

    SetMsgHandled(FALSE);
}

void CEditorDialog::OnClose() {
    // [NEW] Крестик — то же, что Cancel
    EndDialog(IDCANCEL);
}

LRESULT CEditorDialog::OnEditKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    m_dwLastSel = (DWORD)m_edit.SendMessage(EM_GETSEL, 0, 0);
    SetMsgHandled(FALSE);
    return 0;
}

LRESULT CEditorDialog::OnEditM3UKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    m_dwLastSelM3U = (DWORD)m_edit_m3u.SendMessage(EM_GETSEL, 0, 0);
    SetMsgHandled(FALSE);
    return 0;
}

void CEditorDialog::SaveHistory() {
    CComboBox cb(GetDlgItem(IDC_CMB_TAG_PATTERN));
    pfc::string8 new_history;
    for (int i = 0; i < cb.GetCount(); ++i) {
        int len = cb.GetLBTextLen(i);
        if (len != CB_ERR) {
            CString text;
            cb.GetLBText(i, text.GetBuffer(len + 1)); text.ReleaseBuffer(len);
            new_history << pfc::stringcvt::string_utf8_from_os(text).get_ptr();
            if (i < cb.GetCount() - 1) new_history << "|";
        }
    }
    cfg_pattern_history = new_history;
}

void CEditorDialog::SaveM3UHistory() {
    CComboBox cb(GetDlgItem(IDC_CMB_M3U_TEMPLATE));
    pfc::string8 new_history;
    for (int i = 0; i < cb.GetCount(); ++i) {
        int len = cb.GetLBTextLen(i);
        if (len != CB_ERR) {
            CString text;
            cb.GetLBText(i, text.GetBuffer(len + 1)); text.ReleaseBuffer(len);
            new_history << pfc::stringcvt::string_utf8_from_os(text).get_ptr();
            if (i < cb.GetCount() - 1) new_history << "|";
        }
    }
    cfg_m3u_history = new_history;
}

CString CEditorDialog::GetDynamicPattern() {
    // [FIX] Безопасное чтение без магической 256
    CString patternStr = GetDlgItemTextSafe(m_hWnd, IDC_CMB_TAG_PATTERN);

    if (patternStr.Find(L"%tracknumber%") != -1 && patternStr.Find(L"$num(") == -1) {
        int max_digits = 2;
        if (m_items.get_count() > 0) {
            titleformat_object::ptr script_tn;
            titleformat_compiler::get()->compile_safe(script_tn, "%tracknumber%");
            for (t_size i = 0; i < m_items.get_count(); ++i) {
                pfc::string8 tn;
                m_items[i]->format_title(nullptr, tn, script_tn, nullptr);
                int current_len = 0;
                for (t_size j = 0; j < tn.length(); ++j) {
                    if (tn[j] >= '0' && tn[j] <= '9') current_len++;
                }
                if (current_len > max_digits) max_digits = current_len;
            }
        }
        CString paddedTn;
        paddedTn.Format(L"$num(%%tracknumber%%,%d)", max_digits);
        patternStr.Replace(L"%tracknumber%", paddedTn);
    }
    return patternStr;
}

// -------------------------------------------------------------------------
//  ПЕРЕИМЕНОВАНИЕ
// -------------------------------------------------------------------------

void CEditorDialog::OnRename(UINT uNotifyCode, int nID, CWindow wndCtl) {
    bool useTags = IsDlgButtonChecked(IDC_CHK_TAGS) == BST_CHECKED;
    bool useTranslit = IsDlgButtonChecked(IDC_CHK_TRANSLIT) == BST_CHECKED;
    bool useCase = IsDlgButtonChecked(IDC_CHK_CASE) == BST_CHECKED;
    bool saveM3U = IsDlgButtonChecked(IDC_CHK_SAVE_M3U) == BST_CHECKED;
    bool m3uExtinf = IsDlgButtonChecked(IDC_CHK_M3U_EXTINF) == BST_CHECKED;
    bool m3uOverwrite = IsDlgButtonChecked(IDC_CHK_M3U_OVERWRITE) == BST_CHECKED;

    std::vector<std::unique_ptr<INameProcessor>> pipeline;

    if (useTags) {
        CString patternStr = GetDynamicPattern();
        pfc::string8 patternUtf8 = pfc::stringcvt::string_utf8_from_os(patternStr).get_ptr();
        pipeline.push_back(std::make_unique<TagRenamerProcessor>(
            patternUtf8.is_empty() ? "%title%" : patternUtf8.get_ptr()));
    }

    if (useTranslit) {
        pipeline.push_back(std::make_unique<LigatureProcessor>());
        CString repStr = GetDlgItemTextSafe(m_hWnd, IDC_EDT_REPLACE_CHAR);
        pipeline.push_back(std::make_unique<TransliteratorProcessor>(
            pfc::stringcvt::string_utf8_from_os(repStr).get_ptr()));
    }

    int successCount = 0;
    std::vector<CString> failedFiles;
    std::set<CString, CStringLess> used_paths;

    metadb_handle_list old_handles;
    metadb_handle_list new_handles;

    for (t_size i = 0; i < m_items.get_count(); ++i) {
        // --- Получаем старый путь ---
        pfc::string8 old_path_raw;
        filesystem::g_get_display_path(m_items[i]->get_path(), old_path_raw);
        if (strncmp(old_path_raw.get_ptr(), "file://", 7) == 0)
            old_path_raw = old_path_raw.get_ptr() + 7;

        CString wOldPath = pfc::stringcvt::string_os_from_utf8(old_path_raw).get_ptr();
        wOldPath.Replace(L'/', L'\\');
        pfc::string8 old_path = pfc::stringcvt::string_utf8_from_os(wOldPath).get_ptr();

        pfc::string8 directory = pfc::string_directory(old_path);
        pfc::string8 ext = pfc::string_extension(old_path);

        // --- Формируем новое имя ---
        pfc::string8 new_name = useTags ? pfc::string8("") : pfc::string_filename(old_path);
        for (auto& processor : pipeline) processor->Process(new_name, m_items[i]);

        if (useCase) {
            CComboBox cbCase(GetDlgItem(IDC_CMB_CASE_MODE));
            ApplyCaseConversion(new_name, cbCase.GetCurSel());
        }
        SanitizeFilename(new_name);

        CString wDir = pfc::stringcvt::string_os_from_utf8(directory).get_ptr();
        CString wBaseName = pfc::stringcvt::string_os_from_utf8(new_name).get_ptr();
        CString wExt = pfc::stringcvt::string_os_from_utf8(ext).get_ptr();

        // --- Разрешение коллизий ---
        CString candidatePath;
        CString candidateBase = wBaseName;
        int counter = 1;
        while (true) {
            candidatePath = wDir + L"\\" + candidateBase;
            if (!wExt.IsEmpty()) candidatePath += L"." + wExt;
            if (used_paths.find(candidatePath) == used_paths.end()) {
                used_paths.insert(candidatePath);
                break;
            }
            candidateBase.Format(L"%s (%d)", wBaseName, counter++);
        }

        pfc::string8 final_name = pfc::stringcvt::string_utf8_from_os(candidateBase).get_ptr();
        pfc::string8 new_path = directory;
        new_path << "\\" << final_name;
        if (!ext.is_empty()) new_path << "." << ext;

        CString wOld = pfc::stringcvt::string_os_from_utf8(old_path).get_ptr();
        CString wNew = pfc::stringcvt::string_os_from_utf8(new_path).get_ptr();

        // --- Реальное имя на диске (регистр) ---
        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileW(wOld, &fd);
        CString wRealDiskName = (hFind != INVALID_HANDLE_VALUE)
            ? fd.cFileName
            : pfc::stringcvt::string_os_from_utf8(pfc::string_filename_ext(old_path)).get_ptr();
        if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);

        CString wNewFileName = pfc::stringcvt::string_os_from_utf8(
            pfc::string_filename_ext(new_path)).get_ptr();

        if (wOld == wNew && wRealDiskName.Compare(wNewFileName) == 0) {
            old_handles.add_item(m_items[i]);
            new_handles.add_item(m_items[i]);
            successCount++;
            continue;
        }

        // --- MoveFile ---
        bool move_success = false;
        if (wOld.CompareNoCase(wNew) == 0) {
            // Смена только регистра — через временный файл
            CString wTmp = wNew + L".__ren_tmp__";
            if (MoveFileW(wOld, wTmp) != 0) {
                if (MoveFileW(wTmp, wNew) != 0) move_success = true;
                else MoveFileW(wTmp, wOld);
            }
        }
        else {
            if (MoveFileW(wOld, wNew) != 0) move_success = true;
        }

        if (move_success) {
            successCount++;
            old_handles.add_item(m_items[i]);
            pfc::string8 proto_path = "file://"; proto_path << new_path;
            playable_location_impl location(proto_path.get_ptr(), m_items[i]->get_subsong_index());
            metadb_handle_ptr new_handle;
            metadb::get()->handle_create(new_handle, location);
            new_handles.add_item(new_handle);
        }
        else {
            failedFiles.push_back(pfc::stringcvt::string_os_from_utf8(
                pfc::string_filename_ext(old_path)).get_ptr());
        }
    }

    // --- Обновляем плейлисты ---
    if (successCount > 0) {
        static_api_ptr_t<playlist_manager> pm;
        t_size p_count = pm->get_playlist_count();
        for (t_size p = 0; p < p_count; ++p) {
            if ((pm->playlist_lock_get_filter_mask(p) & playlist_lock::filter_replace) != 0)
                continue;
            t_size item_count = pm->playlist_get_item_count(p);
            bool undo_created = false;
            for (t_size item_idx = 0; item_idx < item_count; ++item_idx) {
                metadb_handle_ptr current_item;
                pm->playlist_get_item_handle(current_item, p, item_idx);
                t_size found_idx = old_handles.find_item(current_item);
                if (found_idx != pfc_infinite) {
                    if (!undo_created) { pm->playlist_undo_backup(p); undo_created = true; }
                    pm->playlist_replace_item(p, item_idx, new_handles[found_idx]);
                }
            }
        }

        if (new_handles.get_count() > 0) {
            static_api_ptr_t<metadb_io_v2> io;
            io->load_info_async(new_handles, metadb_io::load_info_default,
                m_hWnd, metadb_io_v2::op_flag_background, nullptr);
        }

        // --- M3U ---
        if (saveM3U && new_handles.get_count() > 0) {
            // Общий предок
            pfc::string8 first_path_utf8;
            filesystem::g_get_display_path(new_handles[0]->get_path(), first_path_utf8);
            if (strncmp(first_path_utf8.get_ptr(), "file://", 7) == 0)
                first_path_utf8 = first_path_utf8.get_ptr() + 7;

            CString wFirstPath = pfc::stringcvt::string_os_from_utf8(first_path_utf8).get_ptr();
            wFirstPath.Replace(L'/', L'\\');
            first_path_utf8 = pfc::stringcvt::string_utf8_from_os(wFirstPath).get_ptr();

            CString wCommon = pfc::stringcvt::string_os_from_utf8(
                pfc::string_directory(first_path_utf8)).get_ptr();

            for (t_size i = 1; i < new_handles.get_count(); ++i) {
                pfc::string8 item_path_utf8;
                filesystem::g_get_display_path(new_handles[i]->get_path(), item_path_utf8);
                if (strncmp(item_path_utf8.get_ptr(), "file://", 7) == 0)
                    item_path_utf8 = item_path_utf8.get_ptr() + 7;

                CString wItemPath = pfc::stringcvt::string_os_from_utf8(item_path_utf8).get_ptr();
                wItemPath.Replace(L'/', L'\\');
                item_path_utf8 = pfc::stringcvt::string_utf8_from_os(wItemPath).get_ptr();

                CString wDir = pfc::stringcvt::string_os_from_utf8(
                    pfc::string_directory(item_path_utf8)).get_ptr();
                while (!wCommon.IsEmpty() &&
                    wDir.Left(wCommon.GetLength()).CompareNoCase(wCommon) != 0) {
                    int slash = wCommon.ReverseFind(L'\\');
                    if (slash == -1) { wCommon.Empty(); break; }
                    wCommon = wCommon.Left(slash);
                }
            }

            CString m3uPatternStr = GetDlgItemTextSafe(m_hWnd, IDC_CMB_M3U_TEMPLATE);
            pfc::string8 m3uPattern = pfc::stringcvt::string_utf8_from_os(m3uPatternStr).get_ptr();
            if (m3uPattern.is_empty()) m3uPattern = "%album%";

            titleformat_object::ptr script_m3u;
            titleformat_compiler::get()->compile_safe(script_m3u, m3uPattern);
            pfc::string8 m3uFilename;
            m_items[0]->format_title(nullptr, m3uFilename, script_m3u, nullptr);
            SanitizeFilename(m3uFilename);
            if (m3uFilename.is_empty()) m3uFilename = "Playlist";

            CString m3uPath = wCommon + L"\\" +
                pfc::stringcvt::string_os_from_utf8(m3uFilename).get_ptr();

            if (!m3uOverwrite) {
                CString candidateBase = m3uPath;
                int counter = 1;
                while (GetFileAttributesW(m3uPath + L".m3u") != INVALID_FILE_ATTRIBUTES) {
                    m3uPath.Format(L"%s (%d)", candidateBase, counter++);
                }
            }
            m3uPath += L".m3u";

            // [FIX] Проверяем errno_t, а не errno
            FILE* f = nullptr;
            errno_t err = _wfopen_s(&f, m3uPath, L"wb");
            if (err == 0 && f != nullptr) {
                const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
                fwrite(bom, 1, 3, f);

                if (m3uExtinf) fwrite("#EXTM3U\r\n", 1, 9, f);

                titleformat_object::ptr script_extinf;
                titleformat_compiler::get()->compile_safe(script_extinf, "%artist% - %title%");

                for (t_size i = 0; i < new_handles.get_count(); ++i) {
                    if (m3uExtinf) {
                        double len = m_items[i]->get_length();
                        int seconds = (len > 0) ? (int)(len + 0.5) : -1;
                        pfc::string8 extinf_text;
                        m_items[i]->format_title(nullptr, extinf_text, script_extinf, nullptr);
                        CStringA line;
                        line.Format("#EXTINF:%d,%s\r\n", seconds, extinf_text.get_ptr());
                        fwrite(line.GetString(), 1, line.GetLength(), f);
                    }

                    pfc::string8 final_item_path;
                    filesystem::g_get_display_path(new_handles[i]->get_path(), final_item_path);
                    if (strncmp(final_item_path.get_ptr(), "file://", 7) == 0)
                        final_item_path = final_item_path.get_ptr() + 7;

                    CString wFull = pfc::stringcvt::string_os_from_utf8(final_item_path).get_ptr();
                    wFull.Replace(L'/', L'\\');
                    CString wRel = wFull.Mid(wCommon.GetLength());
                    if (wRel.GetLength() > 0 && wRel[0] == L'\\') wRel = wRel.Mid(1);

                    CStringA relUtf8 = pfc::stringcvt::string_utf8_from_os(wRel).get_ptr();
                    fwrite(relUtf8.GetString(), 1, relUtf8.GetLength(), f);
                    fwrite("\r\n", 1, 2, f);
                }
                fclose(f);
            }
            else {
                CString errMsg;
                errMsg.Format(L"Failed to create playlist file.\nerrno = %d\npath: %s",
                    (int)err, m3uPath.GetString());
                ::MessageBoxW(m_hWnd, errMsg, L"M3U Error", MB_OK | MB_ICONERROR);
            }
        }
    }

    // --- Отчёт о неудачах ---
    if (!failedFiles.empty()) {
        CString msg;
        msg.Format(_T("Successfully processed: %d files.\n\nFailed: %zu files:\n"),
            successCount, failedFiles.size());
        size_t displayCount = failedFiles.size() > 10 ? 10 : failedFiles.size();
        for (size_t i = 0; i < displayCount; ++i)
            msg += _T("- ") + failedFiles[i] + _T("\n");
        ::MessageBoxW(m_hWnd, msg, L"Partial Success / Error", MB_OK | MB_ICONWARNING);
    }

    EndDialog(IDOK);
}

void CEditorDialog::OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl) {
    EndDialog(IDCANCEL);
}

void CEditorDialog::OnConfigChanged(UINT uNotifyCode, int nID, CWindow wndCtl) {
    UpdatePreview();
}

// -------------------------------------------------------------------------
//  ПРЕДПРОСМОТР
// -------------------------------------------------------------------------

void CEditorDialog::UpdatePreview() {
    bool useTags = IsDlgButtonChecked(IDC_CHK_TAGS) == BST_CHECKED;
    bool useTranslit = IsDlgButtonChecked(IDC_CHK_TRANSLIT) == BST_CHECKED;
    bool useCase = IsDlgButtonChecked(IDC_CHK_CASE) == BST_CHECKED;

    std::vector<std::unique_ptr<INameProcessor>> pipeline;

    if (useTags) {
        CString patternStr = GetDynamicPattern();
        pfc::string8 patternUtf8 = pfc::stringcvt::string_utf8_from_os(patternStr).get_ptr();
        pipeline.push_back(std::make_unique<TagRenamerProcessor>(
            patternUtf8.is_empty() ? "%title%" : patternUtf8.get_ptr()));
    }
    if (useTranslit) {
        pipeline.push_back(std::make_unique<LigatureProcessor>());
        CString repStr = GetDlgItemTextSafe(m_hWnd, IDC_EDT_REPLACE_CHAR);
        pipeline.push_back(std::make_unique<TransliteratorProcessor>(
            pfc::stringcvt::string_utf8_from_os(repStr).get_ptr()));
    }

    std::set<CString, CStringLess> used_paths;

    for (t_size i = 0; i < m_items.get_count(); ++i) {
        pfc::string8 full_path_raw;
        filesystem::g_get_display_path(m_items[i]->get_path(), full_path_raw);
        if (strncmp(full_path_raw.get_ptr(), "file://", 7) == 0)
            full_path_raw = full_path_raw.get_ptr() + 7;

        CString wFullPath = pfc::stringcvt::string_os_from_utf8(full_path_raw).get_ptr();
        wFullPath.Replace(L'/', L'\\');
        pfc::string8 full_path = pfc::stringcvt::string_utf8_from_os(wFullPath).get_ptr();

        pfc::string8 directory = pfc::string_directory(full_path);
        pfc::string8 ext = pfc::string_extension(full_path);
        pfc::string8 new_name = useTags ? pfc::string8("") : pfc::string_filename(full_path);

        for (auto& processor : pipeline) processor->Process(new_name, m_items[i]);

        if (useCase) {
            CComboBox cbCase(GetDlgItem(IDC_CMB_CASE_MODE));
            ApplyCaseConversion(new_name, cbCase.GetCurSel());
        }
        SanitizeFilename(new_name);

        CString wDir = pfc::stringcvt::string_os_from_utf8(directory).get_ptr();
        CString wBaseName = pfc::stringcvt::string_os_from_utf8(new_name).get_ptr();
        CString wExt = pfc::stringcvt::string_os_from_utf8(ext).get_ptr();

        CString candidatePath;
        CString candidateBase = wBaseName;
        int counter = 1;
        while (true) {
            candidatePath = wDir + L"\\" + candidateBase;
            if (!wExt.IsEmpty()) candidatePath += L"." + wExt;
            if (used_paths.find(candidatePath) == used_paths.end()) {
                used_paths.insert(candidatePath);
                break;
            }
            candidateBase.Format(L"%s (%d)", wBaseName, counter++);
        }

        pfc::string8 final_name = pfc::stringcvt::string_utf8_from_os(candidateBase).get_ptr();
        pfc::string8 final_path_with_ext = final_name;
        if (!ext.is_empty()) final_path_with_ext << "." << ext;

        // [NEW] Статус в колонке 2
        CString wOriginal = pfc::stringcvt::string_os_from_utf8(
            pfc::string_filename_ext(full_path)).get_ptr();
        CString wFinal = pfc::stringcvt::string_os_from_utf8(final_path_with_ext).get_ptr();

        m_list.SetItemText((int)i, 1, wFinal);
        if (new_name.is_empty()) {
            m_list.SetItemText((int)i, 2, _T("Empty name!"));
        }
        else if (wOriginal.Compare(wFinal) == 0) {
            m_list.SetItemText((int)i, 2, _T("Unchanged"));
        }
        else if (wOriginal.CompareNoCase(wFinal) == 0) {
            m_list.SetItemText((int)i, 2, _T("Case change"));
        }
        else {
            m_list.SetItemText((int)i, 2, _T("OK"));
        }
    }
}

// -------------------------------------------------------------------------
//  ОБРАБОТЧИКИ UI
// -------------------------------------------------------------------------

void CEditorDialog::OnPatternSelect(UINT uNotifyCode, int nID, CWindow wndCtl) {
    CComboBox cb(wndCtl);
    int sel = cb.GetCurSel();
    if (sel >= 0) {
        int len = cb.GetLBTextLen(sel);
        if (len != CB_ERR) {
            CString text; cb.GetLBText(sel, text.GetBuffer(len + 1)); text.ReleaseBuffer(len);
            cb.SetWindowText(text);
            UpdatePreview();
        }
    }
}

void CEditorDialog::OnTagMenu(UINT uNotifyCode, int nID, CWindow wndCtl) {
    CMenu menu; menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING, 1, _T("%artist%"));
    menu.AppendMenu(MF_STRING, 2, _T("%album artist%"));
    menu.AppendMenu(MF_STRING, 3, _T("%album%"));
    menu.AppendMenu(MF_STRING, 4, _T("%tracknumber%"));
    menu.AppendMenu(MF_STRING, 5, _T("%title%"));
    menu.AppendMenu(MF_STRING, 6, _T("%date%"));
    menu.AppendMenu(MF_STRING, 7, _T("%year%"));
    menu.AppendMenu(MF_STRING, 8, _T("%genre%"));
    menu.AppendMenu(MF_STRING, 9, _T("%comment%"));
    menu.AppendMenu(MF_STRING, 10, _T("%composer%"));
    menu.AppendMenu(MF_SEPARATOR);
    menu.AppendMenu(MF_STRING, 11, _T("'('"));
    menu.AppendMenu(MF_STRING, 12, _T("')'"));
    menu.AppendMenu(MF_STRING, 13, _T("'['"));
    menu.AppendMenu(MF_STRING, 14, _T("']'"));
    menu.AppendMenu(MF_STRING, 15, _T("''"));
    menu.AppendMenu(MF_STRING, 16, _T("''''"));

    RECT rect; ::GetWindowRect(GetDlgItem(IDC_BTN_TAG_MENU), &rect);
    int cmd = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTALIGN,
        rect.left, rect.bottom, m_hWnd);
    if (cmd > 0) {
        CString tag;
        menu.GetMenuString(cmd, tag.GetBuffer(256), 256, MF_BYCOMMAND);
        tag.ReleaseBuffer();
        ::SetFocus(m_edit.m_hWnd);
        ::SendMessage(m_edit.m_hWnd, EM_SETSEL,
            LOWORD(m_dwLastSel), HIWORD(m_dwLastSel));
        ::SendMessage(m_edit.m_hWnd, EM_REPLACESEL, TRUE, (LPARAM)(LPCTSTR)tag);
        UpdatePreview();
    }
}

void CEditorDialog::OnSavePattern(UINT uNotifyCode, int nID, CWindow wndCtl) {
    CString currentText = GetDlgItemTextSafe(m_hWnd, IDC_CMB_TAG_PATTERN);
    if (!currentText.IsEmpty()) {
        CComboBox cb(GetDlgItem(IDC_CMB_TAG_PATTERN));
        if (cb.FindStringExact(-1, currentText) == CB_ERR) {
            cb.AddString(currentText);
            SaveHistory();
        }
    }
}

void CEditorDialog::OnDelPattern(UINT uNotifyCode, int nID, CWindow wndCtl) {
    CComboBox cb(GetDlgItem(IDC_CMB_TAG_PATTERN));
    CString currentText = GetDlgItemTextSafe(m_hWnd, IDC_CMB_TAG_PATTERN);
    int index = cb.FindStringExact(-1, currentText);
    if (index != CB_ERR) {
        cb.DeleteString(index);
        cb.SetWindowText(_T(""));
        SaveHistory();
        UpdatePreview();
    }
}

void CEditorDialog::OnPresetSingle(UINT uNotifyCode, int nID, CWindow wndCtl) {
    CString currentText = GetDlgItemTextSafe(m_hWnd, IDC_CMB_TAG_PATTERN);
    if (!currentText.IsEmpty()) {
        cfg_default_single = pfc::stringcvt::string_utf8_from_os(currentText).get_ptr();
        MessageBox(_T("Current pattern saved as default for Single albums."),
            _T("Saved"), MB_OK | MB_ICONINFORMATION);
    }
}

void CEditorDialog::OnPresetVA(UINT uNotifyCode, int nID, CWindow wndCtl) {
    CString currentText = GetDlgItemTextSafe(m_hWnd, IDC_CMB_TAG_PATTERN);
    if (!currentText.IsEmpty()) {
        cfg_default_va = pfc::stringcvt::string_utf8_from_os(currentText).get_ptr();
        MessageBox(_T("Current pattern saved as default for VA albums."),
            _T("Saved"), MB_OK | MB_ICONINFORMATION);
    }
}

void CEditorDialog::OnSetCaseDefault(UINT uNotifyCode, int nID, CWindow wndCtl) {
    CComboBox cbCase(GetDlgItem(IDC_CMB_CASE_MODE));
    int sel = cbCase.GetCurSel();
    if (sel >= 0) {
        cfg_default_case = sel;
        MessageBox(_T("Current case mode saved as default."),
            _T("Saved"), MB_OK | MB_ICONINFORMATION);
    }
}

void CEditorDialog::OnM3UPatternSelect(UINT uNotifyCode, int nID, CWindow wndCtl) {
    CComboBox cb(wndCtl);
    int sel = cb.GetCurSel();
    if (sel >= 0) {
        int len = cb.GetLBTextLen(sel);
        if (len != CB_ERR) {
            CString text; cb.GetLBText(sel, text.GetBuffer(len + 1)); text.ReleaseBuffer(len);
            cb.SetWindowText(text);
        }
    }
}

void CEditorDialog::OnM3UTagMenu(UINT uNotifyCode, int nID, CWindow wndCtl) {
    CMenu menu; menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING, 1, _T("%artist%"));
    menu.AppendMenu(MF_STRING, 2, _T("%album artist%"));
    menu.AppendMenu(MF_STRING, 3, _T("%album%"));
    menu.AppendMenu(MF_STRING, 6, _T("%date%"));
    menu.AppendMenu(MF_SEPARATOR);
    menu.AppendMenu(MF_STRING, 11, _T("'('"));
    menu.AppendMenu(MF_STRING, 12, _T("')'"));
    menu.AppendMenu(MF_STRING, 13, _T("'['"));
    menu.AppendMenu(MF_STRING, 14, _T("']'"));

    RECT rect; ::GetWindowRect(GetDlgItem(IDC_BTN_M3U_TAGS), &rect);
    int cmd = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTALIGN,
        rect.left, rect.bottom, m_hWnd);
    if (cmd > 0) {
        CString tag;
        menu.GetMenuString(cmd, tag.GetBuffer(256), 256, MF_BYCOMMAND);
        tag.ReleaseBuffer();
        ::SetFocus(m_edit_m3u.m_hWnd);
        ::SendMessage(m_edit_m3u.m_hWnd, EM_SETSEL,
            LOWORD(m_dwLastSelM3U), HIWORD(m_dwLastSelM3U));
        ::SendMessage(m_edit_m3u.m_hWnd, EM_REPLACESEL, TRUE, (LPARAM)(LPCTSTR)tag);
    }
}

void CEditorDialog::OnSaveM3UPattern(UINT uNotifyCode, int nID, CWindow wndCtl) {
    CString currentText = GetDlgItemTextSafe(m_hWnd, IDC_CMB_M3U_TEMPLATE);
    if (!currentText.IsEmpty()) {
        CComboBox cb(GetDlgItem(IDC_CMB_M3U_TEMPLATE));
        if (cb.FindStringExact(-1, currentText) == CB_ERR) {
            cb.AddString(currentText);
            SaveM3UHistory();
        }
    }
}

void CEditorDialog::OnDelM3UPattern(UINT uNotifyCode, int nID, CWindow wndCtl) {
    CComboBox cb(GetDlgItem(IDC_CMB_M3U_TEMPLATE));
    CString currentText = GetDlgItemTextSafe(m_hWnd, IDC_CMB_M3U_TEMPLATE);
    int index = cb.FindStringExact(-1, currentText);
    if (index != CB_ERR) {
        cb.DeleteString(index);
        cb.SetWindowText(_T(""));
        SaveM3UHistory();
    }
}