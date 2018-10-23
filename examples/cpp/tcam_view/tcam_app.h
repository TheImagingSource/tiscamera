#ifndef __TCAM_APP_H__
#define __TCAM_APP_H__

#include <gtkmm.h>

class TcamView;

class TcamApplication: public Gtk::Application{
    protected:
        TcamApplication();
        void on_activate() override;

    public:
        static Glib::RefPtr<TcamApplication> create();

    private:
        void create_view();
        void on_window_hide(TcamView *view);

        std::vector<TcamView*> views_;
};


#endif//__TCAM_APP_H__