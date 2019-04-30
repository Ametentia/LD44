C = clang
CXX = clang++

PROJECT := Ludum
PROJECT_UPPER := LUDUM
BUILD_DIR := ../build

COMPILER := -Wno-switch -Wno-undefined-internal -Wno-writable-strings
DEFINES  := -D$(UPPER_PROJECT)_INTERNAL -D$(UPPER_PROJECT)_SLOW

ifeq ($(OS), Windows_NT)
	PLATFORM  = Win
	LINKER    = /link /LIBPATH:..\libs\CSFML\lib csfml-audio.lib csfml-graphics.lib csfml-window.lib csfml-system.lib opengl32.lib kernel32.lib
	COMPILER += -imsvc ..\libs\CSFML\include /MD /O2 --driver-mode=cl /Fe$(PROJECT).exe
	DEFINES  += -D$(UPPER_PROJECT)_WIN
else
	SHELL  = /bin/bash
	UNAME := $(shell uname -s)
	ifeq ($(UNAME), Linux)
		PLATFORM  = Linux
		LINKER    = -lcsfml-audio -lcsfml-graphics -lcsfml-window -lcsfml-system -lGL -lSDL2
		COMPILER += -ggdb -O0 -isystem ../libs/CSFML/include -o $(PROJECT) 
		DEFINES  += -D$(UPPER_PROJECT)_LINUX
	else ifeq ($(UNAME), Darwin)
		# @Todo: macOS build
	endif
endif

.SILENT all: game

game:
	pushd "$(BUILD_DIR)" && $(CXX) $(COMPILER) $(DEFINES) ../code/$(PLATFORM)_$(PROJECT).cpp $(LINKER)
