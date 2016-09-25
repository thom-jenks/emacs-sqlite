#ifndef EMACS_SYMBOLS_H
#define EMACS_SYMBOLS_H

#include <sqlite3.h>
#include <emacs-module.h>
#include "emacs-housekeeping.h"

void export_symbols(emacs_env* env){

  /* internalise open permissions */
  emacs_value sqlite_open_ro = env->make_integer(env, SQLITE_OPEN_READONLY);
  bind_value(env, "SQLITE_OPEN_READONLY", sqlite_open_ro);
  emacs_value sqlite_open_rw = env->make_integer(env, SQLITE_OPEN_READWRITE);
  bind_value(env, "SQLITE_OPEN_READWRITE", sqlite_open_rw);
  emacs_value sqlite_open_create = env->make_integer(env, SQLITE_OPEN_CREATE);
  bind_value(env, "SQLITE_OPEN_CREATE", sqlite_open_create);

  emacs_value val;
  val = env->make_integer(env, SQLITE_ABORT);
  bind_value(env, "SQLITE_ABORT", val);
  val = env->make_integer(env, SQLITE_AUTH);
  bind_value(env, "SQLITE_AUTH", val);
  val = env->make_integer(env, SQLITE_BUSY);
  bind_value(env, "SQLITE_BUSY", val);
  val = env->make_integer(env, SQLITE_CANTOPEN);
  bind_value(env, "SQLITE_CANTOPEN", val);
  val = env->make_integer(env, SQLITE_CONSTRAINT);
  bind_value(env, "SQLITE_CONSTRAINT", val);
  val = env->make_integer(env, SQLITE_CORRUPT);
  bind_value(env, "SQLITE_CORRUPT", val);
  val = env->make_integer(env, SQLITE_DONE);
  bind_value(env, "SQLITE_DONE", val);
  val = env->make_integer(env, SQLITE_EMPTY);
  bind_value(env, "SQLITE_EMPTY", val);
  val = env->make_integer(env, SQLITE_ERROR);
  bind_value(env, "SQLITE_ERROR", val);
  val = env->make_integer(env, SQLITE_FORMAT);
  bind_value(env, "SQLITE_FORMAT", val);
  val = env->make_integer(env, SQLITE_FULL);
  bind_value(env, "SQLITE_FULL", val);
  val = env->make_integer(env, SQLITE_INTERNAL);
  bind_value(env, "SQLITE_INTERNAL", val);
  val = env->make_integer(env, SQLITE_INTERRUPT);
  bind_value(env, "SQLITE_INTERRUPT", val);
  val = env->make_integer(env, SQLITE_IOERR);
  bind_value(env, "SQLITE_IOERR", val);
  val = env->make_integer(env, SQLITE_LOCKED);
  bind_value(env, "SQLITE_LOCKED", val);
  val = env->make_integer(env, SQLITE_MISMATCH);
  bind_value(env, "SQLITE_MISMATCH", val);
  val = env->make_integer(env, SQLITE_MISUSE);
  bind_value(env, "SQLITE_MISUSE", val);
  val = env->make_integer(env, SQLITE_NOLFS);
  bind_value(env, "SQLITE_NOLFS", val);
  val = env->make_integer(env, SQLITE_NOMEM);
  bind_value(env, "SQLITE_NOMEM", val);
  val = env->make_integer(env, SQLITE_NOTADB);
  bind_value(env, "SQLITE_NOTADB", val);
  val = env->make_integer(env, SQLITE_NOTFOUND);
  bind_value(env, "SQLITE_NOTFOUND", val);
  val = env->make_integer(env, SQLITE_NOTICE);
  bind_value(env, "SQLITE_NOTICE", val);
  val = env->make_integer(env, SQLITE_OK);
  bind_value(env, "SQLITE_OK", val);
  val = env->make_integer(env, SQLITE_PERM);
  bind_value(env, "SQLITE_PERM", val);
  val = env->make_integer(env, SQLITE_PROTOCOL);
  bind_value(env, "SQLITE_PROTOCOL", val);
  val = env->make_integer(env, SQLITE_RANGE);
  bind_value(env, "SQLITE_RANGE", val);
  val = env->make_integer(env, SQLITE_READONLY);
  bind_value(env, "SQLITE_READONLY", val);
  val = env->make_integer(env, SQLITE_ROW);
  bind_value(env, "SQLITE_ROW", val);
  val = env->make_integer(env, SQLITE_SCHEMA);
  bind_value(env, "SQLITE_SCHEMA", val);
  val = env->make_integer(env, SQLITE_TOOBIG);
  bind_value(env, "SQLITE_TOOBIG", val);
  val = env->make_integer(env, SQLITE_WARNING);
  bind_value(env, "SQLITE_WARNING", val);

  
}


#endif /* EMACS-SYMBOLS_H */
