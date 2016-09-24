#include <vector>
#include <string>
#include <fstream>

extern "C"{
#include <emacs-module.h>
}

int plugin_is_GPL_compatible;
static int counter = 1;


std::vector<std::string> split(char delimiter, std::string value){
  std::vector<std::string> result;
  std::string current;
  for(const auto c: value){
    if(c == delimiter){
      result.push_back(current);
      current = "";
      continue;
    }
    current += c;
  }
  return result;
}

emacs_value* vector_to_values(emacs_env *env, std::vector<std::string> arguments){
  emacs_value* result = new emacs_value[arguments.size()];
  
  for(unsigned int i = 0; i < arguments.size(); i++){
    result[i] = env->make_string(env, arguments[i].c_str(), arguments[i].size());
  }
  return result;
}

void log(std::string word){
  std::ofstream of;
  of.open("/home/thom/ac.log", std::ios::out | std::ios::app);
  of << word << std::endl;
}

static emacs_value
Fac_static_test(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * data){
  counter++;
  return env->make_integer(env, counter);
}

static emacs_value
Fsqlite_query(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void * data){

  //return env->funcall(env, env->intern(env, "list"), nargs, args);
  char buffer[256];
  ptrdiff_t size;
  
  if(!env->copy_string_contents(env, args[0], buffer, &size)){
    return env->make_integer(env, -1);
  }
  else{
    std::string word = std::string(buffer);
    return env->make_integer(env, 1);
  }
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
  emacs_value inc = env->make_function (env,
					0,            /* min. number of arguments */
					0,            /* max. number of arguments */
					Fsqlite_query,  /* actual function pointer */
					"Provides possible completions for word from english dictionary",        /* docstring */
					NULL          /* user pointer of your choice (data param in Fmymod_test) */
					);
				       

  bind_function (env, "ac-word", fun);
  bind_function (env, "ac-inc", inc);
  provide (env, "emacs-sql");

  /* loaded successfully */
  return 0;
}
}
