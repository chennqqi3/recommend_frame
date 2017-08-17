CPP			= g++

BUILD	   ?= debug
ifeq (debug, $(shell tr '[:upper:]' '[:lower:]' <<< $(BUILD)))
	override BUILD = debug
	CPPFLAGS = $(INCLUDE) -fPIC -g
else
	override BUILD = release
	CPPFLAGS = $(INCLUDE) -fPIC -O2
endif

JSON_PATH	= json_spirit
INCLUDE		= -I json_spirit/ -Icommon/ -Ibusi/ -Icommon/ssdb-1.9.2/include -I./curl/include/curl -I./curl/include 
			 
LIBS		= -Llib/ common/ssdb-1.9.2/lib/libssdb-client.a libjson.a -lcrypto ./curl/libcurl.so

LDFLAGS		= $(LIBS) 

GEN_OBJS	= common/murmurhash2.o common/tools.o common/ssdb_wrapper.o common/log.o common/ini.o common/util.o common/md5.o $(JSON_PATH)/json_spirit_reader.o $(JSON_PATH)/json_spirit_value.o $(JSON_PATH)/json_spirit_writer.o
BUSI_OBJS	= busi/obtainData_Ssdb.o busi/obtainUrl_Http.o  busi/queryGenerator.o busi/urlMerge.o busi/unittest.o 
OBJS1		= $(GEN_OBJS) ${BUSI_OBJS} 
TARGET1		= unittest


ALL_TARGETS	= $(TARGET1)

all: $(ALL_TARGETS)

.cpp.o:
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(TARGET1): $(OBJS1)
	$(CPP) -o $@ $^ $(LDFLAGS)
	ar rcs libbusi.a common/*.o busi/*.o json_spirit/*.o 

.PHONY: install clean

install:
	@mkdir -p bin
	mv $(ALL_TARGETS) ./bin/
	cp third_party/re2/lib/libre2.so.0 ./bin/
	cp third_party/pcre/libpcre.so.0 ./bin/
	cp config/config.ini ./bin/
	cp config/tlds.dict ./bin/

clean:
	rm common/*.o busi/*.o libbusi.a
tar:
	ar rcs libbusi.a common/*.o busi/*.o json_spirit/*.o 
