
#ifndef _PHPGTK_WEBKITWEBVIEW_H_
#define _PHPGTK_WEBKITWEBVIEW_H_

    #include <phpcpp.h>
    #include <gtk/gtk.h>
    #include <webkit2/webkit2.h>

    #include "../Gtk/GtkWidget.h"

    #include "../../php-gtk.h"

    /**
     * WebKitWebView_
     *
     * https://webkitgtk.org/reference/webkit2gtk/stable/WebKitWebView.html
     */
    class WebKitWebView_ : public GtkWidget_
    {
        /**
         * Publics
         */
        public:

            /**
             *  C++ constructor and destructor
             */
            WebKitWebView_();
            ~WebKitWebView_();

            void __construct();

            void load_uri(Php::Parameters &parameters);

            Php::Value get_uri();

            void reload();

            void stop_loading();

            Php::Value can_go_back();

            void go_back();

            Php::Value can_go_forward();

            void go_forward();

            Php::Value get_title();

            Php::Value is_loading();
    };

#endif
