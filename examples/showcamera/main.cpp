#include <QtGui/QApplication>
#include "showcamera.h"

#include <gst/gst.h>
#include <glib.h>

int main(int argc, char** argv)								
{
    gboolean silent = FALSE;
    gchar *savefile = NULL;

    GOptionContext *ctx;
    GError *err = NULL;
    /*
    GOptionEntry entries[] = 
    {
	{ "silent", 's', 0, G_OPTION_ARG_NONE, &silent,
	  "do not output status information", NULL },
	{ "output", 'o', 0, G_OPTION_ARG_STRING, &savefile,
	  "save xml representation of pipeline to FILE and exit", "FILE" },
	{ NULL }
    };

    ctx = g_option_context_new ("- Show Camera");
    g_option_context_add_main_entries (ctx, entries, NULL);
    g_option_context_add_group (ctx, gst_init_get_option_group ());
    if (!g_option_context_parse (ctx, &argc, &argv, &err)) 
    {
      g_print ("Failed to initialize: %s\n", err->message);
      g_error_free (err);
      return 1;
    }
    */
    gst_init( &argc,&argv); 
    QApplication app(argc, argv);
    showcamera foo;
    foo.show();
    return app.exec();
}
