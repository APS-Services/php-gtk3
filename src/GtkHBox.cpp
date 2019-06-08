
#include "GtkHBox.h"

/**
 * Constructor
 */
GtkHBox_::GtkHBox_() = default;

/**
 * Destructor
 */
GtkHBox_::~GtkHBox_() = default;

/**
 *  PHP Constructor
 */
void GtkHBox_::__construct(Php::Parameters &parameters)
{
	Php::warning << "GtkHBox is deprecated on Gtk 3.20 and will be removed on non-beta release" << std::flush;

	// Construct the parent
	Php::call("parent::__construct", GTK_ORIENTATION_HORIZONTAL, 0);

	// Verify if has homogeneous parameter
	if(parameters.size() >= 1) {
		Php::call("parent::set_homogeneous", parameters[0]);
	}

	// Verify if has spacing parameter
	if(parameters.size() >= 2) {
		Php::call("parent::set_spacing", parameters[1]);
	}
}