#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "emacs-module.h"

int plugin_is_GPL_compatible;


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


/* create symbols with values */
static void
bind_value (emacs_env *env, const char* name, emacs_value Svar){
  emacs_value Qsym = env->intern (env, name);
  emacs_value Qset = env->intern (env, "set");
  emacs_value args[] = { Qsym, Svar };

  env->funcall (env, Qset, 2, args);
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

int
emacs_module_init (struct emacs_runtime *ert)
{
  emacs_env *env = ert->get_environment (ert);

  /* internalise open permissions */
  emacs_value sqlite_open_ro = env->make_integer(env, SQLITE_OPEN_READONLY);
  bind_value(env, "SQLITE_OPEN_READONLY", sqlite_open_ro);
  emacs_value sqlite_open_rw = env->make_integer(env, SQLITE_OPEN_READWRITE);
  bind_value(env, "SQLITE_OPEN_READWRITE", sqlite_open_rw);
  emacs_value sqlite_open_create = env->make_integer(env, SQLITE_OPEN_CREATE);
  bind_value(env, "SQLITE_OPEN_CREATE", sqlite_open_create);

  const char* open_docstr = "Open SQLite3 DB file\nArguments: db_path permissions...\n"\
    "permissions are ORed together.";
  emacs_value sqlite_open = env->make_function(env, 1, 2, Fsqlite_open, open_docstr, NULL);
  bind_function(env, "sqlite3-open", sqlite_open);

  const char* close_docstr = "Close existing DB connection";
  emacs_value sqlite_close = env->make_function(env, 1, 1, Fsqlite_close, close_docstr, NULL);
  bind_function(env, "sqlite3-close", sqlite_close);

  provide (env, "emacs-sqlite");


  /* loaded successfully */
  return 0;
}
