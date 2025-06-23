#include "GdkSeat.h"

// Return the internal GdkSeat pointer
GdkSeat* GdkSeat_::get_instance()
{
    return instance;
}

// PHP __construct (no args, seats come from events or display)
void GdkSeat_::__construct(Php::Parameters &parameters)
{
    // Typically unused; you obtain seats via GdkEvent or GdkDisplay
}

// Set the internal GdkSeat pointer
void GdkSeat_::set_instance(GdkSeat *seat)
{
    instance = seat;
}

// Populate wrapper from a GdkSeat*
void GdkSeat_::populate(GdkSeat *seat)
{
    instance = seat;
    Php::Value self(this);

    // Expose seat name as example
    const gchar *name = gdk_seat_get_name(seat);
    if (name)
    {
        self["name"] = std::string(name);
    }
}

// Expose the seat name via a method
Php::Value GdkSeat_::get_name(Php::Parameters &parameters)
{
    if (!instance)
    {
        return Php::Value();
    }
    const gchar *name = gdk_seat_get_name(instance);
    if (name)
    {
        return std::string(name);
    }
    return Php::Value();
}
