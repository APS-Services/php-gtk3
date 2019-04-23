<?php

$fra = 0;

function GtkWindowDestroy($widget=NULL, $event=NULL, $param1=NULL, $param2=NULL, $param3=NULL, $param4=NULL)
{
	Gtk::main_quit();
}

function GtkWindowButtonPressed($widget, $event, $param1=NULL, $param2=NULL, $param3=NULL, $param4=NULL)
{
	global $txt1, $fra;

	$widget->set_label($param1);

	$fra += 0.1;
	$txt1->set_progress_fraction($fra);
	$txt1->set_progress_pulse_step($fra);
	$txt1->progress_pulse();


	// $txt1->set_alignment(0.5);

	// $buffer = $txt1->get_buffer();
	// $buffer->delete_text(4, 3);
	// $txt1->set_buffer($buffer);
	
	//echo $txt1->get_text() . "\n";
	//$txt1->set_text("TESTE 2");
	
	// echo $txt1->get_text_length() . "\n";
}

function GtkWindowButtonReleased($widget, $event, $param1=NULL, $param2=NULL, $param3=NULL, $param4=NULL)
{
	$widget->set_label($param1);
}

function GtkWindowResized($widget, $event)
{
	// echo "Window size: " . $widget->get_allocated_width() . "x" . $widget->get_allocated_height() . "\n";
}

function GtkWindowButtonClicked($widget, $event)
{
	$widget->destroy();
}

function button3_clicked($widget, $event)
{
	global $btn1, $btn2, $btn3, $btn4, $btn5;

	$widget->hide();
	$btn4->grab_focus();

	$btn5->set_visible(TRUE);

}

function button4_clicked($widget, $event)
{
	global $btn1, $btn2, $btn3, $btn4, $btn5;

	$btn3->show();
	$btn3->grab_focus();
	$btn2->set_sensitive(TRUE);

	$btn5->set_visible(FALSE);
}

// ----------------------
// Horizontal box 1
$hbox1 = new GtkHBox(TRUE, 5);

// Button 1
$btn1 = GtkButton::new_with_label("TEST 1");
$hbox1->pack_start($btn1, TRUE, TRUE, 5);

// Button 2
$btn2 = GtkButton::new_with_label("TEST 2");
$hbox1->pack_end($btn2, TRUE, TRUE, 5);
$btn2->set_sensitive(FALSE);

// ----------------------
// Horizontal box 2
$hbox2 = new GtkHBox(TRUE, 5);

// Button 3
$btn3 = GtkButton::new_with_label("TEST 3");
$hbox2->pack_start($btn3, TRUE, TRUE, 5);

// Button 4
$btn4 = GtkButton::new_with_label("TEST 4");
$hbox2->pack_end($btn4, TRUE, TRUE, 5);

// ----------------------
// Vertical box
$vbox = new GtkVBox(TRUE, 5);
$vbox->pack_start($hbox1, TRUE, TRUE, 5);
$vbox->pack_start($hbox2, TRUE, TRUE, 5);

// ----------------------
// Entry 1

$buffer = new GtkEntryBuffer("TESTE");

$txt1 = GtkEntry::new_with_buffer($buffer);

// $txt1->set_placeholder_text("Test field");
// $txt1->set_visibility(FALSE);
// $txt1->set_invisible_char("-");
// $txt1->set_max_length(4);

$txt1->set_margin_start(5);
$txt1->set_margin_end(5);
$vbox->pack_start($txt1, TRUE, FALSE, 5);

// ----------------------
// Window
//$win = new GtkWindow(GtkWindow::POPUP);
$win = new GtkWindow(GtkWindow::TOPLEVEL);
$win->add($vbox);
$win->set_default_size(300, 200);
$win->set_title("PHP-GTK3 @ 7.3");

// Connects
$win->connect("destroy", "GtkWindowDestroy", "param 1", "param 2", "param 3", "param 4");
$win->connect("check-resize", "GtkWindowResized");
$btn1->connect("pressed", "GtkWindowButtonPressed", "Pressed");
$btn1->connect("released", "GtkWindowButtonReleased", "Released");
$btn2->connect("clicked", "GtkWindowButtonClicked");
$btn3->connect("clicked", "button3_clicked");
$btn4->connect("clicked", "button4_clicked");

$win->show_all();

// Add another object parent 
$btn5 = GtkButton::new_with_label("TEST 5");
$btn5->set_tooltip_text("This button has been dinamic added to the parent of TEST 2 button, with 0.5 opacity, cannot get focus");
$btn5->set_focus_on_click(FALSE);
$btn5->set_opacity(0.5);
$btn5->set_margin_top(5);
$btn5->set_margin_bottom(5);
$parent = $btn2->get_parent();
$parent->pack_start($btn5, TRUE, TRUE);
$parent->show_all();

// Loop
Gtk::main();