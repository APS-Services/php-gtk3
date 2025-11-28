<?php

/**
 * WebKitWebView Communication Example
 * 
 * This example demonstrates bidirectional communication between
 * JavaScript (in the WebView) and PHP (GTK application).
 * 
 * Features demonstrated:
 * - Loading static HTML content
 * - Executing JavaScript from PHP
 * - Handling WebView signals (load-changed)
 * - Interactive communication between browser and application
 * 
 * Note: This requires PHP-GTK3 to be compiled with WITH_WEBKIT=1
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
$window->set_title("WebKit Communication Example");
$window->set_default_size(600, 500);
$window->connect("destroy", "onWindowDestroy");

// Create a vertical box for layout
$vbox = new GtkBox(Gtk::ORIENTATION_VERTICAL, 5);
$window->add($vbox);

// Create control buttons
$hbox = new GtkBox(Gtk::ORIENTATION_HORIZONTAL, 5);
$vbox->pack_start($hbox, false, false, 5);

$btnChangeColor = new GtkButton("Change Background Color");
$hbox->pack_start($btnChangeColor, true, true, 0);

$btnUpdateText = new GtkButton("Update Page Text");
$hbox->pack_start($btnUpdateText, true, true, 0);

$btnShowAlert = new GtkButton("Show Alert from PHP");
$hbox->pack_start($btnShowAlert, true, true, 0);

// Create status label
$statusLabel = new GtkLabel("Status: Ready");
$vbox->pack_start($statusLabel, false, false, 5);

// Create WebKitWebView widget
$webView = new WebKitWebView();

// Create a scrolled window to contain the web view
$scrolled = new GtkScrolledWindow();
$scrolled->add($webView);
$vbox->pack_start($scrolled, true, true, 0);

// Static HTML content with JavaScript
$html = <<<'HTML'
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>WebKit Communication Demo</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #f0f0f0;
            transition: background-color 0.5s;
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
        #message {
            font-size: 18px;
            font-weight: bold;
            color: #2196F3;
        }
        #counter {
            font-size: 24px;
            color: #FF5722;
        }
    </style>
</head>
<body>
    <h1>WebKit ↔ PHP Communication Demo</h1>
    
    <div class="info-box">
        <h2>Interactive Content</h2>
        <p id="message">This is a static HTML page loaded into WebKitWebView!</p>
        <p>Click counter: <span id="counter">0</span></p>
        
        <button onclick="incrementCounter()">Click Me!</button>
        <button onclick="changeColor()">Random Color</button>
        <button onclick="showInfo()">Show Info</button>
    </div>
    
    <div class="info-box">
        <h2>Communication Info</h2>
        <ul>
            <li>The buttons above use JavaScript within the browser</li>
            <li>The buttons in the GTK window execute JavaScript from PHP</li>
            <li>This demonstrates bidirectional communication</li>
        </ul>
    </div>
    
    <script>
        let counter = 0;
        
        function incrementCounter() {
            counter++;
            document.getElementById('counter').textContent = counter;
        }
        
        function changeColor() {
            const colors = ['#ffebee', '#e8f5e9', '#e3f2fd', '#fff3e0', '#f3e5f5'];
            const randomColor = colors[Math.floor(Math.random() * colors.length)];
            document.body.style.backgroundColor = randomColor;
        }
        
        function showInfo() {
            alert('This is a JavaScript alert from within the WebView!\n\nCounter value: ' + counter);
        }
        
        function updateMessage(text) {
            document.getElementById('message').textContent = text;
        }
        
        function setBackgroundColor(color) {
            document.body.style.backgroundColor = color;
        }
        
        // Log when page is loaded
        console.log('WebKit Communication Demo loaded successfully!');
    </script>
</body>
</html>
HTML;

// Load the HTML content
$webView->load_html($html, "about:blank");

// Connect to load-changed signal to know when page is ready
$webView->connect("load-changed", function($webView, $loadEvent) use ($statusLabel) {
    // WebKit load events: 0=STARTED, 1=REDIRECTED, 2=COMMITTED, 3=FINISHED
    if ($loadEvent == 3) { // WEBKIT_LOAD_FINISHED
        $statusLabel->set_text("Status: Page loaded successfully!");
    }
});

// Button: Change Background Color from PHP
$btnChangeColor->connect("clicked", function() use ($webView, $statusLabel) {
    $colors = ['#ffcdd2', '#c8e6c9', '#bbdefb', '#ffe0b2', '#e1bee7'];
    $randomColor = $colors[array_rand($colors)];
    
    $js = "setBackgroundColor('$randomColor');";
    $webView->run_javascript($js);
    
    $statusLabel->set_text("Status: Background color changed from PHP!");
});

// Button: Update Text from PHP
$btnUpdateText->connect("clicked", function() use ($webView, $statusLabel) {
    $messages = [
        "Updated from PHP!",
        "PHP controls the browser!",
        "Bidirectional communication works!",
        "Message sent from GTK application",
        "PHP ↔ JavaScript communication active!"
    ];
    $message = $messages[array_rand($messages)];
    
    // Escape single quotes for JavaScript
    $message = str_replace("'", "\\'", $message);
    $js = "updateMessage('$message');";
    $webView->run_javascript($js);
    
    $statusLabel->set_text("Status: Page text updated from PHP!");
});

// Button: Show Alert from PHP
$btnShowAlert->connect("clicked", function() use ($webView, $statusLabel) {
    $time = date('H:i:s');
    $js = "alert('Hello from PHP!\\n\\nCurrent time: $time\\n\\nThis alert was triggered from the GTK application.');";
    $webView->run_javascript($js);
    
    $statusLabel->set_text("Status: Alert shown from PHP at $time");
});

// Show all widgets
$window->show_all();

// Start GTK main loop
Gtk::main();
