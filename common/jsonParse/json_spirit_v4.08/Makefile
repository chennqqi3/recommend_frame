CPP			= g++

BUILD	   ?= debug
ifeq (debug, $(shell tr '[:upper:]' '[:lower:]' <<< $(BUILD)))
	override BUILD = debug
	CPPFLAGS = $(INCLUDE) -fPIC -g
else
	override BUILD = release
	CPPFLAGS = $(INCLUDE) -fPIC -O2
endif
INCLUDE		= -I json_spirit/
			 
LIBS		= -Llib/ 

LDFLAGS		= $(LIBS) 

GEN_OBJS	= json_spirit/json_spirit_reader.o json_spirit/json_spirit_value.o json_spirit/json_spirit_writer.o json_demo_test.o 
OBJS1		= $(GEN_OBJS) 
TARGET1		= json_demo_test 


ALL_TARGETS	= $(TARGET1)

all: $(ALL_TARGETS)

.cpp.o:
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(TARGET1): $(OBJS1)
	$(CPP) -o $@ $^ $(LDFLAGS)


clean:
	rm  -f json_spirit/*.o
