#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4util/libxfce4util.h>

#include <gtk/gtk.h>
#include <stdlib.h>

#include "calculate-times.h"

#define PACKAGE_VERSION "1.0"
#define PACKAGE_NAME "PrayerTimes"
#define PLUGIN_WEBSITE "https://github.com/enfyna"

typedef struct {
    XfcePanelPlugin* plugin;

    /* panel widgets */
    GtkWidget* ebox;
    GtkWidget* hvbox;
    GtkWidget* label;

    /* sample settings */
    gchar* setting1;
    gint setting2;
    gboolean setting3;

    gint timeout;
    prayer_time* pt_next;
    prayer_times_list* pt_list;
} pt_plugin;

void pt_about(XfcePanelPlugin* plugin)
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

gboolean pt_update(gpointer data);
