
#ifndef _PHPGTK_GDKEVENT_H_
#define _PHPGTK_GDKEVENT_H_

    #include <phpcpp.h>
    #include <gtk/gtk.h>

    #include "GdkEventButton.h"
    #include "GdkEventKey.h"

    /**
     *
     */
    class GdkEvent_ : public Php::Base
    {
        /**
         * Publics
         */
        public:
            GdkEvent *instance;

            /**
             *  C++ constructor and destructor
             */
            GdkEvent_() = default;
            virtual ~GdkEvent_() = default;
            //
            GdkEvent *get_instance();
            void set_instance(GdkEvent *event);

            /**
             * PHP Construct
             */
            void __construct(Php::Parameters &parameters);

            /**
             * Populate GdkEvent to PHPGTK::GDKEVENT
             */
            void populate(GdkEvent *event);

            /** Give PHP a way to fetch the raw GdkEvent*’s seat (or NULL) */
            Php::Value get_seat(Php::Parameters &parameters);
    };

#endif