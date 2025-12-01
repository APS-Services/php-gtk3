
#ifndef _PHPGTK_CEFWEBVIEW_H_
#define _PHPGTK_CEFWEBVIEW_H_

    #include <phpcpp.h>
    #include <gtk/gtk.h>
    #include <string>
    #include <vector>
    #include <map>

    #include "../Gtk/GtkWidget.h"

    #include "../../php-gtk.h"

    /**
     * CefWebView
     *
     * Cross-platform web view widget using Chromium Embedded Framework (CEF)
     * https://bitbucket.org/chromiumembedded/cef
     */
    class CefWebView_ : public GtkWidget_
    {
        /**
         * Publics
         */
        public:

            /**
             *  C++ constructor and destructor
             */
            CefWebView_();
            ~CefWebView_();

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

            void load_html(Php::Parameters &parameters);

            void run_javascript(Php::Parameters &parameters);

            void register_script_message_handler(Php::Parameters &parameters);

            void enable_developer_extras();

            Php::Value get_zoom_level();

            void set_zoom_level(Php::Parameters &parameters);

        /**
         * Private members
         */
        private:
            std::string current_uri;
            std::string current_title;
            bool loading;
            double zoom_level;
            std::vector<std::string> history;
            int history_index;
            std::map<std::string, Php::Value> message_handlers;
    };

#endif
