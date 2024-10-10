#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <glib-object.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4util/libxfce4util.h>
#include <string.h>

#include "plugin-dialogs.h"

void pt_configure(XfcePanelPlugin* plugin, pt_plugin* pt)
{
    xfce_panel_plugin_block_menu(plugin);

    GtkWidget* dialog = xfce_titled_dialog_new_with_mixed_buttons(
        _("Prayer Times"),
        NULL, // GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(plugin))),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        "gtk-help", "Help", GTK_RESPONSE_HELP,
        "gtk-save", "Save", GTK_RESPONSE_APPLY,
        NULL
    );

    GtkWidget* container = gtk_dialog_get_content_area(
        GTK_DIALOG(dialog)
    );
    GtkWidget* input_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    const int label_width = 200;
    const char* labels[] = {
        "Latitude (degrees)", "Longitude (degrees)", "Elevation (m)",
        "Shadow Factor [1, 2]", "Isha Angle (degrees)", "Fajr Angle (degrees)",
        "Notification Interval (seconds)"
    };

    gdouble* settings[] = {
        &pt->latitude, &pt->longitude, &pt->elevation,
        &pt->shadow_factor, &pt->isha_angle, &pt->fajr_angle,
        &pt->not_interval
    };

    GPtrArray* entry_array = g_ptr_array_new(); 

    char dts[10];
    for (int i = 0; i < 7; i++) {
        GtkWidget* input_row = gtk_box_new(
            GTK_ORIENTATION_HORIZONTAL, 10
        );

        GtkWidget* label = gtk_label_new(labels[i]);
        gtk_widget_set_size_request(label, label_width, -1);
        gtk_label_set_xalign(GTK_LABEL(label), -1);

        GtkWidget* entry = gtk_entry_new();
        sprintf(dts, i < 2 ? "%.2lf" : "%.0f", *settings[i]);
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
            g_warning(_("Unable to open the following url: %s"),
                PLUGIN_WEBSITE
            );
    } else if (response == GTK_RESPONSE_APPLY) {
        gdouble* settings[] = {
            &pt->latitude, &pt->longitude, &pt->elevation,
            &pt->shadow_factor, &pt->isha_angle, &pt->fajr_angle,
            &pt->not_interval
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
        time_t now = time(NULL);
        struct tm date = *localtime(&now);

        prayer_times_list* ptl = get_prayer_times_list(
            &date, pt->longitude, pt->latitude, pt->elevation, pt->shadow_factor, pt->fajr_angle, pt->isha_angle);
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
            pt->fajr_angle = xfce_rc_read_int_entry(rc, "fajr_angle", 18);
            pt->isha_angle = xfce_rc_read_int_entry(rc, "isha_angle", 17);
            pt->elevation = xfce_rc_read_int_entry(rc, "elevation", 350);
            pt->shadow_factor = xfce_rc_read_int_entry(rc, "shadow_factor", 1);

            const gchar *lat = xfce_rc_read_entry(rc, "latitude", "37.77");
            const gchar *lon = xfce_rc_read_entry(rc, "longitude", "29.08");
            pt->latitude = atof(lat);
            pt->longitude = atof(lon);

            xfce_rc_close(rc);
            return;
        }
    }
    DBG("Applying default settings");

    pt->fajr_angle = 18;
    pt->isha_angle = 17;
    pt->latitude = 37.77;
    pt->longitude = 29.08;
    pt->elevation = 350;
    pt->shadow_factor = 1;
    pt->not_interval = 600;
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
        xfce_rc_write_int_entry(rc, "not_interval", pt->not_interval);
        xfce_rc_write_int_entry(rc, "fajr_angle", pt->fajr_angle);
        xfce_rc_write_int_entry(rc, "isha_angle", pt->isha_angle);
        xfce_rc_write_int_entry(rc, "elevation", pt->elevation);
        xfce_rc_write_int_entry(rc, "shadow_factor", pt->shadow_factor);

        gchar lat[100];
        gchar lon[100];

        sprintf(lat, "%lf", pt->latitude);
        sprintf(lon, "%lf", pt->longitude);

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

    if (G_LIKELY(pt->pt_list != NULL))
        g_free(pt->pt_list);

    if (G_LIKELY(pt->app != NULL))
        g_object_unref(pt->app);

    /* free the plugin structure */
    g_slice_free(pt_plugin, pt);
}
