# to edit:
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
INCLUDEPATH = -I$(MIXLIGHTLIBPATH) $(PYBIND11INCLUDEPATH) -I$(NUMPYINCLUDEPATH)
LINKINGLIBS = -lopenblas -llapack $(PYTHONRUNTIMELIB)
COMPILATIONFLAGS = -std=c++20 -O3 -fPIC -shared -fconcepts-diagnostics-depth=2 -fvisibility=hidden -MMD -MP

.PHONY: all $(MODULENAMES) setup clean

all: $(COMPILEDMODULES)

makedirs:
	mkdir -p $(SOURCE_DIR) $(BUILD_DIR) $(TARGET_DIR) $(OTHER_DIRS)

$(MODULENAMES): % : $(TARGET_DIR)%$(MODULEEXTENSION)

$(TARGET_DIR)%$(MODULEEXTENSION): $(SOURCE_DIR)%.cpp
	$(CXX) -o $@ $< $(COMPILATIONFLAGS) $(WARNINGS) $(LINKINGLIBS) $(INCLUDEPATH) -MF $(BUILD_DIR)$*.d

-include $(DEPENDENCIES)

setup: getsubmodules createvenv installlibs initrepo makedirs

getsubmodules:
	git submodule update --init --recursive
	git config submodule.recurse true

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