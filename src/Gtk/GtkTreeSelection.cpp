
#include "GtkTreeSelection.h"

/**
 * Constructor
 */
GtkTreeSelection_::GtkTreeSelection_() = default;

/**
 * Destructor
 */
GtkTreeSelection_::~GtkTreeSelection_() = default;

void GtkTreeSelection_::set_mode(Php::Parameters &parameters) {
  int int_type = (int)parameters[0];
  GtkSelectionMode type = (GtkSelectionMode)int_type;

  gtk_tree_selection_set_mode(GTK_TREE_SELECTION(instance), type);
}

Php::Value GtkTreeSelection_::get_mode() {
  GtkSelectionMode ret = gtk_tree_selection_get_mode(GTK_TREE_SELECTION(instance));

  return ret;
}

void GtkTreeSelection_::set_select_function(Php::Parameters &parameters) {
  // GtkTreeIter *iter;
  // if(parameters.size() > 0) {
  // 	Php::Value object_iter = parameters[0];
  // 	GtkTreeIter_ *phpgtk_iter = (GtkTreeIter_ *)object_iter.implementation();
  // 	iter = GTK_WIDGET(phpgtk_iter->get_instance());
  // }

  // gtk_tree_selection_set_select_function (GTK_TREE_SELECTION(instance), iter);

  throw Php::Exception("GtkTreeSelection_->set_select_function() not implemented yet");
}

void GtkTreeSelection_::get_select_function(Php::Parameters &parameters) {
  // GtkTreeIter *parent;
  // if(parameters.size() > 0) {
  // 	Php::Value object_parent = parameters[0];
  // 	GtkTreeIter_ *phpgtk_parent = (GtkTreeIter_ *)object_parent.implementation();
  // 	parent = GTK_WIDGET(phpgtk_parent->get_instance());
  // }

  // gint position = (gint)parameters[1];

  // gtk_tree_selection_get_select_function (GTK_TREE_SELECTION(instance), parent, position);

  throw Php::Exception("GtkTreeSelection_->get_select_function() not implemented yet");
}

Php::Value GtkTreeSelection_::get_user_data(Php::Parameters &parameters) {
  // GtkTreeIter *parent;
  // if(parameters.size() > 0) {
  // 	Php::Value object_parent = parameters[0];
  // 	GtkTreeIter_ *phpgtk_parent = (GtkTreeIter_ *)object_parent.implementation();
  // 	parent = GTK_WIDGET(phpgtk_parent->get_instance());
  // }

  // GtkTreeIter *sibling;
  // if(parameters.size() > 1) {
  // 	Php::Value object_sibling = parameters[1];
  // 	GtkTreeIter_ *phpgtk_sibling = (GtkTreeIter_ *)object_sibling.implementation();
  // 	sibling = GTK_WIDGET(phpgtk_sibling->get_instance());
  // }

  // gpointer ret = gtk_tree_selection_get_user_data (GTK_TREE_SELECTION(instance), parent,
  // sibling);

  // return ret;

  throw Php::Exception("GtkTreeSelection_->get_user_data() not implemented yet");
}

Php::Value GtkTreeSelection_::get_tree_view() {
  GtkTreeView *ret = gtk_tree_selection_get_tree_view(GTK_TREE_SELECTION(instance));

  GtkTreeView_ *return_parsed = new GtkTreeView_();
  return_parsed->set_instance((gpointer *)ret);
  return Php::Object("GtkTreeView", return_parsed);
}

Php::Value GtkTreeSelection_::get_selected(Php::Parameters &parameters) {
  GtkTreeModel *model;
  GtkTreeIter iter;

  bool ret = gtk_tree_selection_get_selected(GTK_TREE_SELECTION(instance), &model, &iter);
  if (!ret) {
    return false;
  }

  GtkTreeIter_ *return_parsed = new GtkTreeIter_();
  return_parsed->set_instance(iter);

  GtkTreeModel_ *return_parsed_model = new GtkTreeModel_();
  return_parsed_model->set_model(model);

  Php::Value result;
  result[0] = Php::Object("GtkTreeModel", return_parsed_model);
  result[1] = Php::Object("GtkTreeIter", return_parsed);

  return result;
}

/**
 * Callback data for selected_foreach().
 *
 * gtk_tree_selection_selected_foreach() is synchronous - it has invoked the
 * callback for every selected row and returned by the time selected_foreach()
 * exits - so this lives on that function's stack and needs no destroy notify.
 */
struct st_selected_foreach {
  Php::Value callback;
  std::vector<Php::Value> user_parameters;
};

/**
 * GtkTreeSelectionForeachFunc - invoked once per selected row.
 *
 * https://docs.gtk.org/gtk3/callback.TreeSelectionForeachFunc.html
 *
 * Written against the exact signature GTK documents rather than routed through
 * generic_callback(): that helper did not know where in its varargs the user
 * data sat, so it walked them casting each in turn to its own struct type and
 * dereferencing it - reading a GtkTreePath and a GtkTreeIter as if they were
 * PHP values. It only ever worked because arbitrary memory rarely looks like a
 * callable. With a typed callback the position is known, so nothing is guessed.
 */
static void selected_foreach_callback(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
                                      gpointer user_data) {
  auto *callback_object = static_cast<struct st_selected_foreach *>(user_data);

  // (GtkTreeModel, path string, GtkTreeIter, ...user parameters)
  Php::Value internal_parameters;

  GtkTreeModel_ *model_ = new GtkTreeModel_();
  model_->set_model(model);
  internal_parameters[0] = Php::Object("GtkTreeModel", model_);

  // Paths are passed as strings ("0:2"), matching get_selected_rows().
  gchar *path_string = gtk_tree_path_to_string(path);
  internal_parameters[1] = (path_string != nullptr) ? Php::Value(path_string) : Php::Value();
  g_free(path_string);

  GtkTreeIter_ *iter_ = new GtkTreeIter_();
  iter_->set_instance(*iter);
  internal_parameters[2] = Php::Object("GtkTreeIter", iter_);

  // Anything the caller passed after the callback itself
  for (size_t i = 1; i < callback_object->user_parameters.size(); i++) {
    internal_parameters[(int)i + 2] = callback_object->user_parameters[i];
  }

  // Call php function with parameters.
  //
  // As in GObject_::connect_callback: a throwable must not unwind across
  // GLib's C frames, so it is captured here and reported only after the catch
  // scope has exited and released the pending Zend exception.
  std::string callback_error;
  long int callback_error_code = 0;
  bool callback_failed = false;
  try {
    Php::call("call_user_func_array", callback_object->callback, internal_parameters);
  } catch (Php::Throwable &throwable) {
    callback_error = throwable.what();
    callback_error_code = throwable.code();
    callback_failed = true;
  }

  if (callback_failed) {
    phpgtk_report_callback_exception(callback_error, callback_error_code,
                                     "GtkTreeSelection::selected_foreach");
  }
}

void GtkTreeSelection_::selected_foreach(Php::Parameters &parameters) {
  // Validate here, while we are still in PHP space and can throw. Inside the
  // callback a bad handler could only be reported, not raised.
  if (parameters.empty() || !parameters[0].isCallable()) {
    throw Php::Exception("GtkTreeSelection::selected_foreach() expects a callable");
  }

  struct st_selected_foreach callback_object;
  callback_object.callback = parameters[0];
  callback_object.user_parameters.assign(parameters.begin(), parameters.end());

  gtk_tree_selection_selected_foreach(GTK_TREE_SELECTION(instance), selected_foreach_callback,
                                      &callback_object);
}

Php::Value GtkTreeSelection_::get_selected_rows() {
  GtkTreeModel *model;

  GList *ret = gtk_tree_selection_get_selected_rows(GTK_TREE_SELECTION(instance), &model);

  Php::Value ret_arr;

  for (int index = 0; GList *item = g_list_nth(ret, index); index++) {
    ret_arr[index] = gtk_tree_path_to_string((GtkTreePath *)item->data);
  }

  GtkTreeModel_ *return_parsed_model = new GtkTreeModel_();
  return_parsed_model->set_model(model);

  Php::Value result;
  result[0] = Php::Object("GtkTreeModel", return_parsed_model);
  result[1] = ret_arr;

  return result;
}

Php::Value GtkTreeSelection_::count_selected_rows() {
  gint ret = gtk_tree_selection_count_selected_rows(GTK_TREE_SELECTION(instance));

  return ret;
}

void GtkTreeSelection_::select_path(Php::Parameters &parameters) {
  std::string param_path = parameters[0];
  GtkTreePath *path = gtk_tree_path_new_from_string(param_path.c_str());

  gtk_tree_selection_select_path(GTK_TREE_SELECTION(instance), path);
}

void GtkTreeSelection_::unselect_path(Php::Parameters &parameters) {
  std::string param_path = parameters[0];
  GtkTreePath *path = gtk_tree_path_new_from_string(param_path.c_str());

  gtk_tree_selection_unselect_path(GTK_TREE_SELECTION(instance), path);
}

Php::Value GtkTreeSelection_::path_is_selected(Php::Parameters &parameters) {
  std::string param_path = parameters[0];
  GtkTreePath *path = gtk_tree_path_new_from_string(param_path.c_str());

  bool ret = gtk_tree_selection_path_is_selected(GTK_TREE_SELECTION(instance), path);

  return ret;
}

void GtkTreeSelection_::select_iter(Php::Parameters &parameters) {
  Php::Value object_iter = parameters[0];
  GtkTreeIter_ *phpgtk_iter = (GtkTreeIter_ *)object_iter.implementation();
  GtkTreeIter iter = phpgtk_iter->get_instance();

  gtk_tree_selection_select_iter(GTK_TREE_SELECTION(instance), &iter);
}

void GtkTreeSelection_::unselect_iter(Php::Parameters &parameters) {
  Php::Value object_iter = parameters[0];
  GtkTreeIter_ *phpgtk_iter = (GtkTreeIter_ *)object_iter.implementation();
  GtkTreeIter iter = phpgtk_iter->get_instance();

  gtk_tree_selection_unselect_iter(GTK_TREE_SELECTION(instance), &iter);
}

Php::Value GtkTreeSelection_::iter_is_selected(Php::Parameters &parameters) {
  Php::Value object_iter = parameters[0];
  GtkTreeIter_ *phpgtk_iter = (GtkTreeIter_ *)object_iter.implementation();
  GtkTreeIter iter = phpgtk_iter->get_instance();

  bool ret = gtk_tree_selection_iter_is_selected(GTK_TREE_SELECTION(instance), &iter);

  return ret;
}

void GtkTreeSelection_::select_all() {
  gtk_tree_selection_select_all(GTK_TREE_SELECTION(instance));
}

void GtkTreeSelection_::unselect_all() {
  gtk_tree_selection_unselect_all(GTK_TREE_SELECTION(instance));
}

void GtkTreeSelection_::unselect_range(Php::Parameters &parameters) {
  std::string param_start_path = parameters[0];
  GtkTreePath *start_path = gtk_tree_path_new_from_string(param_start_path.c_str());

  std::string param_end_path = parameters[1];
  GtkTreePath *end_path = gtk_tree_path_new_from_string(param_end_path.c_str());

  gtk_tree_selection_unselect_range(GTK_TREE_SELECTION(instance), start_path, end_path);
}
void GtkTreeSelection_::select_range(Php::Parameters &parameters) {
  std::string param_start_path = parameters[0];
  GtkTreePath *start_path = gtk_tree_path_new_from_string(param_start_path.c_str());

  std::string param_end_path = parameters[1];
  GtkTreePath *end_path = gtk_tree_path_new_from_string(param_end_path.c_str());

  gtk_tree_selection_select_range(GTK_TREE_SELECTION(instance), start_path, end_path);
}
