# to edit:
#ARMAINCPATH = extern/armadillo/include/
#ENSMALLENCPATH = extern/ensmallen/include/
MIXLIGHTLIBPATH = extern/mixlightlib/
CXX = g++

# do not edit:
SHELL := /bin/bash
SOURCE_DIR := src/
BUILD_DIR := build/
TARGET_DIR := hubbleflow/
OTHER_DIRS := data/ figures/

SOURCES := $(wildcard $(SOURCE_DIR)*.cpp)
DEPENDENCIES := $(patsubst $(SOURCE_DIR)%.cpp, $(BUILD_DIR)%.d, $(SOURCES))
HEADERS := $(wildcard $(SOURCE_DIR)*.hpp) $(wildcard *.hpp)
MODULEEXTENSION := $(shell python-config --extension-suffix)
COMPILEDMODULES := $(patsubst $(SOURCE_DIR)%.cpp,$(TARGET_DIR)%$(MODULEEXTENSION),$(SOURCES))
MODULENAMES := $(patsubst $(SOURCE_DIR)%.cpp,%,$(SOURCES))


PYBIND11INCLUDEPATH = $(shell python -m pybind11 --includes)
NUMPYINCLUDEPATH = $(shell python -c "import numpy; print(numpy.get_include())")
PYTHONRUNTIMELIB := $(shell python-config --ldflags)

WARNINGS = -Wall -Wextra -Werror -pedantic -pedantic-errors -Wpedantic
INCLUDEPATH =  $(PYBIND11INCLUDEPATH) -I$(NUMPYINCLUDEPATH) -I$(MIXLIGHTLIBPATH)# -I$(ARMAINCPATH) -I$(ENSMALLENCPATH)
LINKINGLIBS = -lopenblas -llapack $(PYTHONRUNTIMELIB)
COMPILATIONFLAGS = -std=c++20 -O3 -fPIC -shared -fconcepts-diagnostics-depth=2 -fvisibility=hidden -MMD -MP

.PHONY: all $(MODULENAMES) setup clean

all: $(COMPILEDMODULES)

makedirs:
	mkdir -p $(SOURCE_DIR) $(BUILD_DIR) $(TARGET_DIR) $(OTHER_DIRS)

#main: main.cpp
#	$(CXX) -o main.x main.cpp -std=c++20 -O3 $(WARNINGS) -lopenblas -llapack -I$(ARMAINCPATH) -I$(ENSMALLENCPATH) -I$(MIXLIGHTLIBPATH)

$(MODULENAMES): % : $(TARGET_DIR)%$(MODULEEXTENSION)

$(TARGET_DIR)%$(MODULEEXTENSION): $(SOURCE_DIR)%.cpp
	$(CXX) -o $@ $< $(COMPILATIONFLAGS) $(WARNINGS) $(LINKINGLIBS) $(INCLUDEPATH) -MF $(BUILD_DIR)$*.d

-include $(DEPENDENCIES)

setup: createvenv installlibs initrepo makedirs

installlibs: createvenv
	. env/bin/activate && python -m pip install -r requirements.txt

createvenv:
	if [ ! -d env ]; then python -m venv env; fi

initrepo:
	git config filter.strip-notebook-output.clean 'jupyter nbconvert --ClearOutputPreprocessor.enabled=True --to=notebook --stdin --stdout --log-level=ERROR'
	git config filter.strip-notebook-output.smudge 'cat'
	git config filter.strip-notebook-output.required true

clean:
	rm -f main.x $(TARGET_DIR)*$(MODULEEXTENSION) $(BUILD_DIR)*