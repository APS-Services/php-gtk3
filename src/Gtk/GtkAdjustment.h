
#ifndef _PHPGTK_GTKADJUSTMENT_H_
#define _PHPGTK_GTKADJUSTMENT_H_

    #include <phpcpp.h>
    #include <gtk/gtk.h>

    #include "../G/GObject.h"

    /**
     * GtkAdjustment_
     * 
     * https://developer.gnome.org/gtk3/stable/GtkAdjustment.html
     */
    class GtkAdjustment_ : public GObject_
    {
        /**
         * Publics
         */
        public:

            /**
             *  C++ constructor and destructor
             */
            GtkAdjustment_();
            ~GtkAdjustment_();

            /**
             * PHP Construct
             */
            void __construct(Php::Parameters &parameters);

            void set_value(Php::Parameters& parameters);

            Php::Value get_value();
    };

#endif