
#include "GdkEvent.h"
#include "GdkSeat.h"

// 
GdkEvent *GdkEvent_::get_instance()
{
    return instance;
}

/**
 *  PHP Constructor
 */
void GdkEvent_::__construct(Php::Parameters& parameters)
{

}

// 
void GdkEvent_::set_instance(GdkEvent *event)
{
    instance = event;
}


void GdkEvent_::populate(GdkEvent *event)
{
    instance = event;
    
   // get self reference as Php::Value object
    Php::Value self(this);

    // GdkEventType
    self["type"] = event->type;
    
    // GtkEventButton
    GdkEventButton_ *eventbutton_ = new GdkEventButton_();
    Php::Value gdkeventbutton = Php::Object("GdkEventButton", eventbutton_);
    eventbutton_->populate(event->button);
    self["button"] = eventbutton_;

    // GtkEventKey
    GdkEventKey_ *eventkey_ = new GdkEventKey_();
    Php::Value gdkeventkey = Php::Object("GdkEventKey", eventkey_);
    eventkey_->populate(event->key);
    self["key"] = eventkey_;
}

Php::Value GdkEvent_::get_seat(Php::Parameters &)
{
    if (!instance) return nullptr;
    GdkSeat *seat = gdk_event_get_seat(instance);
    if (!seat)    return nullptr;
    // If you have a GdkSeat_ wrapper class:
    return Php::Object("GdkSeat", new GdkSeat_(seat));
}

