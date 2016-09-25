#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "emacs-module.h"
#include "emacs-symbols.h"
#include "emacs-housekeeping.h"

int plugin_is_GPL_compatible;

static void
Fstatement_fin(void* data){
  sqlite3_finalize(data);
}


static emacs_value
Fsqlite_step(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * data){
  sqlite3_stmt* statement = env->get_user_ptr(env, args[0]);
 
  int result = sqlite3_step(statement);
  return env->make_integer(env, result);
}

/* Prepare SQL statement */
static emacs_value
Fsqlite_prepare(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * data){
  (void)data;
  (void)nargs;
  
  sqlite3* db = env->get_user_ptr(env, args[0]);

  
  ptrdiff_t size = 0;
  env->copy_string_contents(env, args[1], NULL, &size);
  if(env->non_local_exit_check(env) != emacs_funcall_exit_return){
    return env->intern(env, "nil");
  }

  char* sql_str = malloc(size*sizeof(char));
  env->copy_string_contents(env, args[1], sql_str, &size);

  
  sqlite3_stmt* statement;
  int result = sqlite3_prepare_v2(db, sql_str, size, &statement, NULL);
  free(sql_str);
  if(result != SQLITE_OK){
    /* Error */
    emacs_value symbol = env->intern(env, "error");
    const char *message = sqlite3_errstr(result);
    emacs_value exit_data = env->make_string(env, message, strlen(message));
    env->non_local_exit_signal(env, symbol, exit_data);
    return env->intern(env, "nil");
  }
  else{
    emacs_value e_statement = env->make_user_ptr(env, Fstatement_fin, statement);
    return e_statement;
  }
  
}


/* Close db */
static emacs_value
Fsqlite_close(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * data){
  (void)nargs;
  (void)data;

  sqlite3* db = env->get_user_ptr(env, args[0]);

  int result = sqlite3_close_v2(db);
  if(result == SQLITE_OK){
    return env->intern(env, "t");
  }
  else if(result == SQLITE_BUSY){
    /* Signal the error message. */
    emacs_value symbol = env->intern(env, "error");
    const char *message = "Database is currently busy";
    emacs_value data = env->make_string(env, message, strlen(message));
    env->non_local_exit_signal(env, symbol, data);
    return env->intern(env, "f");
  }
  emacs_value symbol = env->intern(env, "error");
  const char *message = "Unexpected error - unknown state.";
  emacs_value exit_data = env->make_string(env, message, strlen(message));
  env->non_local_exit_signal(env, symbol, exit_data);

  return env->intern(env, "f");
}

static void
sqlite_close_fin(void* pDB){
  sqlite3_close((sqlite3*) pDB);
}

/* Allow multiple arguments assume defaults */
static emacs_value
Fsqlite_open(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * data){
  /* Silence unused variable warnings */
  (void)nargs;
  (void)data;

  ptrdiff_t size = 0;
  /* Get length of arg zero */
  env->copy_string_contents(env, args[0], NULL, &size);
  if(env->non_local_exit_check(env) != emacs_funcall_exit_return){
    return env->intern(env, "nil");
  }

  char* db_name = malloc(size*sizeof(char));
  env->copy_string_contents(env, args[0], db_name, &size);

  int sqlite_flags = 0;

  if(nargs == 2){
    sqlite_flags = env->extract_integer(env, args[1]);
  }
  else{
    sqlite_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  }

  sqlite3* db;
  int result = sqlite3_open_v2(db_name, &db, sqlite_flags, NULL);
  free(db_name);
  if(result == SQLITE_OK){
    /* return user ptr */
    return env->make_user_ptr(env, sqlite_close_fin, db);
  }
  else{
    /* Signal the error message. */
    emacs_value symbol = env->intern(env, "error");
    const char *message = sqlite3_errstr(result);
    emacs_value exit_data = env->make_string(env, message, strlen(message));
    env->non_local_exit_signal(env, symbol, exit_data);
    return env->intern(env, "nil");
  }
}



int
emacs_module_init (struct emacs_runtime *ert)
{
  emacs_env *env = ert->get_environment (ert);

  export_symbols(env);

  const char* open_docstr = "Open SQLite3 DB file\nArguments: db_path permissions...\n"\
    "permissions are ORed together.";
  emacs_value sqlite_open = env->make_function(env, 1, 2, Fsqlite_open, open_docstr, NULL);
  bind_function(env, "sqlite3-open", sqlite_open);

  const char* close_docstr = "Close existing DB connection";
  emacs_value sqlite_close = env->make_function(env, 1, 1, Fsqlite_close, close_docstr, NULL);
  bind_function(env, "sqlite3-close", sqlite_close);

  const char* prepare_docstr = "Prepare SQL statement for execution";
  emacs_value sqlite_prepare = env->make_function(env, 2, 2, Fsqlite_prepare, prepare_docstr, NULL);
  bind_function(env, "sqlite3-prepare", sqlite_prepare);

  const char* step_docstr = "Step through execution of prepared SQL statement";
  emacs_value sqlite_step = env->make_function(env, 1, 1, Fsqlite_step, step_docstr, NULL);
  bind_function(env, "sqlite3-step", sqlite_step);

  provide (env, "emacs-sqlite");


  /* loaded successfully */
  return 0;
}
