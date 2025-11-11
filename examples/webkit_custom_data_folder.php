#!/usr/bin/env php
<?php
/**
 * WebView Custom Data Folder Example
 * 
 * Demonstrates setting a custom user data folder for WebView2 on Windows.
 * The user data folder stores cache, cookies, local storage, and other browser data.
 * 
 * This is useful for:
 * - Isolating different application instances
 * - Controlling where data is stored
 * - Easy data cleanup/reset
 * 
 * Note: set_user_data_folder() must be called BEFORE the widget is realized (shown).
 * On Linux/macOS, this is a no-op (WebKit2GTK manages data differently).
 */

Gtk::init();

$window = new GtkWindow();
$window->set_title("WebView - Custom Data Folder");
$window->set_default_size(800, 600);
$window->connect_simple("destroy", function () {
    Gtk::main_quit();
});

// Create WebView
$webView = new WebKitWebView();

// Set custom user data folder (Windows only)
// On Windows, this will create/use: C:\Temp\MyAppWebViewData\
// If not called, defaults to: ./tmp (relative to current directory)
// On Linux/macOS, this call has no effect (warning logged)
$customFolder = "C:/Temp/MyAppWebViewData";

echo "[PHP] Setting custom user data folder: $customFolder\n";
echo "[PHP] Note: On Windows, WebView2 data will be stored here.\n";
echo "[PHP] Note: Default location (if not set) is: ./tmp\n";
echo "[PHP] Note: On Linux/macOS, this setting is not used (WebKit2GTK uses different mechanism).\n\n";

$webView->set_user_data_folder($customFolder);

// Create scrolled window for WebView
$scrolled = new GtkScrolledWindow();
$scrolled->add($webView);
$window->add($scrolled);

// Load a page that uses local storage to demonstrate persistent data
$html = <<<'HTML'
<!DOCTYPE html>
<html>
<head>
    <title>Custom Data Folder Demo</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 600px;
            margin: 50px auto;
            padding: 20px;
        }
        .info {
            background: #e7f3ff;
            padding: 15px;
            border-left: 4px solid #2196F3;
            margin-bottom: 20px;
        }
        .test-section {
            background: #f5f5f5;
            padding: 15px;
            margin-top: 20px;
            border-radius: 5px;
        }
        button {
            background: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            margin: 5px;
        }
        button:hover {
            background: #45a049;
        }
        .reset {
            background: #f44336;
        }
        .reset:hover {
            background: #da190b;
        }
        #counter {
            font-size: 24px;
            font-weight: bold;
            color: #2196F3;
        }
    </style>
</head>
<body>
    <h1>Custom User Data Folder Test</h1>
    
    <div class="info">
        <strong>Windows:</strong> Data is stored in: <code>C:\Temp\MyAppWebViewData\</code><br>
        <strong>Default (if not set):</strong> <code>./tmp</code><br>
        <strong>Linux/macOS:</strong> Default WebKit2GTK location is used
    </div>
    
    <div class="test-section">
        <h3>Local Storage Test</h3>
        <p>This counter is stored in localStorage. Close and reopen the app to see it persist!</p>
        <p>Visit count: <span id="counter">0</span></p>
        <button onclick="incrementCounter()">Increment Counter</button>
        <button class="reset" onclick="resetCounter()">Reset Counter</button>
    </div>
    
    <div class="test-section">
        <h3>Cookie Test</h3>
        <p id="cookie-info">No cookie set</p>
        <button onclick="setCookie()">Set Cookie</button>
        <button class="reset" onclick="deleteCookie()">Delete Cookie</button>
    </div>
    
    <script>
        // Initialize counter from localStorage
        function updateCounter() {
            let count = localStorage.getItem('visitCount') || 0;
            document.getElementById('counter').textContent = count;
        }
        
        function incrementCounter() {
            let count = parseInt(localStorage.getItem('visitCount') || 0);
            count++;
            localStorage.setItem('visitCount', count);
            updateCounter();
        }
        
        function resetCounter() {
            localStorage.setItem('visitCount', 0);
            updateCounter();
        }
        
        // Cookie functions
        function setCookie() {
            // Set cookie to expire in 10 years (max-age=315360000 seconds)
            document.cookie = "testCookie=CustomDataFolderDemo; max-age=315360000";
            checkCookie();
        }
        
        function deleteCookie() {
            document.cookie = "testCookie=; expires=Thu, 01 Jan 1970 00:00:00 GMT";
            checkCookie();
        }
        
        function checkCookie() {
            if (document.cookie.indexOf('testCookie') !== -1) {
                document.getElementById('cookie-info').textContent = '✓ Cookie is set: ' + document.cookie;
            } else {
                document.getElementById('cookie-info').textContent = '✗ No cookie set';
            }
        }
        
        // Initialize on load
        updateCounter();
        checkCookie();
    </script>
</body>
</html>
HTML;

$webView->load_html($html, "about:blank");

echo "[PHP] WebView created. Close and reopen the app to see persistent data.\n";
echo "[PHP] On Windows, you can inspect the folder to see cache/storage files.\n";

// Show window
$window->show_all();

Gtk::main();
