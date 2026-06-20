#include <foobar2000/SDK/foobar2000.h>
#include "editor_dialog.h"

class my_context_menu : public contextmenu_item_simple {
public:
    unsigned get_num_items() override { return 1; }

    void get_item_name(unsigned p_index, pfc::string_base& p_out) override {
        p_out = "File Name Editor...";
    }

    void get_item_default_path(unsigned p_index, pfc::string_base& p_out) override {
        p_out = "File Operations";
    }

    // ИСПРАВЛЕНИЕ 1: Возвращаем bool и отдаем true
    bool get_item_description(unsigned p_index, pfc::string_base& p_out) override {
        p_out = "Open Advanced File Name Editor dialog.";
        return true;
    }

    void context_command(unsigned p_index, metadb_handle_list_cref p_data, const GUID& p_caller) override {
        if (p_data.get_count() == 0) return;

        // Создаем объект нашего диалога и передаем ему выделенные треки
        CEditorDialog dlg(p_data);

        // Открываем окно в модальном режиме (поверх основного окна фубара)
        dlg.DoModal(core_api::get_main_window());
    }

    GUID get_item_guid(unsigned p_index) override {
        static const GUID guid =
        { 0x9f1b4e2d, 0x8a3c, 0x4e61, { 0xbd, 0x22, 0x7f, 0xc1, 0x38, 0x9a, 0x55, 0x1c } };
        return guid;
    }

    // ИСПРАВЛЕНИЕ 2: Полностью убрали get_item_display
};

static contextmenu_item_factory_t<my_context_menu> g_my_context_menu_factory;