#include <iostream>
#include <string>
#include "sqlite.hpp"

extern "C" {
#include <emacs-module.h>
}

emacs_value create_list(emacs_env* env, std::vector<std::string> row);


int plugin_is_GPL_compatible;

/* store instances of SQLiteConnection on connection 

   queries find the appropriate instance and use the associated sqlite3*
*/
static connection_map_t current_connections;


emacs_value signal_error(emacs_env* env, std::string msg){
  emacs_value error = env->make_string(env, msg.c_str(), msg.size());
  return env->funcall(env, env->intern(env, "error"), 1, &error);
}


emacs_value execute_query(emacs_env* env, std::string query, SQLiteConnection* connection){
  try{
    SQLiteQuery q(query, connection);
    std::vector<row_t> data = q.data();
    emacs_value rows[data.size()];
    for(unsigned int i = 0; i < data.size(); i++){
      rows[i] = create_list(env, data[i]);
    }
    emacs_value response = env->funcall(env, env->intern(env, "list"), data.size(), rows);
    return response;
  }
  catch(const SQLException& e){
    std::string msg = e.what();
    return signal_error(env, msg);
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
Fsqlite_disconnect(emacs_env* env, ptrdiff_t nargs, emacs_value args[], void * data){
  char buffer[256];
  ptrdiff_t size = 256;
  if(env->copy_string_contents(env, args[0], buffer, &size)){
    std::string connection_name = buffer;
    auto it = current_connections.find(connection_name);
    if(it == current_connections.end()){
      return signal_error(env, "No matching connection to disconnect");
    }
    else{
      /* clean up resources and remove from connections map */
      SQLiteConnection* conn = it->second;
      delete conn;
      current_connections.erase(it);
      return env->intern(env, "t");
    }
  }
  else{
    return signal_error(env, "Invalid argument");
  }
}

static emacs_value
Fsqlite_connections(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * data){
  std::vector<std::string> connections;
  for(const auto& it: current_connections){
    connections.push_back(it.first);
  }
  return create_list(env, connections);
}

static emacs_value
Fsqlite_connect(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * data){
  char buffer[256];
  ptrdiff_t size = 256;
  if(env->copy_string_contents(env, args[0], buffer, &size)){
    try{
      std::string connection_name = buffer;

      auto it = current_connections.find(connection_name);

      if(it != current_connections.end()){
	return signal_error(env, "Connection already exists");
      }
      else{
	SQLiteConnection* connection = new SQLiteConnection;
	connection->open(buffer);
	current_connections[buffer] = connection;
	return env->intern(env, "t");
      }
    }
    catch(const SQLException& e){
      std::string msg = e.what();
      return signal_error(env, msg);
    }
  }
  else{
    std::string msg = "Invalid argument";
    return signal_error(env, msg);
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
  SQLiteConnection* connection;
  try{
    auto it = current_connections.find(arguments[0]);
    if(it == current_connections.end()){
      std::string msg = "Connection not found: " + arguments[0];
      return signal_error(env, msg);
    }
    else{
      connection = it->second;
      query = arguments[1];
    }
      
  }
  catch(const SQLException& e){
    std::string msg = e.what();
    return signal_error(env, msg);
  }
  
  return execute_query(env, query, connection);

}

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
						   2,            /* min. number of arguments */
						   2,            /* max. number of arguments */
						   Fsqlite_query,  /* actual function pointer */
						   "Execute SQLite query on preset connection",        /* docstring */
						   NULL          /* user pointer of your choice (data param in Fmymod_test) */
						   );

    /* create a lambda (returns an emacs_value) */
    emacs_value sqlite_disconnect = env->make_function(env, 1, 1, Fsqlite_disconnect, "Disconnect from SQLite file", NULL);
    
    emacs_value sqlite_connections = env->make_function (env,
						   0,            /* min. number of arguments */
						   0,            /* max. number of arguments */
						   Fsqlite_connections,  /* actual function pointer */
						   "Execute SQLite query on preset connection",        /* docstring */
						   NULL          /* user pointer of your choice (data param in Fmymod_test) */
						   );
				       
    bind_function(env, "sqlite-query", sqlite_query);
    bind_function(env, "sqlite-connect", sqlite_connect);
    bind_function(env, "sqlite-disconnect", sqlite_disconnect);
    bind_function(env, "sqlite-connections", sqlite_connections);
    
    provide (env, "emacs-sql");

    /* loaded successfully */
    return 0;
  }
}
