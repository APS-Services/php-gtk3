<?php

/**
 * CEF JavaScript Communication Example
 * 
 * This example demonstrates bidirectional communication between PHP and JavaScript
 * in a CefWebView widget.
 * 
 * Features:
 * - Send messages from JavaScript to PHP
 * - Execute JavaScript from PHP
 * - Register custom message handlers
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

// Create main window
$window = new GtkWindow();
$window->set_title("CEF JavaScript Communication Example");
$window->set_default_size(800, 600);
$window->connect("destroy", "onWindowDestroy");

// Create main vertical box
$vbox = new GtkBox(GtkOrientation::VERTICAL, 0);
$window->add($vbox);

// Create toolbar with buttons to test JavaScript communication
$toolbar = new GtkBox(GtkOrientation::HORIZONTAL, 5);
$toolbar->set_margin_start(5);
$toolbar->set_margin_end(5);
$toolbar->set_margin_top(5);
$toolbar->set_margin_bottom(5);
$vbox->pack_start($toolbar, false, false, 0);

// Button to change background color from PHP
$colorBtn = new GtkButton();
$colorBtn->set_label("Change Background (PHP → JS)");
$colorBtn->connect("clicked", function() use (&$webView) {
    $colors = ['lightblue', 'lightgreen', 'lightyellow', 'lightcoral', 'lightpink'];
    $randomColor = $colors[array_rand($colors)];
    $webView->run_javascript("document.body.style.backgroundColor = '$randomColor';");
});
$toolbar->pack_start($colorBtn, false, false, 0);

// Button to alert from JavaScript
$alertBtn = new GtkButton();
$alertBtn->set_label("Show Alert (PHP → JS)");
$alertBtn->connect("clicked", function() use (&$webView) {
    $webView->run_javascript("alert('Hello from PHP!');");
});
$toolbar->pack_start($alertBtn, false, false, 0);

// Button to update title from JavaScript
$titleBtn = new GtkButton();
$titleBtn->set_label("Update Title (PHP → JS)");
$titleBtn->connect("clicked", function() use (&$webView) {
    $timestamp = date('H:i:s');
    $webView->run_javascript("document.title = 'Updated at $timestamp';");
});
$toolbar->pack_start($titleBtn, false, false, 0);

// Create CefWebView widget
$webView = new CefWebView();

// Register a message handler to receive messages from JavaScript
$webView->register_script_message_handler("phpApp", function($messageData) {
    echo "✉ Message received from JavaScript: $messageData\n";
    
    // Parse the message if it's JSON
    $data = json_decode($messageData, true);
    if ($data && isset($data['type'])) {
        echo "  Type: {$data['type']}\n";
        if (isset($data['message'])) {
            echo "  Message: {$data['message']}\n";
        }
    }
});

// Create a scrolled window
$scrolled = new GtkScrolledWindow();
$scrolled->add($webView);
$vbox->pack_start($scrolled, true, true, 0);

// Create HTML content with JavaScript
$html = <<<HTML
<!DOCTYPE html>
<html>
<head>
    <title>CEF JavaScript Communication</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            padding: 20px;
            background-color: white;
            transition: background-color 0.5s;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        h1 {
            color: #333;
        }
        button {
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            margin: 5px;
            font-size: 14px;
        }
        button:hover {
            background-color: #45a049;
        }
        .info {
            background-color: #f0f0f0;
            padding: 15px;
            border-radius: 5px;
            margin: 10px 0;
        }
        .code {
            background-color: #333;
            color: #0f0;
            padding: 10px;
            border-radius: 5px;
            font-family: monospace;
            overflow-x: auto;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔄 JavaScript ↔ PHP Communication</h1>
        
        <div class="info">
            <h2>Send Messages to PHP (JS → PHP)</h2>
            <p>Click these buttons to send messages from JavaScript to PHP:</p>
            <button onclick="sendSimpleMessage()">Send Simple Message</button>
            <button onclick="sendStructuredMessage()">Send Structured Message</button>
            <button onclick="sendUserInfo()">Send User Info</button>
        </div>
        
        <div class="info">
            <h2>Receive Messages from PHP (PHP → JS)</h2>
            <p>Use the buttons in the toolbar above to:</p>
            <ul>
                <li>Change the background color</li>
                <li>Show an alert dialog</li>
                <li>Update the page title</li>
            </ul>
        </div>
        
        <div class="info">
            <h2>How It Works</h2>
            <div class="code">
// JavaScript to PHP:<br>
window.cef.messageHandlers.phpApp.postMessage('Hello PHP!');<br>
<br>
// PHP to JavaScript:<br>
$webView->run_javascript("alert('Hello JS!');");
            </div>
        </div>
        
        <div id="output"></div>
    </div>
    
    <script>
        // Initialize CEF message handler proxy if not available
        // In a real CEF implementation, this would be provided by the CEF runtime
        if (typeof window.cef === 'undefined') {
            window.cef = {
                messageHandlers: {
                    phpApp: {
                        postMessage: function(data) {
                            // Placeholder for CEF message handler
                            console.log('Would send to PHP:', data);
                            document.getElementById('output').innerHTML = 
                                '<div class="info">⚠ Note: Full CEF integration pending. Message would be sent: ' + 
                                JSON.stringify(data) + '</div>';
                        }
                    }
                }
            };
        }
        
        function sendSimpleMessage() {
            window.cef.messageHandlers.phpApp.postMessage('Hello from JavaScript!');
        }
        
        function sendStructuredMessage() {
            const message = {
                type: 'event',
                message: 'Button clicked',
                timestamp: new Date().toISOString()
            };
            window.cef.messageHandlers.phpApp.postMessage(JSON.stringify(message));
        }
        
        function sendUserInfo() {
            const userInfo = {
                type: 'userInfo',
                userAgent: navigator.userAgent,
                language: navigator.language,
                platform: navigator.platform,
                screenWidth: screen.width,
                screenHeight: screen.height
            };
            window.cef.messageHandlers.phpApp.postMessage(JSON.stringify(userInfo));
        }
        
        // Send initial message when page loads
        window.addEventListener('load', function() {
            console.log('Page loaded successfully');
        });
    </script>
</body>
</html>
HTML;

// Load the HTML content
$webView->load_html($html, "about:communication");

// Show all widgets
$window->show_all();

// Start GTK main loop
Gtk::main();
