#include "tcam_view.h"

int main(int argc, char*argv[])
{
    gst_init(&argc, &argv);
    auto app = Gtk::Application::create(argc, argv, "com.theimagingsource.tcam.tcam_view");
    TcamView view;
    app->run(*view.get_window());
    return 0;
}