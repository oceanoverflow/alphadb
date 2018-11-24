CXX     = g++
FLEX    = flex
BISON   = bison
FLAGS   = -std=c++11 -stdlib=libc++
PREFIX  = /usr/local
SOURCES = $(wildcard consensus/raft/*.cc) \
		  $(wildcard db/*.cc) \
		  $(wildcard index/b+tree/*.cc) \
		  $(wildcard io/*.cc) \
	  	  $(wildcard network/*.cc) \
		  $(wildcard network/rpc/*.cc) \
		  $(wildcard sql/*.cc) \
		  $(wildcard sql/parser/*.cc) \
		  $(wildcard sql/statement/*.cc) \
		  $(wildcard table/*.cc) \
		  $(wildcard transaction/*.cc) \
	 	  $(wildcard *.cc)
OBJECTS = $(SOURCES:.cc=.o)
PARSER  = $(wildcard sql/parser/*.y)
LEXER   = $(wildcard sql/parser/*.l)

bison_parser.cc: $(PARSER)
	$(BISON) $< -o=$@ -d=bison_parser.h -v

flex_lexer.cc: $(LEXER)
	$(FLEX) $(LEXER)

alphadb: flex_lexer.cc bison_parser.cc $(OBJECTS)
	$(CXX) $(FLAGS) -o $@ $^

.PHONY: clean install uninstall

install: alphadb
    mkdir -p $(PREFIX)/bin
    cp $< $(PREFIX)/bin/alphadb

uninstall:
    rm -f $(PREFIX)/bin/alphadb

clean:
	rm -rf *.o target