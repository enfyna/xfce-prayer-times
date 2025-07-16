#include <assert.h>
#include <libxfce4ui/libxfce4ui.h>
#include <stddef.h>

#include "calculate-times.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "libxfce4util/libxfce4util.h"
#include "plugin-dialogs.h"

#define SETTINGS_COUNT (PT_ARGS_COUNT + 3) // 3 = not interval + aggressive mode + show_seconds

#define TW_ENTRY_INT 0
#define TW_ENTRY_FLOAT 1
#define TW_CHECKBOX 2

#define CT_CALC 0
#define CT_NOT 1

typedef struct {
    guchar category;
    guchar t_widget;
    GtkWidget* widget;
    gdouble* ptr;
} Conf;

Conf config[SETTINGS_COUNT] = {
    [0] = { .t_widget = TW_ENTRY_FLOAT },
    [1] = { .t_widget = TW_ENTRY_FLOAT },
    [2] = { .t_widget = TW_ENTRY_FLOAT },
    [3] = { .t_widget = TW_ENTRY_FLOAT },

    [7] = { .category = CT_NOT },
    [8] = { .category = CT_NOT },
    [9] = { .t_widget = TW_CHECKBOX, .category = CT_NOT }
};

#define set_margin(widget, margin)                    \
    do {                                              \
        gtk_widget_set_margin_bottom(widget, margin); \
        gtk_widget_set_margin_start(widget, margin);  \
        gtk_widget_set_margin_top(widget, margin);    \
        gtk_widget_set_margin_end(widget, margin);    \
    } while (0)

#define write_entry(buf, name, val)         \
    do {                                    \
        snprintf(buf, 24, "%lf", val);      \
        xfce_rc_write_entry(rc, name, buf); \
    } while (0)

void pt_configure(XfcePanelPlugin* plugin, pt_plugin* pt)
{
    xfce_panel_plugin_block_menu(plugin);

    GtkWidget* dialog = xfce_titled_dialog_new_with_mixed_buttons(
        _("Prayer Times"),
        NULL,
        GTK_DIALOG_DESTROY_WITH_PARENT,
        "gtk-help", _("Help"), GTK_RESPONSE_HELP,
        "gtk-save", _("Save"), GTK_RESPONSE_APPLY,
        NULL
    );

    GtkWidget* container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget* notebook = gtk_notebook_new();
    GtkWidget* pt_settings_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget* nt_settings_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    set_margin(GTK_WIDGET(notebook), 4);
    set_margin(GTK_WIDGET(pt_settings_box), 4);
    set_margin(GTK_WIDGET(nt_settings_box), 4);

    const int label_width = 200;
    const char* labels[] = {
        _("Isha Angle (degrees)"),
        _("Fajr Angle (degrees)"),
        _("Latitude (degrees)"),
        _("Longitude (degrees)"),
        _("Shadow Factor [1, 2]"),
        _("Elevation (m)"),
        _("Descent Correction (minutes)"),
        _("Notification Interval (minutes)"),
        _("Aggressive Mode (minutes)"),
        _("Show seconds"),
    };

    for (size_t i = 0; i < PT_ARGS_COUNT; i++) {
        config[i].ptr = &pt->pt_args.items[i];
    }

    for (size_t i = PT_ARGS_COUNT; i < SETTINGS_COUNT; i++) {
        config[i].ptr = &pt->pl_args[i - PT_ARGS_COUNT];
    }

    char dts[10];
    for (size_t i = 0; i < SETTINGS_COUNT; i++) {
        GtkWidget* input_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

        GtkWidget* label = gtk_label_new(labels[i]);
        gtk_widget_set_size_request(label, label_width, -1);
        gtk_label_set_xalign(GTK_LABEL(label), -1);

        GtkWidget* widget;
        size_t type = config[i].t_widget;

        if (type == TW_CHECKBOX) {
            widget = gtk_check_button_new();
            gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(widget), *config[i].ptr > 0);
        } else if (type == TW_ENTRY_INT || type == TW_ENTRY_FLOAT) {
            widget = gtk_entry_new();
            snprintf(dts, 10,
                type == TW_ENTRY_INT ? "%.0f" : "%.2f", *config[i].ptr);
            gtk_entry_set_text(GTK_ENTRY(widget), dts);
            gtk_entry_set_alignment(GTK_ENTRY(widget), GTK_ALIGN_END);
        } else {
            assert(FALSE && "Unreachable: undefined t_widget");
        }

        gtk_box_pack_start(GTK_BOX(input_row), label, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(input_row), widget, FALSE, FALSE, 0);

        if (config[i].category == CT_CALC) {
            gtk_box_pack_start(GTK_BOX(pt_settings_box), input_row, FALSE, FALSE, 0);
        } else if (config[i].category == CT_NOT) {
            gtk_box_pack_start(GTK_BOX(nt_settings_box), input_row, FALSE, FALSE, 0);
        } else {
            assert(FALSE && "Unreachable: undefined category");
        }

        config[i].widget = widget;
    }
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), pt_settings_box, gtk_label_new(_("Calculation")));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), nt_settings_box, gtk_label_new(_("Notification")));
    gtk_box_pack_start(GTK_BOX(container), notebook, TRUE, TRUE, 0);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_name(GTK_WINDOW(dialog), "xfce4-settings");

    g_signal_connect(
        G_OBJECT(dialog), "response", G_CALLBACK(pt_configure_response), pt);

    gtk_widget_show_all(dialog);
}

void pt_configure_response(GtkWidget* dialog, gint response, pt_plugin* pt)
{
    switch (response) {
    case GTK_RESPONSE_HELP: {
        gboolean result = g_spawn_command_line_async(
            "exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);
        if (G_UNLIKELY(result == FALSE))
            g_warning("Unable to open the following url: %s", PLUGIN_WEBSITE);
        break;
    }
    case GTK_RESPONSE_APPLY: {
        for (guint i = 0; i < SETTINGS_COUNT; i++) {
            GtkWidget* widget = config[i].widget;
            if (config[i].t_widget == TW_CHECKBOX) {
                const gboolean input = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
                *config[i].ptr = input;
            } else if (config[i].t_widget == TW_ENTRY_INT
                || config[i].t_widget == TW_ENTRY_FLOAT) {
                const gchar* input = gtk_entry_get_text(GTK_ENTRY(widget));
                gdouble res = atof(input);
                *config[i].ptr = res;
            } else {
                assert(FALSE && "Unreachable: undefined t_widget");
            }
        }
        pt_save(pt->plugin, pt);

        pt_list ptl = pt_get_list(&pt->pt_args);
        pt->pt_list = ptl;

        set_tooltip_text(pt);
        break;
    }
    }
    gtk_widget_destroy(dialog);
    xfce_panel_plugin_unblock_menu(pt->plugin);
}

void pt_read(pt_plugin* pt)
{
    XfceRc* rc;
    gchar* file;

    /* get the plugin config file location */
    file = xfce_panel_plugin_save_location(pt->plugin, TRUE);
    g_info("[INFO] Reading config file: %s", file);

    if (G_LIKELY(file != NULL)) {
        /* open the config file, readonly */
        rc = xfce_rc_simple_open(file, TRUE);

        /* cleanup */
        g_free(file);

        if (G_LIKELY(rc != NULL)) {
            /* read the settings */
            elevation(&pt->pt_args) = xfce_rc_read_int_entry(rc, "elevation", 350);
            shadow_factor(&pt->pt_args) = xfce_rc_read_int_entry(rc, "shadow_factor", 1);
            not_interval(pt->pl_args) = xfce_rc_read_int_entry(rc, "not_interval", 600);
            aggressive_mode(pt->pl_args) = xfce_rc_read_int_entry(rc, "aggressive_mode", 0);
            show_seconds(pt->pl_args) = xfce_rc_read_int_entry(rc, "show_seconds", 0);

            const gchar* fajr = xfce_rc_read_entry(rc, "fajr_angle", "18");
            const gchar* isha = xfce_rc_read_entry(rc, "isha_angle", "17");
            const gchar* descent = xfce_rc_read_entry(rc, "descend_correction", "0");
            fajr_angle(&pt->pt_args) = atof(fajr);
            isha_angle(&pt->pt_args) = atof(isha);
            descend_correction(&pt->pt_args) = atof(descent);

            const gchar* lat = xfce_rc_read_entry(rc, "latitude", "37.77");
            const gchar* lon = xfce_rc_read_entry(rc, "longitude", "29.08");
            latitude(&pt->pt_args) = atof(lat);
            longitude(&pt->pt_args) = atof(lon);

            xfce_rc_close(rc);
            return;
        }
    }
    DBG("[INFO] Applying default settings");

    isha_angle(&pt->pt_args) = 17;
    fajr_angle(&pt->pt_args) = 18;
    latitude(&pt->pt_args) = 37.77;
    longitude(&pt->pt_args) = 29.08;
    shadow_factor(&pt->pt_args) = 1;
    elevation(&pt->pt_args) = 350;
    descend_correction(&pt->pt_args) = 0;
    not_interval(pt->pl_args) = 60;
    aggressive_mode(pt->pl_args) = 0;
    show_seconds(pt->pl_args) = 0;
}

void pt_save(XfcePanelPlugin* plugin, pt_plugin* pt)
{
    XfceRc* rc;
    gchar* file;

    /* get the config file location */
    file = xfce_panel_plugin_save_location(plugin, TRUE);

    if (G_UNLIKELY(file == NULL)) {
        DBG("[ERROR] Failed to open config file");
        return;
    }

    /* open the config file, read/write */
    rc = xfce_rc_simple_open(file, FALSE);
    g_free(file);

    if (G_LIKELY(rc != NULL)) {
        DBG("[INFO] Saving to config file");
        xfce_rc_write_int_entry(rc, "elevation", elevation(&pt->pt_args));
        xfce_rc_write_int_entry(rc, "shadow_factor", shadow_factor(&pt->pt_args));
        xfce_rc_write_int_entry(rc, "not_interval", not_interval(pt->pl_args));
        xfce_rc_write_int_entry(rc, "aggressive_mode", aggressive_mode(pt->pl_args));
        xfce_rc_write_int_entry(rc, "show_seconds", show_seconds(pt->pl_args));

        gchar buf[24];
        write_entry(buf, "descend_correction", descend_correction(&pt->pt_args));
        write_entry(buf, "isha_angle", isha_angle(&pt->pt_args));
        write_entry(buf, "fajr_angle", fajr_angle(&pt->pt_args));
        write_entry(buf, "latitude", latitude(&pt->pt_args));
        write_entry(buf, "longitude", longitude(&pt->pt_args));

        /* close the rc file */
        xfce_rc_close(rc);
    }
}

void pt_about(void)
{
    const gchar* auth[] = { "enfyna <ramazanaslan21901@gmail.com>", NULL };
    gtk_show_about_dialog(
        NULL,
        "logo-icon-name", "call-start",
        "license", xfce_get_license_text(XFCE_LICENSE_TEXT_GPL),
        "version", PACKAGE_VERSION,
        "program-name", PACKAGE_NAME,
        "comments", _("Islamic prayer times plugin"),
        "website", PLUGIN_WEBSITE,
        "copyright", "Copyright \xc2\xa9 2024",
        "authors", auth,
        NULL);
}

void pt_free(XfcePanelPlugin* plugin, pt_plugin* pt)
{
    GtkWidget* dialog;

    /* check if the dialog is still open. if so, destroy it */
    dialog = g_object_get_data(G_OBJECT(plugin), "dialog");
    if (G_UNLIKELY(dialog != NULL))
        gtk_widget_destroy(dialog);

    /* destroy the panel widgets */
    gtk_widget_destroy(pt->hvbox);

    if (G_LIKELY(pt->app != NULL))
        g_object_unref(pt->app);

    /* free the plugin structure */
    g_slice_free(pt_plugin, pt);
}
