

#include "GtkAlignment.h"

GtkAlignment_::GtkAlignment_() = default;
GtkAlignment_::~GtkAlignment_() = default;

void GtkAlignment_::set(Php::Parameters &parameters)
{
	double d_xalign = parameters[0];

	gfloat xalign = (float)d_xalign;

	double d_yalign = parameters[1];

	gfloat yalign = (float)d_yalign;

	double d_xscale = parameters[2];

	gfloat xscale = (float)d_xscale;

	double d_yscale = parameters[3];

	gfloat yscale = (float)d_yscale;

	gtk_alignment_set(GTK_ALIGNMENT(instance), xalign, yalign, xscale, yscale);
}

void GtkAlignment_::set_padding(Php::Parameters &parameters)
{
	guint padding_top = (int)parameters[0];

	guint padding_bottom = (int)parameters[1];

	guint padding_left = (int)parameters[2];

	guint padding_right = (int)parameters[3];

	gtk_alignment_set_padding(GTK_ALIGNMENT(instance), padding_top, padding_bottom, padding_left, padding_right);
}

Php::Value GtkAlignment_::get_padding()
{
	// Declare variables as guint (not pointers)
	guint top, bottom, left, right;

	// Pass the addresses of these variables to the function
	gtk_alignment_get_padding(GTK_ALIGNMENT(instance), &top, &bottom, &left, &right);

	// Create a Php::Array to hold the padding values
	Php::Value arr;
	arr["top"] = (int)top;        
	arr["bottom"] = (int)bottom;
	arr["left"] = (int)left;
	arr["right"] = (int)right;

	// Return the array
	return arr;
}



void GtkAlignment_::__construct(Php::Parameters &parameters)
{
	double d_xalign = parameters[0];

	gfloat xalign = (float)d_xalign;

	double d_yalign = parameters[1];

	gfloat yalign = (float)d_yalign;

	double d_xscale = parameters[2];

	gfloat xscale = (float)d_xscale;

	double d_yscale = parameters[3];

	gfloat yscale = (float)d_yscale;

	instance = (gpointer *)gtk_alignment_new(xalign, yalign, xscale, yscale);
}


