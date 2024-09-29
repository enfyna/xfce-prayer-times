#include <libintl.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4util/libxfce4util.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "plugin-dialogs.h"

static pt_plugin* create_pt_plugin(XfcePanelPlugin* plugin);

static void construct_pt_plugin(XfcePanelPlugin* plugin);
XFCE_PANEL_PLUGIN_REGISTER(construct_pt_plugin);

static void construct_pt_plugin(XfcePanelPlugin* plugin)
{
    pt_plugin* pt;

    pt = create_pt_plugin(plugin);
    g_signal_connect(G_OBJECT(plugin), "about", G_CALLBACK(pt_about), NULL);

    /* read the user settings */
    // sample_read (sample);

    time_t now = time(NULL);
    struct tm date = *localtime(&now);

    prayer_times_list* ptl = get_prayer_times_list(
        &date, 29.08, 37.77, 350, 1);

    prayer_time* next_prayer = get_next_prayer(ptl);
    pt->pt_next = next_prayer;
    pt->pt_list = ptl;

    char* tooltip_text = malloc(sizeof(char) * 400);
    sprintf(tooltip_text,
        "%02d.%02d.%d +%d \n"
        "%2s : Fajr\n"
        "%2s : Sunrise \n"
        "%2s : Zuhr\n"
        "%2s : Asr \n"
        "%2s : Maghrib\n"
        "%2s : Isha  ",
        date.tm_mday, date.tm_mon + 1, date.tm_year + 1900, (int)date.tm_gmtoff / 3600,
        prayer_time_to_string(ptl->FAJR), prayer_time_to_string(ptl->SUNRISE),
        prayer_time_to_string(ptl->ZUHR), prayer_time_to_string(ptl->ASR),
        prayer_time_to_string(ptl->MAGHRIB), prayer_time_to_string(ptl->ISHA));
    gtk_widget_set_tooltip_text(GTK_WIDGET(pt->hvbox), tooltip_text);
    free(tooltip_text);

    pt_update(pt);
    pt->timeout = g_timeout_add_seconds(1, pt_update, pt);
}

static pt_plugin* create_pt_plugin(XfcePanelPlugin* plugin)
{
    GtkOrientation orientation = xfce_panel_plugin_get_orientation(plugin);

    pt_plugin* pt = g_slice_new0(pt_plugin);
    pt->plugin = plugin;

    pt->ebox = gtk_event_box_new();
    pt->hvbox = gtk_box_new(orientation, 2);
    gtk_container_add(GTK_CONTAINER(pt->ebox), pt->hvbox);

    pt->label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(pt->hvbox), pt->label, TRUE, FALSE, 3);

    gtk_container_add(GTK_CONTAINER(plugin), pt->ebox);
    gtk_widget_show_all(GTK_WIDGET(plugin));

    xfce_panel_plugin_menu_show_about(plugin);

    return pt;
}

static void sample_free(XfcePanelPlugin* plugin, pt_plugin* pt)
{
    GtkWidget* dialog;

    /* check if the dialog is still open. if so, destroy it */
    dialog = g_object_get_data(G_OBJECT(plugin), "dialog");
    if (G_UNLIKELY(dialog != NULL))
        gtk_widget_destroy(dialog);

    /* destroy the panel widgets */
    gtk_widget_destroy(pt->hvbox);

    /* cleanup the settings */
    if (G_LIKELY(pt->setting1 != NULL))
        g_free(pt->setting1);

    /* free the plugin structure */
    g_slice_free(pt_plugin, pt);
}

static void sample_orientation_changed(XfcePanelPlugin* plugin,
    GtkOrientation orientation,
    pt_plugin* sample)
{
    /* change the orientation of the box */
    gtk_orientable_set_orientation(GTK_ORIENTABLE(sample->hvbox), orientation);
}

static gboolean sample_size_changed(XfcePanelPlugin* plugin, gint size,
    pt_plugin* sample)
{
    GtkOrientation orientation;

    /* get the orientation of the plugin */
    orientation = xfce_panel_plugin_get_orientation(plugin);

    /* set the widget size */
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
        gtk_widget_set_size_request(GTK_WIDGET(plugin), -1, size);
    else
        gtk_widget_set_size_request(GTK_WIDGET(plugin), size, -1);

    return TRUE;
}

void send_notification(char* time_left)
{
    GApplication* application = 
        g_application_new("prayer.times", G_APPLICATION_FLAGS_NONE);

    g_application_register(application, NULL, NULL);

    GNotification* notification = 
        g_notification_new("Prayer Times");

    g_notification_set_body(notification, 
            time_left);

    GIcon* icon = g_themed_icon_new("call-start");
    g_notification_set_icon(notification, icon);

    g_application_send_notification(application, NULL, notification);
    g_object_unref(icon);
    g_object_unref(notification);
    g_object_unref(application);
}

gboolean pt_update(gpointer data)
{
    pt_plugin* pt = data;
    char* label_text = malloc(sizeof(char) * 10);

    time_t now = time(NULL);
    struct tm* date = localtime(&now);

    int next_prayer_seconds = pt->pt_next->HOUR * 3600 + pt->pt_next->MINUTE * 60 + pt->pt_next->SECOND;
    int current_seconds = date->tm_hour * 3600 + date->tm_min * 60 + date->tm_sec;

    int time_left = next_prayer_seconds - current_seconds;
    if (time_left < 0) {
        prayer_time* next_prayer = get_next_prayer(pt->pt_list);
        pt->pt_next = next_prayer;
    }
    int hour = time_left / 3600;
    int min = ((time_left / 3600.0) - hour) * 60;
    int sec = time_left - (hour * 3600) - (min * 60);

    sprintf(label_text, "%02d:%02d:%02d", hour, min, sec);
    gtk_label_set_text((GtkLabel*)pt->label, label_text);

    if (time_left % 60 == 0) {
        send_notification(label_text);
    }

    free(label_text);
    return TRUE;
}
