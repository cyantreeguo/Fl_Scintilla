SRCPATH = ../../FL/src/
FLTK = -I../../FL -I../../ -I/usr/include/freetype2 -lXft  -lfontconfig -lXinerama -lpthread -ldl -lm  -lX11 -lXext \
	$(SRCPATH)*.c $(SRCPATH)*.cxx ../../fltk/*.cxx ../../iconv/*.c ../../lexers/*.cxx ../../lexlib/*.cxx ../../src/*.cxx

GLPATH = ./FL/extra_gl/src/
FLTK_GL = -lGL -lGLU \
		$(GLPATH)*.cxx
all:
	g++ -o editor ../../test_editor.cxx $(FLTK) -I../../fltk -I../../iconv -I../../include -I../../src -I../../lexers -I../../lexlib -DSCI_LEXER -DSCI_NAMESPACE
#	$(FLTK_GL)
		
clean:
	rm -rf editor *.o
