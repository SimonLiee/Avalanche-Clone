CC = clang

SOURCE_LIBS = -Ilib/

OSX_OPT = -Llib/ -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL lib/libraylib.a
OSX_OUT = bin/build_osx

CFILES = src/*.c

build_osx: 
	$(CC) $(CFILES) $(SOURCE_LIBS) -o "$(OSX_OUT)" $(OSX_OPT)

build_and_run_osx: build_osx
	./$(OSX_OUT)
