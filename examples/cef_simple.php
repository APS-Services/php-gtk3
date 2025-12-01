<?php

/**
 * Simple CefWebView Example
 * 
 * This example shows the minimal code needed to display a web page
 * in a GTK window using CefWebView (Chromium Embedded Framework).
 * 
 * Note: This requires PHP-GTK3 to be compiled with WITH_CEF=1
 */

// Initialize GTK
Gtk::init();

// Callback for when window is closed
function onWindowDestroy()
{
    Gtk::main_quit();
}

// Create main window
$window = new GtkWindow();
$window->set_title("Simple CEF Example");
$window->set_default_size(800, 600);
$window->connect("destroy", "onWindowDestroy");

// Create CefWebView widget
$webView = new CefWebView();

// Create a scrolled window to make the WebView scrollable
// The WebView will automatically resize to fit its parent
$scrolled = new GtkScrolledWindow();
$scrolled->add($webView);
$window->add($scrolled);

// Load a URL
$webView->load_uri("https://www.example.com");

// Show all widgets
$window->show_all();

// Start GTK main loop
Gtk::main();
