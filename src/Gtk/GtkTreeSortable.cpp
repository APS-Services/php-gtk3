

#include "GtkTreeSortable.h"

GtkTreeSortable_::GtkTreeSortable_() = default;
GtkTreeSortable_::~GtkTreeSortable_() = default;

void GtkTreeSortable_::sort_column_changed()
{
	gtk_tree_sortable_sort_column_changed(GTK_TREE_SORTABLE(instance));
}

Php::Value GtkTreeSortable_::get_sort_column_id(Php::Parameters& parameters)
{
    // Declare sort_column_id as a gint (not a pointer)
    gint sort_column_id;

    // Cast the parameter directly to GtkSortType (not a pointer)
    GtkSortType order = static_cast<GtkSortType>(parameters[0].numericValue());

    // Pass the address of sort_column_id to the function
    gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(instance), &sort_column_id, &order);

    // Return the actual value of sort_column_id
    return sort_column_id;
}


void GtkTreeSortable_::set_sort_column_id(Php::Parameters &parameters)
{
	gint sort_column_id = (gint)parameters[0];

	int int_order = parameters[1];
	GtkSortType order = (GtkSortType) int_order;

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(instance), sort_column_id, order);
}

Php::Value GtkTreeSortable_::has_default_sort_func()
{
	bool ret = gtk_tree_sortable_has_default_sort_func(GTK_TREE_SORTABLE(instance));

	return ret;
}


