#include <libintl.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4util/libxfce4util.h>

#include <glib-object.h>
#include <gio/gio.h>
#include <glib.h>
#include <gtk/gtk.h>

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "plugin-dialogs.h"

static gboolean pt_update(gpointer data);
static pt_plugin* create_pt_plugin(XfcePanelPlugin* plugin);

static void construct_pt_plugin(XfcePanelPlugin* plugin);
XFCE_PANEL_PLUGIN_REGISTER(construct_pt_plugin);

static void construct_pt_plugin(XfcePanelPlugin* plugin)
{
    const int pwd_len = strlen(getenv("PWD")) + 10;
    char locale_dir[pwd_len];
    snprintf(locale_dir, pwd_len, "%s/panel-po", getenv("PWD"));

    bindtextdomain("xpt", locale_dir);
    textdomain("xpt");

    pt_plugin* pt = create_pt_plugin(plugin);

    pt_read(pt);

    pt->pt_list = get_prayer_times_list(
        pt->longitude, pt->latitude, pt->elevation,
        pt->shadow_factor, pt->fajr_angle, pt->isha_angle
    );

    set_tooltip_text(pt);

    pt_update(pt);
    pt->timeout = g_timeout_add_seconds(1, pt_update, pt);
}

static pt_plugin* create_pt_plugin(XfcePanelPlugin* plugin)
{
    GtkOrientation orientation = xfce_panel_plugin_get_orientation(plugin);

    pt_plugin* pt = g_slice_new0(pt_plugin);
    pt->plugin = plugin;

    pt->app = g_application_new("prayer.times", G_APPLICATION_DEFAULT_FLAGS);
    g_application_register(pt->app, NULL, NULL);

    pt->ebox = gtk_event_box_new();
    pt->hvbox = gtk_box_new(orientation, 2);
    gtk_container_add(GTK_CONTAINER(pt->ebox), pt->hvbox);

    pt->label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(pt->hvbox), pt->label, TRUE, FALSE, 5);

    pt->check = gtk_check_button_new();
    gtk_box_pack_start(GTK_BOX(pt->hvbox), pt->check, TRUE, FALSE, 5);
    gtk_widget_set_tooltip_text(GTK_WIDGET(pt->check), _("Check if prayed"));

    gtk_container_add(GTK_CONTAINER(plugin), pt->ebox);
    gtk_widget_show_all(GTK_WIDGET(plugin));

    g_signal_connect(G_OBJECT(plugin), "about",
        G_CALLBACK(pt_about), NULL);

    g_signal_connect(G_OBJECT(plugin), "free-data",
        G_CALLBACK(pt_free), pt);

    g_signal_connect(G_OBJECT(plugin), "configure-plugin",
        G_CALLBACK(pt_configure), pt);

    xfce_panel_plugin_menu_show_about(plugin);
    xfce_panel_plugin_menu_show_configure(plugin);
    return pt;
}

static void send_notification(pt_plugin* pt, char* time_left, int count)
{
    GNotification* notification = g_notification_new(_("Time left to next prayer:"));

    g_notification_set_body(notification, time_left);

    GIcon* icon = g_themed_icon_new("call-start");
    g_notification_set_icon(notification, icon);

    for (int i = 0; i < count; i++) {
        g_application_send_notification(pt->app, NULL, notification);
    }
    g_object_unref(icon);
    g_object_unref(notification);
}

static gboolean pt_update(gpointer data)
{
    pt_plugin* pt = data;
    char label_text[9];

    gboolean prayed = gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(pt->check)
    );

    time_t now = time(NULL);
    struct tm* date = localtime(&now);

    prayer_time* next_prayer = get_next_prayer(pt->pt_list);

    int next_prayer_seconds = next_prayer->HOUR * 3600 + next_prayer->MINUTE * 60 + next_prayer->SECOND;
    int current_seconds = date->tm_hour * 3600 + date->tm_min * 60 + date->tm_sec;

    int time_left = next_prayer_seconds - current_seconds;

    int hour = time_left / 3600;
    int min = (time_left % 3600) / 60;
    int sec = time_left % 60;

    sprintf(label_text, "%02d:%02d:%02d", hour, min, sec);
    gtk_label_set_text((GtkLabel*)pt->label, label_text);

    if (!prayed) {
        if (pt->aggressive_mode > 0 && time_left < pt->aggressive_mode) {
            send_notification(pt, label_text, 5);
        } else if (time_left % (int)pt->not_interval == 0) {
            send_notification(pt, label_text, 1);
        }
    }

    if (time_left <= 0) {
        gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(pt->check), FALSE
        );
    }

    return TRUE;
}

void set_tooltip_text(pt_plugin* pt)
{
    char tooltip_text[200];

    time_t now = time(NULL);
    struct tm date = *localtime(&now);
    prayer_times_list* ptl = pt->pt_list;

    char* fjr_str = prayer_time_to_string(ptl->FAJR);
    char* sun_str = prayer_time_to_string(ptl->SUNRISE);
    char* zhr_str = prayer_time_to_string(ptl->ZUHR);
    char* asr_str = prayer_time_to_string(ptl->ASR);
    char* mrb_str = prayer_time_to_string(ptl->MAGHRIB);
    char* ish_str = prayer_time_to_string(ptl->ISHA);

    sprintf(tooltip_text, _(
        "%02d.%02d.%d %s \n"
        "----------------\n"
        "%2s : Fajr      \n"
        "%2s : Sunrise   \n"
        "%2s : Zuhr      \n"
        "%2s : Asr       \n"
        "%2s : Maghrib   \n"
        "%2s : Isha        "),
        date.tm_mday, date.tm_mon + 1, date.tm_year + 1900, date.tm_zone,
        fjr_str, sun_str, zhr_str, asr_str, mrb_str, ish_str
    );

    gtk_widget_set_tooltip_text(GTK_WIDGET(pt->hvbox), tooltip_text);
    free(fjr_str);
    free(sun_str);
    free(zhr_str);
    free(asr_str);
    free(mrb_str);
    free(ish_str);
}
