#ifndef EMACS_HOUSEKEEPING_H
#define EMACS_HOUSEKEEPING_H

#include <emacs-module.h>

/* create symbols with values */
void
bind_value (emacs_env *env, const char* name, emacs_value Svar){
  emacs_value Qsym = env->intern (env, name);
  emacs_value Qset = env->intern (env, "set");
  emacs_value args[] = { Qsym, Svar };

  env->funcall (env, Qset, 2, args);
}


void bind_function(emacs_env *env, const char* name, emacs_value Sfun)
{
  /* Convert the strings to symbols by interning them */
  emacs_value Qfset = env->intern (env, "fset");
  emacs_value Qsym = env->intern (env, name);

  /* Prepare the arguments array */
  emacs_value args[] = { Qsym, Sfun };

  /* Make the call (2 == nb of arguments) */
  env->funcall (env, Qfset, 2, args);

}

/* Provide FEATURE to Emacs.  */
void
provide (emacs_env *env, const char *feature)
{
  /* call 'provide' with FEATURE converted to a symbol */

  emacs_value Qfeat = env->intern (env, feature);
  emacs_value Qprovide = env->intern (env, "provide");
  emacs_value args[] = { Qfeat };

  env->funcall (env, Qprovide, 1, args);
}


#endif /* EMACS-HOUSEKEEPING_H */
