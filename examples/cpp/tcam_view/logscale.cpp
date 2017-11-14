#include "logscale.h"

#include <math.h>
#include <string>
#include <iostream>

LogScale::LogScale(const Glib::RefPtr<Gtk::Adjustment >& adjustment,
        Gtk::Orientation orientation)
    :
    min_(adjustment->get_lower()),
    max_(adjustment->get_upper()),
    value_(adjustment->get_value()),
    Gtk::Scale(orientation)
{
    if ((max_ - min_) > 10000)
    {
        set_adjustment(Gtk::Adjustment::create(value_to_pos(adjustment->property_value()), 0.0, 100.0, 1.0, 1.0, 1.0));
        signal_format_value().connect(sigc::mem_fun(*this, &LogScale::on_format_value));
        is_logarithmic_ = true;
    }
    else if((max_ - min_) > 0)
    {
        set_adjustment(adjustment);
        is_logarithmic_ = false;
    }
}

double
LogScale::get_value() const
{
    if (is_logarithmic_)
        return pos_to_value(Gtk::Scale::get_value());
    else
        return Gtk::Scale::get_value();
}

double
LogScale::value_to_pos(double value)
{
    return log(value - min_) / log(max_ - min_) * 100.0;
}

double
LogScale::pos_to_value(double pos) const
{
    return pow(M_E, (pos * log(max_ - min_)) / 100.0);
}

inline std::string format(const char* fmt, ...){
    int size = 512;
    char* buffer = 0;
    buffer = new char[size];
    va_list vl;
    va_start(vl, fmt);
    int nsize = vsnprintf(buffer, size, fmt, vl);
    if(size<=nsize){ //fail delete buffer and try again
        delete[] buffer;
        buffer = 0;
        buffer = new char[nsize+1]; //+1 for /0
        nsize = vsnprintf(buffer, size, fmt, vl);
    }
    std::string ret(buffer);
    va_end(vl);
    delete[] buffer;
    return ret;
}

Glib::ustring
LogScale::on_format_value(double val)
{
    Glib::ustring ret;
    if (is_logarithmic_)
    {
        std::string fmtstring = format("%%.%df", get_digits());
        ret = Glib::ustring(format(fmtstring.c_str(), pos_to_value(val)));
    } else {
        ret = Gtk::Scale::on_format_value(val);
    }
    return ret;
}