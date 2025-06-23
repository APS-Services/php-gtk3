#ifndef _PHPGTK_GDKSEAT_H_
#define _PHPGTK_GDKSEAT_H_

#include <phpcpp.h>
#include <gdk/gdk.h>

/**
 * Wrapper for GdkSeat in PHP-GTK
 */
class GdkSeat_ : public Php::Base
{
public:
    /** Underlying GdkSeat pointer */
    GdkSeat *instance = nullptr;

    /** C++ constructor and destructor */
    GdkSeat_() = default;
    virtual ~GdkSeat_() = default;

    /**
     * PHP constructor
     * Typically not used: seats are obtained from events or display
     */
    void __construct(Php::Parameters &parameters);

    /**
     * Populate wrapper from a GdkSeat*
     */
    void populate(GdkSeat *seat);

    /**
     * Get raw GdkSeat* for internal GTK calls
     */
    GdkSeat* get_instance();
    void     set_instance(GdkSeat *seat);

    /**
     * Example: expose seat name to PHP
     */
    Php::Value get_name(Php::Parameters &parameters);
};

#endif // _PHPGTK_GDKSEAT_H_
