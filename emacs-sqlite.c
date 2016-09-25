#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "emacs-module.h"

int plugin_is_GPL_compatible;

static void
get_int_list(emacs_env* env, emacs_value elist, int* clist, int* len){
  //emacs_value type = env->type_of(env, elist);
  /* TODO: error if type is not list */

  if((*len) == 0){
    emacs_value length = env->funcall(env, env->intern(env, "length"), 1, &elist);
    (*len) = env->extract_integer(env, length);
    return;
  }
  else{
    int i;
    for(i = 0; i < *len; i++){
      emacs_value args[] = { env->make_integer(env, 2), elist };
      emacs_value val = env->funcall(env, env->intern(env, "nth"), 2, args);
      clist[i] = env->extract_integer(env, val);
    }
  }
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
  if(nargs > 1){
    int permissions_len = 0;
    get_int_list(env, args[1], NULL, &permissions_len);
    int* permissions_list = malloc(permissions_len*sizeof(int));
    int i = 0;
    for(i = 0; i < permissions_len; i++){
      sqlite_flags |= permissions_list[i];
    }
    free(permissions_list);
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
    emacs_value data = env->make_string(env, message, strlen(message));
    env->non_local_exit_signal(env, symbol, data);
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
  
  emacs_value sqlite_open = env->make_function(env, 1, 2, Fsqlite_open, "Open sqlite3 db file", NULL);
  bind_function(env, "sqlite3-open", sqlite_open);
 
  provide (env, "emacs-sqlite");


  /* loaded successfully */
  return 0;
}
