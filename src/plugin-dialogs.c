#include <libxfce4ui/libxfce4ui.h>

#include "plugin-dialogs.h"

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

    GtkWidget* container = gtk_dialog_get_content_area(
        GTK_DIALOG(dialog)
    );
    GtkWidget* input_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    const int label_width = 200;
    const char* labels[] = {
        _("Isha Angle (degrees)"), _("Fajr Angle (degrees)"),
        _("Latitude (degrees)"), _("Longitude (degrees)"), 
        _("Shadow Factor [1, 2]"), _("Elevation (m)"),
        _("Notification Interval (seconds)"),
        _("Aggressive Mode")
    };

    gdouble* settings[] = {
        &pt->pt_args->isha_angle, &pt->pt_args->fajr_angle,
        &pt->pt_args->latitude, &pt->pt_args->longitude, 
        &pt->pt_args->shadow_factor, &pt->pt_args->elevation,
        &pt->not_interval, &pt->aggressive_mode
    };

    GPtrArray* entry_array = g_ptr_array_new(); 

    char dts[10];
    for (int i = 0; i < 8; i++) {
        GtkWidget* input_row = gtk_box_new(
            GTK_ORIENTATION_HORIZONTAL, 10
        );

        GtkWidget* label = gtk_label_new(labels[i]);
        gtk_widget_set_size_request(label, label_width, -1);
        gtk_label_set_xalign(GTK_LABEL(label), -1);

        GtkWidget* entry = gtk_entry_new();
        sprintf(dts, i < 4 ? "%.2lf" : "%.0f", *settings[i]);
        gtk_entry_set_text(GTK_ENTRY(entry), dts);
        gtk_entry_set_alignment(GTK_ENTRY(entry), GTK_ALIGN_END);

        gtk_box_pack_start(GTK_BOX(input_row), label, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(input_row), entry, FALSE, FALSE, 0);

        gtk_box_pack_start(GTK_BOX(input_box), input_row, FALSE, FALSE, 2);
        g_ptr_array_add(entry_array, entry);
    }
    g_object_set_data_full(G_OBJECT(plugin),
        "entry_array", entry_array, (GDestroyNotify)g_ptr_array_unref
    );
    gtk_box_pack_start(GTK_BOX(container), input_box, TRUE, TRUE, 12);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_name(GTK_WINDOW(dialog), "xfce4-settings");

    g_object_set_data(G_OBJECT(plugin), "dialog", dialog);
    g_signal_connect(G_OBJECT(dialog), "response",
        G_CALLBACK(pt_configure_response), pt
    );

    gtk_widget_show_all(dialog);
}

void pt_configure_response(
    GtkWidget* dialog, gint response, pt_plugin* pt)
{
    gboolean result;

    if (response == GTK_RESPONSE_HELP) {
        result = g_spawn_command_line_async(
            "exo-open --launch WebBrowser " PLUGIN_WEBSITE,
            NULL
        );
        if (G_UNLIKELY(result == FALSE))
            g_warning("Unable to open the following url: %s",
                PLUGIN_WEBSITE
            );
    } else if (response == GTK_RESPONSE_APPLY) {
        gdouble* settings[] = {
            &pt->pt_args->isha_angle, &pt->pt_args->fajr_angle,
            &pt->pt_args->latitude, &pt->pt_args->longitude, 
            &pt->pt_args->shadow_factor, &pt->pt_args->elevation,
            &pt->not_interval, &pt->aggressive_mode
        };
        GPtrArray* entry_array = g_object_get_data(
            G_OBJECT(pt->plugin), "entry_array"
        );
        for (guint i = 0; i < entry_array->len; i++) {
            GtkEntry* entry = GTK_ENTRY(
                g_ptr_array_index(entry_array, i)
            );
            const gchar* input = gtk_entry_get_text(entry);
            gdouble res = atof(input);
            *settings[i] = res;
        }
        pt_list* ptl = pt_get_list(pt->pt_args);
        pt->pt_list = ptl;

        set_tooltip_text(pt);
    }
    /* remove the dialog data from the plugin */
    g_object_set_data(G_OBJECT(pt->plugin), "dialog", NULL);
    xfce_panel_plugin_unblock_menu(pt->plugin);
    pt_save(pt->plugin, pt);
    gtk_widget_destroy(dialog);
}

void pt_read(pt_plugin* pt)
{
    XfceRc* rc;
    gchar* file;

    /* get the plugin config file location */
    file = xfce_panel_plugin_save_location(pt->plugin, TRUE);
    g_info("config file: %s", file);

    if (G_LIKELY(file != NULL)) {
        /* open the config file, readonly */
        rc = xfce_rc_simple_open(file, TRUE);

        /* cleanup */
        g_free(file);

        if (G_LIKELY(rc != NULL)) {
            /* read the settings */
            pt->not_interval = xfce_rc_read_int_entry(rc, "not_interval", 600);
            pt->pt_args->elevation = xfce_rc_read_int_entry(rc, "elevation", 350);
            pt->pt_args->shadow_factor = xfce_rc_read_int_entry(rc, "shadow_factor", 1);
            pt->aggressive_mode = xfce_rc_read_int_entry(rc, "aggressive_mode", 0);

            const gchar *fajr = xfce_rc_read_entry(rc, "fajr_angle", "18");
            const gchar *isha = xfce_rc_read_entry(rc, "isha_angle", "17");
            pt->pt_args->fajr_angle = atof(fajr);
            pt->pt_args->isha_angle = atof(isha);

            const gchar *lat = xfce_rc_read_entry(rc, "latitude", "37.77");
            const gchar *lon = xfce_rc_read_entry(rc, "longitude", "29.08");
            pt->pt_args->latitude = atof(lat);
            pt->pt_args->longitude = atof(lon);

            xfce_rc_close(rc);
            return;
        }
    }
    DBG("Applying default settings");

    pt->pt_args->fajr_angle = 18;
    pt->pt_args->isha_angle = 17;
    pt->pt_args->latitude = 37.77;
    pt->pt_args->longitude = 29.08;
    pt->pt_args->elevation = 350;
    pt->pt_args->shadow_factor = 1;
    pt->not_interval = 600;
    pt->aggressive_mode = 0;
}

void pt_save(XfcePanelPlugin* plugin, pt_plugin* pt)
{
    XfceRc* rc;
    gchar* file;

    /* get the config file location */
    file = xfce_panel_plugin_save_location(plugin, TRUE);

    if (G_UNLIKELY(file == NULL)) {
        DBG("Failed to open config file");
        return;
    }

    /* open the config file, read/write */
    rc = xfce_rc_simple_open(file, FALSE);
    g_free(file);

    if (G_LIKELY(rc != NULL)) {
        /* save the settings */
        DBG(".");
        xfce_rc_write_int_entry(rc, "elevation", pt->pt_args->elevation);
        xfce_rc_write_int_entry(rc, "shadow_factor", pt->pt_args->shadow_factor);
        xfce_rc_write_int_entry(rc, "not_interval", pt->not_interval);
        xfce_rc_write_int_entry(rc, "aggressive_mode", pt->aggressive_mode);

        gchar fajr[100];
        gchar isha[100];

        sprintf(fajr, "%lf", pt->pt_args->fajr_angle);
        sprintf(isha, "%lf", pt->pt_args->isha_angle);

        xfce_rc_write_entry(rc, "fajr_angle", fajr);
        xfce_rc_write_entry(rc, "isha_angle", isha);

        gchar lat[100];
        gchar lon[100];

        sprintf(lat, "%lf", pt->pt_args->latitude);
        sprintf(lon, "%lf", pt->pt_args->longitude);

        xfce_rc_write_entry(rc, "latitude", lat);
        xfce_rc_write_entry(rc, "longitude", lon);

        /* close the rc file */
        xfce_rc_close(rc);
    }
}

void pt_about()
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
        NULL
    );
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

    if (G_LIKELY(pt->pt_list != NULL))
        g_free(pt->pt_list);

    if (G_LIKELY(pt->app != NULL))
        g_object_unref(pt->app);

    /* free the plugin structure */
    g_slice_free(pt_plugin, pt);
}
