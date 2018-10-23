#include "tcam_app.h"
#include "tcam_view.h"

TcamApplication::TcamApplication()
: Gtk::Application("com.theimagingsource.tcam.tcam_view")
{
    Glib::set_application_name("Tcam View");
}

Glib::RefPtr<TcamApplication> TcamApplication::create()
{
    return Glib::RefPtr<TcamApplication>(new TcamApplication);
}

void TcamApplication::create_view()
{
    TcamView *view = new TcamView();
    auto window = view->get_window();
    add_window(*window);
    window->signal_hide().connect(sigc::bind(sigc::mem_fun(*this, &TcamApplication::on_window_hide), view));
    window->show();
    views_.push_back(view);
}

void TcamApplication::on_window_hide(TcamView *view)
{
    views_.erase(std::remove(views_.begin(), views_.end(), view), views_.end());
    delete view;
}

void TcamApplication::on_activate()
{
    create_view();
}

