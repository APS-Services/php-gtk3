<?php

/**
 * CEF Browser Example
 * 
 * A simple browser implementation using CefWebView with navigation controls.
 * This demonstrates the full API of the CefWebView widget.
 * 
 * Note: This requires PHP-GTK3 to be compiled with WITH_CEF=1
 */

// Initialize GTK
Gtk::init();

// Global reference to web view
$webView = null;

// Callback for when window is closed
function onWindowDestroy()
{
    Gtk::main_quit();
}

// Navigation callbacks
function onBackClicked()
{
    global $webView;
    if ($webView->can_go_back()) {
        $webView->go_back();
    }
}

function onForwardClicked()
{
    global $webView;
    if ($webView->can_go_forward()) {
        $webView->go_forward();
    }
}

function onReloadClicked()
{
    global $webView;
    $webView->reload();
}

function onStopClicked()
{
    global $webView;
    $webView->stop_loading();
}

function onGoClicked($entry)
{
    global $webView;
    $url = $entry->get_text();
    
    // Add protocol if missing
    if (!preg_match("/^https?:\/\//i", $url)) {
        $url = "https://" . $url;
    }
    
    $webView->load_uri($url);
}

function onHomeClicked()
{
    global $webView;
    $webView->load_uri("https://www.google.com");
}

function onZoomInClicked()
{
    global $webView;
    $currentZoom = $webView->get_zoom_level();
    $webView->set_zoom_level($currentZoom + 0.1);
}

function onZoomOutClicked()
{
    global $webView;
    $currentZoom = $webView->get_zoom_level();
    $webView->set_zoom_level($currentZoom - 0.1);
}

function onDevToolsClicked()
{
    global $webView;
    $webView->enable_developer_extras();
}

// Create main window
$window = new GtkWindow();
$window->set_title("CEF Browser");
$window->set_default_size(1024, 768);
$window->connect("destroy", "onWindowDestroy");

// Create main vertical box
$vbox = new GtkBox(GtkOrientation::VERTICAL, 0);
$window->add($vbox);

// Create toolbar
$toolbar = new GtkBox(GtkOrientation::HORIZONTAL, 5);
$toolbar->set_margin_start(5);
$toolbar->set_margin_end(5);
$toolbar->set_margin_top(5);
$toolbar->set_margin_bottom(5);
$vbox->pack_start($toolbar, false, false, 0);

// Back button
$backBtn = new GtkButton();
$backBtn->set_label("◄ Back");
$backBtn->connect("clicked", "onBackClicked");
$toolbar->pack_start($backBtn, false, false, 0);

// Forward button
$forwardBtn = new GtkButton();
$forwardBtn->set_label("Forward ►");
$forwardBtn->connect("clicked", "onForwardClicked");
$toolbar->pack_start($forwardBtn, false, false, 0);

// Reload button
$reloadBtn = new GtkButton();
$reloadBtn->set_label("⟳ Reload");
$reloadBtn->connect("clicked", "onReloadClicked");
$toolbar->pack_start($reloadBtn, false, false, 0);

// Stop button
$stopBtn = new GtkButton();
$stopBtn->set_label("✕ Stop");
$stopBtn->connect("clicked", "onStopClicked");
$toolbar->pack_start($stopBtn, false, false, 0);

// Home button
$homeBtn = new GtkButton();
$homeBtn->set_label("⌂ Home");
$homeBtn->connect("clicked", "onHomeClicked");
$toolbar->pack_start($homeBtn, false, false, 0);

// URL entry
$urlEntry = new GtkEntry();
$urlEntry->set_text("https://www.example.com");
$urlEntry->connect("activate", "onGoClicked");
$toolbar->pack_start($urlEntry, true, true, 0);

// Go button
$goBtn = new GtkButton();
$goBtn->set_label("Go");
$goBtn->connect("clicked", function() use ($urlEntry) {
    onGoClicked($urlEntry);
});
$toolbar->pack_start($goBtn, false, false, 0);

// Create second toolbar row for additional controls
$toolbar2 = new GtkBox(GtkOrientation::HORIZONTAL, 5);
$toolbar2->set_margin_start(5);
$toolbar2->set_margin_end(5);
$toolbar2->set_margin_bottom(5);
$vbox->pack_start($toolbar2, false, false, 0);

// Zoom controls
$zoomLabel = new GtkLabel("Zoom:");
$toolbar2->pack_start($zoomLabel, false, false, 0);

$zoomOutBtn = new GtkButton();
$zoomOutBtn->set_label("−");
$zoomOutBtn->connect("clicked", "onZoomOutClicked");
$toolbar2->pack_start($zoomOutBtn, false, false, 0);

$zoomInBtn = new GtkButton();
$zoomInBtn->set_label("+");
$zoomInBtn->connect("clicked", "onZoomInClicked");
$toolbar2->pack_start($zoomInBtn, false, false, 0);

// Developer tools button
$devToolsBtn = new GtkButton();
$devToolsBtn->set_label("🛠 Developer Tools");
$devToolsBtn->connect("clicked", "onDevToolsClicked");
$toolbar2->pack_end($devToolsBtn, false, false, 0);

// Create CefWebView widget
$webView = new CefWebView();

// Create a scrolled window to make the WebView scrollable
$scrolled = new GtkScrolledWindow();
$scrolled->add($webView);
$vbox->pack_start($scrolled, true, true, 0);

// Status bar
$statusbar = new GtkStatusbar();
$vbox->pack_start($statusbar, false, false, 0);

// Load initial page
$webView->load_uri($urlEntry->get_text());

// Show all widgets
$window->show_all();

// Start GTK main loop
Gtk::main();
