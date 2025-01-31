#include "transactiondialog.hpp"
#include "../../helpers/stringhelpers.hpp"
#include "../../helpers/translation.hpp"

using namespace NickvisionMoney::Controllers;
using namespace NickvisionMoney::Helpers;
using namespace NickvisionMoney::Models;
using namespace NickvisionMoney::UI::Views;

TransactionDialog::TransactionDialog(GtkWindow* parent, TransactionDialogController& controller) : m_controller{ controller }, m_gobj{ adw_message_dialog_new(parent, "", nullptr) }
{
    //Dialog Settings
    gtk_window_set_hide_on_close(GTK_WINDOW(m_gobj), true);
    adw_message_dialog_add_responses(ADW_MESSAGE_DIALOG(m_gobj), "cancel", _("Cancel"), "ok", _("OK"), nullptr);
    adw_message_dialog_set_response_appearance(ADW_MESSAGE_DIALOG(m_gobj), "ok", ADW_RESPONSE_SUGGESTED);
    adw_message_dialog_set_default_response(ADW_MESSAGE_DIALOG(m_gobj), "ok");
    adw_message_dialog_set_close_response(ADW_MESSAGE_DIALOG(m_gobj), "cancel");
    g_signal_connect(m_gobj, "response", G_CALLBACK((void (*)(AdwMessageDialog*, gchar*, gpointer))([](AdwMessageDialog*, gchar* response, gpointer data) { reinterpret_cast<TransactionDialog*>(data)->setResponse({ response }); })), this);
    adw_message_dialog_set_heading(ADW_MESSAGE_DIALOG(m_gobj), StringHelpers::format(_("Transaction - %s"), m_controller.getIdAsString().c_str()).c_str());
    //Grid
    m_boxMain = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    //Main Preferences Group
    m_preferencesGroupMain = adw_preferences_group_new();
    gtk_box_append(GTK_BOX(m_boxMain), m_preferencesGroupMain);
    //Description
    m_rowDescription = adw_entry_row_new();
    gtk_widget_set_size_request(m_rowDescription, 320, -1);
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowDescription), _("Description"));
    adw_entry_row_set_activates_default(ADW_ENTRY_ROW(m_rowDescription), true);
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_preferencesGroupMain), m_rowDescription);
    //Amount
    m_rowAmount = adw_entry_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowAmount), _("Amount"));
    adw_entry_row_set_activates_default(ADW_ENTRY_ROW(m_rowAmount), true);
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_preferencesGroupMain), m_rowAmount);
    //Type Box and Buttons
    m_btnIncome = gtk_toggle_button_new_with_label(pgettext("Transaction|Edition", "Income"));
    g_signal_connect(m_btnIncome, "toggled", G_CALLBACK((void (*)(GtkToggleButton*, gpointer))[](GtkToggleButton*, gpointer data) { reinterpret_cast<TransactionDialog*>(data)->onTypeChanged(); }), this);
    m_btnExpense = gtk_toggle_button_new_with_label(pgettext("Transaction|Edition", "Expense"));
    g_signal_connect(m_btnExpense, "toggled", G_CALLBACK((void (*)(GtkToggleButton*, gpointer))[](GtkToggleButton*, gpointer data) { reinterpret_cast<TransactionDialog*>(data)->onTypeChanged(); }), this);
    g_object_bind_property(m_btnIncome, "active", m_btnExpense, "active", (GBindingFlags)(G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN));
    m_boxType = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(m_boxType, "linked");
    gtk_widget_set_valign(m_boxType, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(m_boxType), m_btnIncome);
    gtk_box_append(GTK_BOX(m_boxType), m_btnExpense);
    //Type
    m_rowType = adw_action_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowType), _("Type"));
    adw_action_row_add_suffix(ADW_ACTION_ROW(m_rowType), m_boxType);
    adw_action_row_set_activatable_widget(ADW_ACTION_ROW(m_rowType), m_btnIncome);
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_preferencesGroupMain), m_rowType);
    //Date and Repeat Interval Preferences Group
    m_preferencesGroupDateRepeat = adw_preferences_group_new();
    gtk_widget_set_hexpand(m_preferencesGroupDateRepeat, true);
    gtk_box_append(GTK_BOX(m_boxMain), m_preferencesGroupDateRepeat);
    //Date
    m_calendarDate = gtk_calendar_new();
    gtk_widget_set_name(m_calendarDate, "calendarTransactions");
    g_signal_connect(m_calendarDate, "day-selected", G_CALLBACK((void (*)(GtkCalendar*, gpointer))([](GtkCalendar*, gpointer data) { reinterpret_cast<TransactionDialog*>(data)->onDateChanged(); })), this);
    m_popoverDate = gtk_popover_new();
    gtk_popover_set_child(GTK_POPOVER(m_popoverDate), m_calendarDate);
    m_btnDate = gtk_menu_button_new();
    gtk_widget_add_css_class(m_btnDate, "flat");
    gtk_widget_set_valign(m_btnDate, GTK_ALIGN_CENTER);
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(m_btnDate), m_popoverDate);
    gtk_menu_button_set_label(GTK_MENU_BUTTON(m_btnDate), g_date_time_format(gtk_calendar_get_date(GTK_CALENDAR(m_calendarDate)), "%x"));
    m_rowDate = adw_action_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowDate), _("Date"));
    adw_action_row_add_suffix(ADW_ACTION_ROW(m_rowDate), m_btnDate);
    adw_action_row_set_activatable_widget(ADW_ACTION_ROW(m_rowDate), m_btnDate);
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_preferencesGroupDateRepeat), m_rowDate);
    //Repeat Interval
    m_rowRepeatInterval = adw_combo_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowRepeatInterval), _("Repeat Interval"));
    adw_combo_row_set_model(ADW_COMBO_ROW(m_rowRepeatInterval), G_LIST_MODEL(gtk_string_list_new(new const char*[8]{ _("Never"), _("Daily"), _("Weekly"), _("Monthly"), _("Quarterly"), _("Yearly"), _("Biyearly"), nullptr })));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_preferencesGroupDateRepeat), m_rowRepeatInterval);
    //Group and Color Preferences Group
    m_preferencesGroupGroupColor = adw_preferences_group_new();
    gtk_box_append(GTK_BOX(m_boxMain), m_preferencesGroupGroupColor);
    //Group
    m_rowGroup = adw_combo_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowGroup), _("Group"));
    const char** groupNames{ new const char*[m_controller.getGroupNames().size() + 1] };
    for(size_t i = 0; i < m_controller.getGroupNames().size(); i++)
    {
        groupNames[i] = m_controller.getGroupNames()[i].c_str();
    }
    groupNames[m_controller.getGroupNames().size()] = nullptr;
    adw_combo_row_set_model(ADW_COMBO_ROW(m_rowGroup), G_LIST_MODEL(gtk_string_list_new(groupNames)));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_preferencesGroupGroupColor), m_rowGroup);
    //Color
    m_btnColor = gtk_color_button_new();
    gtk_widget_set_valign(m_btnColor, GTK_ALIGN_CENTER);
    m_rowColor = adw_action_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowColor), _("Color"));
    adw_action_row_add_suffix(ADW_ACTION_ROW(m_rowColor), m_btnColor);
    adw_action_row_set_activatable_widget(ADW_ACTION_ROW(m_rowColor), m_btnColor);
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_preferencesGroupGroupColor), m_rowColor);
    //Amount Event Controller Key
    //m_eventAmountKey = gtk_event_controller_key_new();
    //g_signal_connect(m_eventAmountKey, "key-released", G_CALLBACK((void (*)(GtkEventControllerKey*, unsigned int, unsigned int, GdkModifierType, gpointer))([](GtkEventControllerKey*, unsigned int keyval, unsigned int, GdkModifierType state, gpointer data) { reinterpret_cast<TransactionDialog*>(data)->onAmountKeyReleased(keyval, state); })), this);
    //gtk_widget_add_controller(m_rowAmount, m_eventAmountKey);
    //Layout
    adw_message_dialog_set_extra_child(ADW_MESSAGE_DIALOG(m_gobj), m_boxMain);
    //Load Transaction
    gtk_calendar_select_day(GTK_CALENDAR(m_calendarDate), g_date_time_new_local(m_controller.getYear(), m_controller.getMonth(), m_controller.getDay(), 0, 0, 0.0));
    gtk_editable_set_text(GTK_EDITABLE(m_rowDescription), m_controller.getDescription().c_str());
    if(m_controller.getType() == TransactionType::Income)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_btnIncome), true);
    }
    else
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_btnExpense), true);
    }
    adw_combo_row_set_selected(ADW_COMBO_ROW(m_rowRepeatInterval), m_controller.getRepeatIntervalAsInt());
    adw_combo_row_set_selected(ADW_COMBO_ROW(m_rowGroup), m_controller.getGroupAsIndex());
    GdkRGBA color;
    if(!gdk_rgba_parse(&color, m_controller.getRGBA().c_str()))
    {
        gdk_rgba_parse(&color, "#3584e4");
    }
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(m_btnColor), &color);
    gtk_editable_set_text(GTK_EDITABLE(m_rowAmount), m_controller.getAmountAsString().c_str());
}

GtkWidget* TransactionDialog::gobj()
{
    return m_gobj;
}

bool TransactionDialog::run()
{
    gtk_widget_show(m_gobj);
    gtk_window_set_modal(GTK_WINDOW(m_gobj), true);
    gtk_widget_grab_focus(m_rowDescription);
    while(gtk_widget_is_visible(m_gobj))
    {
        g_main_context_iteration(g_main_context_default(), false);
    }
    if(m_controller.getResponse() == "ok")
    {
        gtk_widget_hide(m_gobj);
        gtk_window_set_modal(GTK_WINDOW(m_gobj), false);
        TransactionType type{ gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_btnIncome)) ? TransactionType::Income : TransactionType::Expense };
        GdkRGBA color;
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(m_btnColor), &color);
        TransactionCheckStatus status{ m_controller.updateTransaction(g_date_time_format(gtk_calendar_get_date(GTK_CALENDAR(m_calendarDate)), "%Y-%m-%d"), gtk_editable_get_text(GTK_EDITABLE(m_rowDescription)), type, adw_combo_row_get_selected(ADW_COMBO_ROW(m_rowRepeatInterval)), adw_combo_row_get_selected(ADW_COMBO_ROW(m_rowGroup)), gdk_rgba_to_string(&color), gtk_editable_get_text(GTK_EDITABLE(m_rowAmount))) };
        //Invalid Transaction
        if(status != TransactionCheckStatus::Valid)
        {
            //Reset UI
            gtk_widget_remove_css_class(m_rowDescription, "error");
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowDescription), _("Description"));
            gtk_widget_remove_css_class(m_rowAmount, "error");
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowAmount), _("Amount"));
            //Mark Error
            if(status == TransactionCheckStatus::EmptyDescription)
            {
                gtk_widget_add_css_class(m_rowDescription, "error");
                adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowDescription), _("Description (Empty)"));
            }
            else if(status == TransactionCheckStatus::EmptyAmount)
            {
                gtk_widget_add_css_class(m_rowAmount, "error");
                adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowAmount), _("Amount (Empty)"));
            }
            else if(status == TransactionCheckStatus::InvalidAmount)
            {
                gtk_widget_add_css_class(m_rowAmount, "error");
                adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_rowAmount), _("Amount (Invalid)"));
            }
            return run();
        }
    }
    gtk_window_destroy(GTK_WINDOW(m_gobj));
    return m_controller.getResponse() == "ok";
}

void TransactionDialog::setResponse(const std::string& response)
{
    m_controller.setResponse(response);
}

void TransactionDialog::onDateChanged()
{
    gtk_menu_button_set_label(GTK_MENU_BUTTON(m_btnDate), g_date_time_format(gtk_calendar_get_date(GTK_CALENDAR(m_calendarDate)), "%x"));
    gtk_popover_popdown(GTK_POPOVER(m_popoverDate));
}

void TransactionDialog::onTypeChanged()
{
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_btnIncome)))
    {
        gtk_widget_add_css_class(m_btnIncome, "success");
        gtk_widget_remove_css_class(m_btnExpense, "error");
    }
    else
    {
        gtk_widget_remove_css_class(m_btnIncome, "success");
        gtk_widget_add_css_class(m_btnExpense, "error");
    }
}

/*
void TransactionDialog::onAmountKeyReleased(unsigned int keyval, GdkModifierType state)
{
    if(keyval == 65454 && state == 16) //number pad period with no modifier
    {
        if(!m_controller.isLocaleDotDecimalSeperated())
        {
            std::string amountText{ gtk_editable_get_text(GTK_EDITABLE(m_rowAmount)) };
            amountText = amountText.substr(0, amountText.size() - 1);
            amountText += ",";
            gtk_editable_set_text(GTK_EDITABLE(m_rowAmount), amountText.c_str());
            gtk_editable_set_position(GTK_EDITABLE(m_rowAmount), -1);
        }
    }
}
*/
