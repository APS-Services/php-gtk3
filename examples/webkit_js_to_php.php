<?php

/**
 * WebKitWebView JavaScript → PHP Messaging Example
 * 
 * This example demonstrates how JavaScript in the WebView can send
 * custom messages with payloads to the PHP/GTK application.
 * 
 * This uses WebKit's UserContentManager script message handlers.
 * 
 * Note: This requires PHP-GTK3 to be compiled with WITH_WEBKIT=1
 */

// Initialize GTK
Gtk::init();

echo "[PHP] Starting JavaScript → PHP Messaging Example\n";
echo "[PHP] ================================================\n";

// Callback for when window is closed
function onWindowDestroy()
{
    Gtk::main_quit();
}

// Create main window
$window = new GtkWindow();
$window->set_title("JavaScript → PHP Messaging");
$window->set_default_size(700, 600);
$window->connect("destroy", "onWindowDestroy");

// Create a vertical box for layout
$vbox = new GtkBox(Gtk::ORIENTATION_VERTICAL, 5);
$window->add($vbox);

// Create message log area
$logLabel = new GtkLabel("Messages from JavaScript:");
$logLabel->set_xalign(0);
$vbox->pack_start($logLabel, false, false, 5);

// Create scrolled window for message log
$scrolledLog = new GtkScrolledWindow();
$scrolledLog->set_size_request(-1, 150);
$vbox->pack_start($scrolledLog, false, true, 0);

// Create text view for logging messages
$logView = new GtkTextView();
$logView->set_editable(false);
$buffer = $logView->get_buffer();
$scrolledLog->add($logView);

// Create WebKitWebView widget
echo "[PHP] Creating WebKitWebView widget...\n";
$webView = new WebKitWebView();
echo "[PHP] WebKitWebView created successfully\n";

// Register a script message handler named "phpApp" with a callback
// JavaScript can send messages using: window.webkit.messageHandlers.phpApp.postMessage(data)
echo "[PHP] Registering script message handler 'phpApp'...\n";
$webView->register_script_message_handler("phpApp", function ($messageData = null) use ($buffer) {
    // Callback is invoked when JavaScript sends a message
    // The message data sent from JavaScript is passed as the first parameter
    
    echo "[PHP DEBUG] Callback invoked with data: " . var_export($messageData, true) . "\n";
    
    $timestamp = date('H:i:s');
    
    if ($messageData !== null) {
        $message = "[$timestamp] Message from JavaScript: $messageData\n";
    } else {
        $message = "[$timestamp] Message received from JavaScript (no data)\n";
    }

    $endIter = $buffer->get_end_iter();
    $buffer->insert($endIter, $message, -1);

    // Auto-scroll to bottom
    $mark = $buffer->create_mark(null, $endIter, false);
    // Note: scroll_to_mark would be called here in a full implementation
});
echo "[PHP] Script message handler registered successfully\n";

// Create a scrolled window to contain the web view
$scrolled = new GtkScrolledWindow();
$scrolled->add($webView);
$vbox->pack_start($scrolled, true, true, 0);

// Static HTML content with JavaScript that sends messages to PHP
$html = <<<'HTML'
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>JavaScript → PHP Messaging</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #f5f5f5;
        }
        h1 {
            color: #333;
        }
        .info-box {
            background: white;
            padding: 20px;
            border-radius: 8px;
            margin: 20px 0;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        button {
            background-color: #2196F3;
            color: white;
            padding: 12px 24px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            margin: 5px;
            font-size: 14px;
        }
        button:hover {
            background-color: #1976D2;
        }
        button:active {
            background-color: #0D47A1;
        }
        .success {
            background-color: #4CAF50 !important;
        }
        .warning {
            background-color: #FF9800 !important;
        }
        .danger {
            background-color: #F44336 !important;
        }
        #status {
            padding: 10px;
            margin: 10px 0;
            border-radius: 4px;
            background-color: #E3F2FD;
            color: #1565C0;
        }
        .code {
            background: #263238;
            color: #aed581;
            padding: 15px;
            border-radius: 4px;
            font-family: 'Courier New', monospace;
            margin: 10px 0;
            overflow-x: auto;
        }
    </style>
</head>
<body>
    <h1>JavaScript → PHP Messaging Demo</h1>
    
    <div class="info-box">
        <h2>Send Messages to PHP</h2>
        <p>Click the buttons below to send messages from JavaScript to the PHP/GTK application.</p>
        
        <button onclick="sendSimpleMessage()">Send Simple Message</button>
        <button class="success" onclick="sendTextMessage()">Send Text Data</button>
        <button class="warning" onclick="sendObjectMessage()">Send Object Data</button>
        <button class="danger" onclick="sendCounterValue()">Send Counter Value</button>
        
        <div id="status">Ready to send messages...</div>
    </div>
    
    <div class="info-box">
        <h2>Click Counter</h2>
        <p>Clicks: <span id="counter" style="font-size: 24px; color: #F44336;">0</span></p>
        <button onclick="incrementCounter()">Increment Counter</button>
    </div>
    
    <div class="info-box">
        <h2>How It Works</h2>
        <p>JavaScript can send messages to PHP using:</p>
        <div class="code">
window.webkit.messageHandlers.phpApp.postMessage(yourData);
        </div>
        <p>The PHP application receives these messages via a callback function passed to <code>register_script_message_handler()</code>.</p>
    </div>
    
    <script>
        let counter = 0;
        let messageCount = 0;
        
        function updateStatus(message) {
            document.getElementById('status').textContent = message;
        }
        
        function sendSimpleMessage() {
            try {
                window.webkit.messageHandlers.phpApp.postMessage('Hello from JavaScript!');
                messageCount++;
                updateStatus('✓ Simple message sent (#' + messageCount + ')');
            } catch (e) {
                updateStatus('✗ Error: ' + e.message);
            }
        }
        
        function sendTextMessage() {
            try {
                const timestamp = new Date().toLocaleTimeString();
                window.webkit.messageHandlers.phpApp.postMessage('Message at ' + timestamp);
                messageCount++;
                updateStatus('✓ Text message sent with timestamp (#' + messageCount + ')');
            } catch (e) {
                updateStatus('✗ Error: ' + e.message);
            }
        }
        
        function sendObjectMessage() {
            try {
                const data = {
                    type: 'object',
                    timestamp: new Date().toISOString(),
                    counter: counter,
                    random: Math.floor(Math.random() * 100)
                };
                window.webkit.messageHandlers.phpApp.postMessage(JSON.stringify(data));
                messageCount++;
                updateStatus('✓ Object message sent (#' + messageCount + ')');
            } catch (e) {
                updateStatus('✗ Error: ' + e.message);
            }
        }
        
        function sendCounterValue() {
            try {
                window.webkit.messageHandlers.phpApp.postMessage('Counter value: ' + counter);
                messageCount++;
                updateStatus('✓ Counter value sent: ' + counter + ' (#' + messageCount + ')');
            } catch (e) {
                updateStatus('✗ Error: ' + e.message);
            }
        }
        
        function incrementCounter() {
            counter++;
            document.getElementById('counter').textContent = counter;
        }
        
        // Auto-send a welcome message when loaded
        window.addEventListener('load', function() {
            setTimeout(function() {
                try {
                    window.webkit.messageHandlers.phpApp.postMessage('Page loaded successfully!');
                    updateStatus('✓ Initial message sent automatically');
                } catch (e) {
                    updateStatus('WebKit message handler not available');
                }
            }, 500);
        });
    </script>
</body>
</html>
HTML;

// Load the HTML content
echo "[PHP] Loading HTML content into WebView...\n";
$webView->load_html($html, "about:blank");
echo "[PHP] HTML content loaded\n";

// Show all widgets
echo "[PHP] Showing all widgets...\n";
$window->show_all();
echo "[PHP] Widgets shown\n";

// Log initial message
$endIter = $buffer->get_end_iter();
$buffer->insert($endIter, "Waiting for messages from JavaScript...\n", -1);

// Start GTK main loop
echo "[PHP] Starting GTK main loop...\n";
echo "[PHP] Application is now running. You can interact with the UI.\n";
Gtk::main();
