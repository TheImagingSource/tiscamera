#include <gtkmm/scale.h>
#include <gtkmm/adjustment.h>
#include <glibmm/ustring.h>

class LogScale : public Gtk::Scale
{
public:
    LogScale(const Glib::RefPtr<Gtk::Adjustment >& adjustment,Gtk::Orientation orientation=Gtk::Orientation::ORIENTATION_HORIZONTAL);

    double get_value() const;

private:
    Glib::ustring on_format_value(double val);
    double value_to_pos(double value);
    double pos_to_value(double pos) const;

    double min_;
    double max_;
    double value_;
    bool is_logarithmic_;
    const double steps_ = 100;
};