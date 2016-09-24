#include <iostream>
#include <string>
#include "sqlite.hpp"

extern "C" {
  #include <emacs-module.h>
}

emacs_value create_list(emacs_env* env, std::vector<std::string> row);


int plugin_is_GPL_compatible;
static SQLiteConnection connection;

emacs_value execute_query(emacs_env* env, std::string query){
  try{
    SQLiteQuery q(query);
    std::vector<row_t> data = q.data();
    emacs_value rows[data.size()];
    for(unsigned int i = 0; i < data.size(); i++){
      rows[i] = create_list(env, data[i]);
    }
    emacs_value response = env->funcall(env, env->intern(env, "list"), data.size(), rows); /* list to store response */
    return response;
  }
  catch(const SQLException& e){
    std::string msg = e.what();
    return env->make_string(env, msg.c_str(), msg.size());
  }
}


emacs_value create_list(emacs_env* env, std::vector<std::string> row){
  emacs_value* values = new emacs_value[row.size()];
  for(unsigned int i = 0; i < row.size(); i++){
    values[i] = env->make_string(env, row[i].c_str(), row[i].size());
  }
  emacs_value list = env->funcall(env, env->intern(env, "list"), row.size(), values);
  delete[] values;
  return list;
}

static emacs_value
Fsqlite_connect(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * data){
  char buffer[256];
  ptrdiff_t size = 256;
  if(env->copy_string_contents(env, args[0], buffer, &size)){
    try{
      connection.open(buffer);
      return env->intern(env, "t");
    }
    catch(const SQLException& e){
      std::string msg = e.what();
      return env->make_string(env, msg.c_str(), msg.size());
    }
  }
  else{
    std::string msg = "Invalid argument";
    return env->make_string(env, msg.c_str(), msg.size());
  }

}

static emacs_value
Fsqlite_query(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * data){

  //return env->funcall(env, env->intern(env, "list"), nargs, args);
  char buffer[256];
  ptrdiff_t size;

  row_t arguments;
  for(int i = 0; i < nargs; i++){
    size = 256;
    /* size represents the number of chars to be read */
    if(env->copy_string_contents(env, args[i], buffer, &size)){
      arguments.push_back(buffer);
    }
    else{
      return env->intern(env, "nil");
    }
  }

  std::string query;
  if(arguments.size() == 1){
    query = arguments[0];
  }

  else if(arguments.size() == 2){
    try{
      connection.open(arguments[0]);
    }
    catch(const SQLException& e){
      std::string msg = e.what();
      return env->make_string(env, msg.c_str(), msg.size());
    }
    query = arguments[1];
  }
  
  return execute_query(env, query);

  /*
  if(env->copy_string_contents(env, args[0], buffer, &size)){
    std::string query = std::string(buffer);
    
    try {
      SQLiteConnection connection("default.sqlite");
      SQLiteQuery* q = new SQLiteQuery(std::string(buffer));
      std::vector<row_t> rows = q->data();
      delete q;
  
      emacs_value row_data[rows.size()];
      for(unsigned int i = 0; i < rows.size(); i++){
	row_data[i] = create_list(env, rows[i]);
      }

      if(rows.size() == 0) return env->intern(env, "nil");
      emacs_value query_result = env->funcall(env, env->intern(env, "list"), rows.size(), row_data);
      return query_result;;
    }
    catch(SQLException& e){
      std::string msg = e.what();
      return env->make_string(env, msg.c_str(), msg.size());
    }
  }
  else {
    return env->intern(env, "nil");
  }
  */
}

  /* 
  

  try{
    SQLiteQuery* q = new SQLiteQuery(std::string(buffer));
    std::vector<row_t> rows = q->data();
    delete q;
  
    emacs_value row_data[rows.size()];
    for(unsigned int i = 0; i < rows.size(); i++){
      row_data[i] = create_list(env, rows[i]);
    }

    if(rows.size() == 0) return env->intern(env, "nil");
    emacs_value query_result = env->funcall(env, env->intern(env, "list"), rows.size(), row_data);
    return query_result;;
  }
  catch(SQLException& e){
    std::string msg = e.what();
    return env->make_string(env, msg.c_str(), msg.size());
  }
	
*/  
static void
bind_function (emacs_env *env, const char* name, emacs_value Sfun){
  /* Convert the strings to symbols by interning them */
  emacs_value Qfset = env->intern (env, "fset");
  emacs_value Qsym = env->intern (env, name);

  /* Prepare the arguments array */
  emacs_value args[] = { Qsym, Sfun };

  /* Make the call (2 == nb of arguments) */
  env->funcall (env, Qfset, 2, args);
}

/* Provide FEATURE to Emacs.  */
static void
provide (emacs_env *env, const char *feature)
{
  /* call 'provide' with FEATURE converted to a symbol */

  emacs_value Qfeat = env->intern (env, feature);
  emacs_value Qprovide = env->intern (env, "provide");
  emacs_value args[] = { Qfeat };

  env->funcall (env, Qprovide, 1, args);
}


extern "C"{
int
emacs_module_init (struct emacs_runtime *ert)
{
  emacs_env *env = ert->get_environment (ert);
				       
  /* create a lambda (returns an emacs_value) */
  emacs_value sqlite_connect = env->make_function (env,
						   1,            /* min. number of arguments */
						   1,            /* max. number of arguments */
						   Fsqlite_connect,  /* actual function pointer */
						   "Specify SQLite database file to connect to.",        /* docstring */
						   NULL          /* user pointer of your choice (data param in Fmymod_test) */
						   );
  /* create a lambda (returns an emacs_value) */
  emacs_value sqlite_query = env->make_function (env,
					      1,            /* min. number of arguments */
					      2,            /* max. number of arguments */
					      Fsqlite_query,  /* actual function pointer */
					      "Execute SQLite query on preset connection",        /* docstring */
					      NULL          /* user pointer of your choice (data param in Fmymod_test) */
					);
				       
  bind_function(env, "sqlite-query", sqlite_query);
  bind_function(env, "sqlite-connect", sqlite_connect);
  provide (env, "emacs-sql");

  /* loaded successfully */
  return 0;
}
}
