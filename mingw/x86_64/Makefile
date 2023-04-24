
PLAT = x86_64

OPT = -O2

# msys_version := $(if $(findstring Msys, $(shell uname -o)),$(word 1, $(subst ., ,$(shell uname -r))),0)

# UNAME := $(shell uname)

CXX = $(PLAT)-w64-mingw32-g++

# ifeq ($(msys_version),1)
#   $(error Error, unsupported version. Aborting. Upgrade to msys2)
# endif

# ifeq ($(msys_version),2)
#   $(info 1)
# else
#   $(info 0)
  WINRES = $(PLAT)-w64-mingw32-windres
  BIN_SRC = /usr/local/$(PLAT)-w64-mingw32/bin/
  INC_SRC = /usr/local/$(PLAT)-w64-mingw32/include/
  LIB_SRC = /usr/local/$(PLAT)-w64-mingw32/lib/

  # ifeq ($(UNAME), Linux)
  #   $(error Makefile supposed to be run on Windows in msys2-environment. Aborting.)
  # endif
# endif


# checking for msys2 with official snippet not possible in github-actions
# check if file exists, and substitute with msys2-path if needed
ifeq ("$(wildcard $($(PLAT)-w64-mingw32-windres))","")
  WINRES = /opt/bin/$(PLAT)-w64-mingw32-windres
endif

ifeq ("$(wildcard $(/usr/local/$(PLAT)-w64-mingw32/bin/))","")
  BIN_SRC = /mingw64/bin/
endif

ifeq ("$(wildcard $(/usr/local/$(PLAT)-w64-mingw32/include/))","")
  INC_SRC = /mingw64/include/

ifeq ("$(wildcard $(/usr/local/$(PLAT)-w64-mingw32/lib/))","")
  LIB_SRC = /mingw64/lib/


-include ../mingw.mk
