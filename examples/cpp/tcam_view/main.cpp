#include "tcamcamera.h"
#include "tcam_app.h"

int main(int argc, char*argv[])
{
    gst_init(&argc, &argv);
    //auto app = Gtk::Application::create(argc, argv, "com.theimagingsource.tcam.tcam_view");
    auto app = TcamApplication::create();
    app->run(argc, argv);
    return 0;
}