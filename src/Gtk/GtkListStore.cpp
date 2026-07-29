
#include "GtkListStore.h"

struct GtkListStore_::st_request_callback {
  Php::Parameters user_parameters;
  Php::Object self_widget;
};

/**
 * Fetch the GtkTreeIter behind a PHP parameter, validating that it really is a
 * GtkTreeIter object first - the C-style cast below is unchecked, so a null or
 * wrong-typed parameter would otherwise crash PHP. Throwing here is safe:
 * these methods run in PHP space, not inside a GLib callback.
 */
static GtkTreeIter phpgtk_require_tree_iter(const Php::Value &value, const char *method) {
  if (!value.isObject() || !value.instanceOf("GtkTreeIter")) {
    throw Php::Exception(std::string(method) + "() expects a GtkTreeIter");
  }

  GtkTreeIter_ *phpgtk_iter = (GtkTreeIter_ *)value.implementation();
  if (phpgtk_iter == nullptr) {
    throw Php::Exception(std::string(method) + "(): invalid GtkTreeIter");
  }

  return phpgtk_iter->get_instance();
}

/**
 * Constructor
 */
GtkListStore_::GtkListStore_() = default;

/**
 * Destructor
 */
GtkListStore_::~GtkListStore_() = default;

void GtkListStore_::__construct(Php::Parameters &parameters) {
  gint n_columns = (gint)parameters.size();

  GType *types;
  types = g_new(GType, n_columns);

  for (int index = 0; index < (int)parameters.size(); index++) {
    int a = parameters[index];

    types[index] = (GType)a;
  }

  // Create the store
  model = GTK_TREE_MODEL(gtk_list_store_newv(n_columns, types));
}

void GtkListStore_::set_column_types(Php::Parameters &parameters) {
  // gint n_columns = (gint)parameters[0];

  // gtk_list_store_set_column_types (GTK_LIST_STORE(model), n_columns, types);

  throw Php::Exception("GtkListStore_::set_column_types not implemented yet");
}

void GtkListStore_::set(Php::Parameters &parameters) {
  throw Php::Exception("GtkListStore_::set not implemented yet");

  // Still not working..

  // GtkTreeIter iter;
  // Php::Value object_iter = parameters[0];
  // GtkTreeIter_* phpgtk_iter = (GtkTreeIter_*)object_iter.implementation();
  // iter = phpgtk_iter->get_instance();

  // gint column = (int)parameters[1];

  // // Get column type
  // GType type_column = gtk_tree_model_get_column_type(GTK_TREE_MODEL(model), column);

  // // Populate the column var with correct type
  // GValue value = phpgtk_get_gvalue(parameters[2], type_column);

  // gtk_list_store_set (GTK_LIST_STORE(model), &iter, column, &value, -1);
}

void GtkListStore_::set_valist(Php::Parameters &parameters) {
  // GtkTreeIter *iter;
  // if(parameters.size() > 0) {
  // 	Php::Value object_iter = parameters[0];
  // 	GtkTreeIter_ *phpgtk_iter = (GtkTreeIter_ *)object_iter.implementation();
  // 	iter = phpgtk_iter->get_instance();
  // }

  // gtk_list_store_set_valist (GTK_LIST_STORE(model), iter, var_args);

  throw Php::Exception("GtkListStore_::set_valist not implemented yet");
}

void GtkListStore_::set_value(Php::Parameters &parameters) {
  GtkTreeIter iter = phpgtk_require_tree_iter(parameters[0], "GtkListStore::set_value");

  gint column = (int)parameters[1];

  // Get column type
  GType type_column = gtk_tree_model_get_column_type(GTK_TREE_MODEL(model), column);

  // Populate the column var with correct type
  GValue value = phpgtk_get_gvalue(parameters[2], type_column);

  // Add the value
  gtk_list_store_set_value(GTK_LIST_STORE(model), &iter, column, &value);
}

void GtkListStore_::set_valuesv(Php::Parameters &parameters) {
  // GtkTreeIter *iter;
  // if(parameters.size() > 0) {
  // 	Php::Value object_iter = parameters[0];
  // 	GtkTreeIter_ *phpgtk_iter = (GtkTreeIter_ *)object_iter.implementation();
  // 	iter = phpgtk_iter->get_instance();
  // }

  // gint columns = (gint)parameters[1];

  // gint n_values = (gint)parameters[3];

  // gtk_list_store_set_valuesv (GTK_LIST_STORE(model), iter, columns, values, n_values);

  throw Php::Exception("GtkListStore_::set_valuesv not implemented yet");
}

Php::Value GtkListStore_::remove(Php::Parameters &parameters) {
  // The iter is required: without this check an omitted parameter passed an
  // uninitialized GtkTreeIter to GTK.
  if (parameters.empty()) {
    throw Php::Exception("GtkListStore::remove() expects a GtkTreeIter");
  }

  GtkTreeIter iter = phpgtk_require_tree_iter(parameters[0], "GtkListStore::remove");

  bool ret = gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

  return ret;
}

Php::Value GtkListStore_::insert(Php::Parameters &parameters) {
  GtkTreeIter iter;

  gint position = (gint)parameters[0];

  gtk_list_store_insert(GTK_LIST_STORE(model), &iter, position);

  GtkTreeIter_ *return_parsed = new GtkTreeIter_();
  return_parsed->set_instance(iter);
  return Php::Object("GtkTreeIter", return_parsed);
}

Php::Value GtkListStore_::insert_before(Php::Parameters &parameters) {
  GtkTreeIter iter;

  // A null/omitted sibling is allowed - GTK then appends the row.
  bool have_sibling = !parameters.empty() && !parameters[0].isNull();
  GtkTreeIter sibling;
  if (have_sibling) {
    sibling = phpgtk_require_tree_iter(parameters[0], "GtkListStore::insert_before");
  }

  gtk_list_store_insert_before(GTK_LIST_STORE(model), &iter, have_sibling ? &sibling : nullptr);

  GtkTreeIter_ *return_parsed = new GtkTreeIter_();
  return_parsed->set_instance(iter);
  return Php::Object("GtkTreeIter", return_parsed);
}

Php::Value GtkListStore_::insert_after(Php::Parameters &parameters) {
  GtkTreeIter iter;

  // A null/omitted sibling is allowed - GTK then prepends the row.
  bool have_sibling = !parameters.empty() && !parameters[0].isNull();
  GtkTreeIter sibling;
  if (have_sibling) {
    sibling = phpgtk_require_tree_iter(parameters[0], "GtkListStore::insert_after");
  }

  gtk_list_store_insert_after(GTK_LIST_STORE(model), &iter, have_sibling ? &sibling : nullptr);

  GtkTreeIter_ *return_parsed = new GtkTreeIter_();
  return_parsed->set_instance(iter);
  return Php::Object("GtkTreeIter", return_parsed);
}

void GtkListStore_::insert_with_values(Php::Parameters &parameters) {
  // GtkTreeIter *iter;
  // if(parameters.size() > 0) {
  // 	Php::Value object_iter = parameters[0];
  // 	GtkTreeIter_ *phpgtk_iter = (GtkTreeIter_ *)object_iter.implementation();
  // 	iter = phpgtk_iter->get_instance();
  // }

  // gint position = (gint)parameters[1];

  // gtk_list_store_insert_with_values (GTK_LIST_STORE(model), iter, position, );

  throw Php::Exception("GtkListStore_::set_valuesv not implemented yet");
}

void GtkListStore_::insert_with_valuesv(Php::Parameters &parameters) {
  throw Php::Exception("GtkListStore_::insert_with_valuesv implemented yet");
}

Php::Value GtkListStore_::prepend(Php::Parameters &parameters) {
  // Add new line
  GtkTreeIter localIter;
  gtk_list_store_prepend(GTK_LIST_STORE(model), &localIter);

  // Get param
  Php::Value arr = parameters[0];

  // Loop columns of param
  for (int index = 0; index < (int)arr.size(); index++) {
    // Get column type
    GType type_column = gtk_tree_model_get_column_type(GTK_TREE_MODEL(model), index);

    // Populate the column var with correct type
    GValue a = phpgtk_get_gvalue(arr[index], type_column);

    // Set the value
    gtk_list_store_set_value(GTK_LIST_STORE(model), &localIter, index, &a);
  }

  GtkTreeIter_ *return_parsed = new GtkTreeIter_();
  return_parsed->set_instance(localIter);
  return Php::Object("GtkTreeIter", return_parsed);
}

Php::Value GtkListStore_::append(Php::Parameters &parameters) {
  // Add new line
  GtkTreeIter localIter;
  gtk_list_store_append(GTK_LIST_STORE(model), &localIter);

  // Get param
  Php::Value arr = parameters[0];
  // Php::call("var_dump", arr);
  // Loop columns of param
  for (int index = 0; index < (int)arr.size(); index++) {
    // Get column type
    GType type_column = gtk_tree_model_get_column_type(GTK_TREE_MODEL(model), index);

    // Populate the column var with correct type
    GValue a = phpgtk_get_gvalue(arr[index], type_column);

    // Php::call("var_dump", g_value_get_long(&a));
    // printf("%lld", g_value_get_long(&a));
    //  Set the value
    gtk_list_store_set_value(GTK_LIST_STORE(model), &localIter, index, &a);
  }

  GtkTreeIter_ *return_parsed = new GtkTreeIter_();
  return_parsed->set_instance(localIter);
  return Php::Object("GtkTreeIter", return_parsed);
}

void GtkListStore_::clear() {
  gtk_list_store_clear(GTK_LIST_STORE(model));
}

Php::Value GtkListStore_::iter_is_valid(Php::Parameters &parameters) {
  // No iter is not a valid iter. Previously an omitted parameter passed an
  // uninitialized GtkTreeIter to GTK.
  if (parameters.empty() || parameters[0].isNull()) {
    return false;
  }

  GtkTreeIter iter = phpgtk_require_tree_iter(parameters[0], "GtkListStore::iter_is_valid");

  bool ret = gtk_list_store_iter_is_valid(GTK_LIST_STORE(model), &iter);

  return ret;
}

void GtkListStore_::reorder(Php::Parameters &parameters) {
  // gint new_order = (gint)parameters[0];

  // gtk_list_store_reorder (GTK_LIST_STORE(model), new_order);

  throw Php::Exception("GtkListStore_::reorder not implemented yet");
}

void GtkListStore_::swap(Php::Parameters &parameters) {
  // Both iters are required: without this check omitted parameters passed
  // uninitialized GtkTreeIters to GTK.
  if (parameters.size() < 2) {
    throw Php::Exception("GtkListStore::swap() expects two GtkTreeIter parameters");
  }

  GtkTreeIter iter_a = phpgtk_require_tree_iter(parameters[0], "GtkListStore::swap");
  GtkTreeIter iter_b = phpgtk_require_tree_iter(parameters[1], "GtkListStore::swap");

  gtk_list_store_swap(GTK_LIST_STORE(model), &iter_a, &iter_b);
}

void GtkListStore_::move_before(Php::Parameters &parameters) {
  // The row iter is required: without this check an omitted parameter passed
  // an uninitialized GtkTreeIter to GTK.
  if (parameters.empty()) {
    throw Php::Exception("GtkListStore::move_before() expects a GtkTreeIter");
  }

  GtkTreeIter iter = phpgtk_require_tree_iter(parameters[0], "GtkListStore::move_before");

  // A null/omitted position is allowed - GTK then moves the row to the end.
  bool have_position = parameters.size() > 1 && !parameters[1].isNull();
  GtkTreeIter position;
  if (have_position) {
    position = phpgtk_require_tree_iter(parameters[1], "GtkListStore::move_before");
  }

  gtk_list_store_move_before(GTK_LIST_STORE(model), &iter, have_position ? &position : nullptr);
}

void GtkListStore_::move_after(Php::Parameters &parameters) {
  // The row iter is required: without this check an omitted parameter passed
  // an uninitialized GtkTreeIter to GTK.
  if (parameters.empty()) {
    throw Php::Exception("GtkListStore::move_after() expects a GtkTreeIter");
  }

  GtkTreeIter iter = phpgtk_require_tree_iter(parameters[0], "GtkListStore::move_after");

  // A null/omitted position is allowed - GTK then moves the row to the start.
  bool have_position = parameters.size() > 1 && !parameters[1].isNull();
  GtkTreeIter position;
  if (have_position) {
    position = phpgtk_require_tree_iter(parameters[1], "GtkListStore::move_after");
  }

  gtk_list_store_move_after(GTK_LIST_STORE(model), &iter, have_position ? &position : nullptr);
}
/**
 * 1. sort_column_id
 * 2. sort_func
 * 3. *user_data
 */
void GtkListStore_::set_sort_func(Php::Parameters &parameters) {
  gint sort_column_id = (gint)parameters[0];

  // Create gpointer user data
  struct st_request_callback *callback_object =
      (struct st_request_callback *)malloc(sizeof(struct st_request_callback));
  memset(callback_object, 0, sizeof(struct st_request_callback));
  callback_object->user_parameters = parameters;
  callback_object->self_widget = Php::Object("GtkListStore", this);

  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(model), sort_column_id, set_sort_func_callback,
                                  (gpointer)callback_object, nullptr);
}

gint GtkListStore_::set_sort_func_callback(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b,
                                           gpointer user_data) {
  // Return to st_callback
  struct st_request_callback *callback_object = (struct st_request_callback *)user_data;

  // Check if callback name is valid
  if (callback_object->user_parameters.size() < 2 || callback_object->user_parameters[1].isNull()) {
    std::cerr << "Invalid callback name" << '\n';
    return 0;
  }
  Php::Value callback_name = callback_object->user_parameters[1];

  // Create internal params (GtkTreeModel, GtkTreeIter, GtkTreeIter, user_data)
  Php::Value internal_parameters;

  // GtkTreeModel model
  GtkTreeModel_ *model_ = new GtkTreeModel_();
  model_->set_model((GtkTreeModel *)model);
  internal_parameters[0] = Php::Object("GtkTreeModel", model_);

  // GtkTreeIter a
  GtkTreeIter_ *iter_a_ = new GtkTreeIter_();
  iter_a_->set_instance(*a);
  internal_parameters[1] = Php::Object("GtkTreeIter", iter_a_);
  // if(!gtk_list_store_iter_is_valid(GTK_LIST_STORE(model), a)) {
  // 	Php::call("var_dump", "ITER A NOT OK");
  // 	internal_parameters[2] = Php::Value();
  // }

  // GtkTreeIter b
  GtkTreeIter_ *iter_b_ = new GtkTreeIter_();
  iter_b_->set_instance(*b);
  internal_parameters[2] = Php::Object("GtkTreeIter", iter_b_);
  // if(!gtk_list_store_iter_is_valid(GTK_LIST_STORE(model), b)) {
  // 	Php::call("var_dump", "ITER B NOT OK");
  // 	internal_parameters[2] = Php::Value();
  // }
  // Value

  // user data
  for (int i = 2; i < (int)callback_object->user_parameters.size(); i++) {
    internal_parameters[i + 1] = callback_object->user_parameters[i];
  }

  // Try to call the PHP function.
  //
  // As in GObject_::connect_callback: a throwable must not unwind across
  // GLib's C frames, so it is captured here and reported only after the catch
  // scope has exited and released the pending Zend exception.
  std::string callback_error;
  long int callback_error_code = 0;
  bool callback_failed = false;
  try {
    gint ret = Php::call("call_user_func_array", callback_name, internal_parameters);
    return ret;
  } catch (Php::Throwable &throwable) {
    callback_error = throwable.what();
    callback_error_code = throwable.code();
    callback_failed = true;
  }

  if (callback_failed) {
    phpgtk_report_callback_exception(callback_error, callback_error_code,
                                     "GtkListStore::set_sort_func");
  }

  // 0 = "rows compare equal": the least disruptive result a failed sort
  // function can produce.
  return 0;
}

void GtkListStore_::set_sort_column_id(Php::Parameters &parameters) {
  gint sort_column_id = (gint)parameters[0];

  int int_order = parameters[1];
  GtkSortType order = (GtkSortType)int_order;

  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), sort_column_id, order);
}

Php::Value GtkListStore_::get_sort_column_id() {
  gint sort_column_id;
  GtkSortType order;

  gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(model), &sort_column_id, &order);

  Php::Value ret;

  ret["sort_column_id"] = sort_column_id;
  ret["order"] = (int)order;

  ret[0] = sort_column_id;
  ret[1] = (int)order;

  return ret;
}

Php::Value GtkListStore_::iter_n_children(Php::Parameters &parameters) {
  if (!parameters.empty() && !parameters[0].isNull()) {
    GtkTreeIter iter = phpgtk_require_tree_iter(parameters[0], "GtkListStore::iter_n_children");

    // Reject stale or never-populated iterators here: GTK would emit a
    // CRITICAL and return -1, which PHP code reading a count would happily
    // treat as truthy.
    if (!gtk_list_store_iter_is_valid(GTK_LIST_STORE(model), &iter)) {
      throw Php::Exception(
          "GtkListStore::iter_n_children(): GtkTreeIter is not valid for this model");
    }

    gint ret = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), &iter);
    return ret;
  }

  gint ret = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), nullptr);

  return ret;
}

Php::Value GtkListStore_::get_iter_first(Php::Parameters &parameters) {
  GtkTreeIter iter;

  bool ret = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);

  return ret;
}