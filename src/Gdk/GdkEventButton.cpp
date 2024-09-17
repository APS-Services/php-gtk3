
#include "GdkEventButton.h"


/**
 *  c++ constructor
 */
GdkEventButton_::GdkEventButton_()
{
    
}

/**
 *  PHP Constructor
 */
void GdkEventButton_::__construct(Php::Parameters& parameters)
{

}

void GdkEventButton_::populate(GdkEventButton event)
{
   // get self reference as Php::Value object
    Php::Value self(this);

    // initialize a public property
    self["type"] = (int)event.type;
    // self["window"] = (int)event.window;  // GdkWindow
    self["send_event"] = (int)event.send_event;
    self["time"] = (int)event.time;
    self["x"] = (int)event.x;
    self["y"] = (int)event.y;
    // Convert event.axes to a std::vector<double>
    if (event.axes != nullptr) {
        std::vector<double> axes(event.axes, event.axes + sizeof(event.axes) / sizeof(event.axes[0]));
        self["axes"] = axes;
    }
    else {
        self["axes"] = nullptr;
    }
    self["state"] = (int)event.state;
    self["button"] = (int)event.button;
    // self["device"] = (int)event.device;  // GdkDevice
    self["x_root"] = (double)event.x_root;
    self["y_root"] = (double)event.y_root;

}