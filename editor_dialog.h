#pragma once
#include <foobar2000/SDK/foobar2000.h>
#include "resource.h"
#include "processor.h"

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlctrls.h>
#include <atlcrack.h>
#include <atlstr.h>

class CEditorDialog : public CDialogImpl<CEditorDialog> {
public:
    enum { IDD = IDD_EDITOR_DIALOG };
    metadb_handle_list m_items;

    CContainedWindow m_edit;
    CContainedWindow m_edit_m3u; // Для поля ввода M3U
    DWORD m_dwLastSel;
    DWORD m_dwLastSelM3U; // Для курсора M3U

    CEditorDialog(metadb_handle_list_cref items) : m_items(items), m_edit(this, 1), m_edit_m3u(this, 2), m_dwLastSel(0), m_dwLastSelM3U(0) {}

    BEGIN_MSG_MAP_EX(CEditorDialog)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_DESTROY(OnDestroy)

        COMMAND_ID_HANDLER_EX(IDOK, OnRename)
        COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)

        COMMAND_ID_HANDLER_EX(IDC_CHK_TAGS, OnConfigChanged)
        COMMAND_ID_HANDLER_EX(IDC_CHK_TRANSLIT, OnConfigChanged)
        COMMAND_ID_HANDLER_EX(IDC_CHK_CASE, OnConfigChanged)

        COMMAND_HANDLER_EX(IDC_CMB_CASE_MODE, CBN_SELCHANGE, OnConfigChanged)
        COMMAND_HANDLER_EX(IDC_CMB_TAG_PATTERN, CBN_EDITCHANGE, OnConfigChanged)
        COMMAND_HANDLER_EX(IDC_CMB_TAG_PATTERN, CBN_SELCHANGE, OnPatternSelect)

        COMMAND_ID_HANDLER_EX(IDC_BTN_TAG_MENU, OnTagMenu)
        COMMAND_ID_HANDLER_EX(IDC_BTN_SAVE_PATTERN, OnSavePattern)
        COMMAND_ID_HANDLER_EX(IDC_BTN_DEL_PATTERN, OnDelPattern)

        COMMAND_ID_HANDLER_EX(IDC_BTN_SET_SINGLE, OnPresetSingle)
        COMMAND_ID_HANDLER_EX(IDC_BTN_SET_VA, OnPresetVA)
        COMMAND_ID_HANDLER_EX(IDC_BTN_SET_CASE_DEF, OnSetCaseDefault)

        // НОВЫЕ ПЕРЕХВАТЧИКИ ДЛЯ M3U
        COMMAND_ID_HANDLER_EX(IDC_CHK_SAVE_M3U, OnConfigChanged)
        COMMAND_ID_HANDLER_EX(IDC_CHK_M3U_EXTINF, OnConfigChanged)
        COMMAND_ID_HANDLER_EX(IDC_CHK_M3U_OVERWRITE, OnConfigChanged)
        COMMAND_HANDLER_EX(IDC_CMB_M3U_TEMPLATE, CBN_EDITCHANGE, OnConfigChanged)
        COMMAND_HANDLER_EX(IDC_CMB_M3U_TEMPLATE, CBN_SELCHANGE, OnM3UPatternSelect)
        COMMAND_ID_HANDLER_EX(IDC_BTN_M3U_TAGS, OnM3UTagMenu)
        COMMAND_ID_HANDLER_EX(IDC_BTN_M3U_SAVE, OnSaveM3UPattern)
        COMMAND_ID_HANDLER_EX(IDC_BTN_M3U_DELETE, OnDelM3UPattern)

        ALT_MSG_MAP(1)
        MESSAGE_HANDLER_EX(WM_KILLFOCUS, OnEditKillFocus)
        ALT_MSG_MAP(2)
        MESSAGE_HANDLER_EX(WM_KILLFOCUS, OnEditM3UKillFocus)
        END_MSG_MAP()

    BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
    void OnDestroy();
    void OnRename(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnConfigChanged(UINT uNotifyCode, int nID, CWindow wndCtl);

    void OnPatternSelect(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnTagMenu(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnSavePattern(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnDelPattern(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnPresetSingle(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnPresetVA(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnSetCaseDefault(UINT uNotifyCode, int nID, CWindow wndCtl);

    // НОВЫЕ ФУНКЦИИ ДЛЯ M3U
    void OnM3UPatternSelect(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnM3UTagMenu(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnSaveM3UPattern(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnDelM3UPattern(UINT uNotifyCode, int nID, CWindow wndCtl);
    void SaveM3UHistory();

    LRESULT OnEditKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnEditM3UKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CListViewCtrl m_list;
    void UpdatePreview();
    void SaveHistory();
    CString GetDynamicPattern();
};