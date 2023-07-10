CXXFLAGS	 =  -O2 -g -Wall -fmessage-length=0
SOURCES	    :=  $(shell find src -name '*.cpp')
OBJECTS	    :=  $(SOURCES:src/%.cpp=build/%.o)
DIRECTORIES :=  $(sort $(dir $(OBJECTS)))
LDFLAGS		 =  

INCLUDE_PATHS	= -I../../lib/common -I../../lib/parse -I../../lib/parse_ucs -I../../lib/parse_expression -I../../lib/parse_prs -I../../lib/parse_dot -I../../lib/ucs -I../../lib/boolean -I../../lib/prs -I../../lib/interpret_ucs -I../../lib/interpret_boolean -I../../lib/interpret_prs
LIBRARY_PATHS	= -L../../lib/common -L../../lib/parse -L../../lib/parse_ucs -L../../lib/parse_expression -L../../lib/parse_prs -L../../lib/parse_dot -L../../lib/ucs -L../../lib/boolean -L../../lib/prs -L../../lib/interpret_ucs -L../../lib/interpret_boolean -L../../lib/interpret_prs
LIBRARIES		= -lprs -linterpret_prs -linterpret_boolean -linterpret_ucs -lboolean -lucs -lparse_dot -lparse_prs -lparse_expression -lparse_ucs -lparse -lcommon
TARGET		 =  bubble

all: build $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LIBRARY_PATHS) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o $(TARGET) $(LIBRARIES)

build/%.o: src/%.cpp 
	$(CXX) $(INCLUDE_PATHS) $(CXXFLAGS) $(LDFLAGS) -c -o $@ $<
	
build:
	mkdir $(DIRECTORIES)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET).exe
