# emacs-sqlite
SQLite3 Interface for Emacs 25.1

This repository provides the source to a Linux shared object to be loaded into Emacs.
This uses features introduced in the latest version (as of now!) and must be compiled against
the Emacs /src/ directory. Additionally, it will only work for versions of emacs compiled with the
`--with-modules` flag.


# Building
I have been lazy and left the Makefile configured for my filesystem. Change the ROOT in the Makefile to the emacs source directory. This additionally depends on the 'sqlite3' library being in the search path.