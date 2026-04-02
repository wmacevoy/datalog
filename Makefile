CC = cc
CFLAGS = -O2 -std=c11 -fPIC -Inative -Ivendor/qjson/native -Ivendor/qjson/native/libbf

# QJSON sources (vendored)
QJSON_SRC = vendor/qjson/native/qjson.c \
            vendor/qjson/native/qjson_lex.c \
            vendor/qjson/native/qjson_parse.c \
            vendor/qjson/native/libbf/libbf.c \
            vendor/qjson/native/libbf/cutils.c
QJSON_FLAGS = -DQJSON_USE_LIBBF

# Datalog sources
DATALOG_SRC = native/datalog.c native/datalog_parse.c

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
  SHARED = -dynamiclib -undefined dynamic_lookup
  EXT_SUFFIX = .dylib
else
  SHARED = -shared
  EXT_SUFFIX = .so
endif

.PHONY: all clean test family pump vending

all: libdatalog.a

# Static library
libdatalog.a: $(DATALOG_SRC) $(QJSON_SRC)
	$(CC) $(CFLAGS) $(QJSON_FLAGS) -c $(DATALOG_SRC) $(QJSON_SRC)
	ar rcs $@ *.o
	rm -f *.o

# Tests
test: test_datalog
	./test_datalog

test_datalog: test/test_datalog.c $(DATALOG_SRC) $(QJSON_SRC)
	$(CC) $(CFLAGS) $(QJSON_FLAGS) -o $@ $^ -lm

# Examples
family: examples/family/family.dl $(DATALOG_SRC) $(QJSON_SRC)
	@echo "=== Family tree ==="
	# TODO: datalog CLI runner

pump: examples/pump/pump.dl $(DATALOG_SRC) $(QJSON_SRC)
	@echo "=== Insulin pump ==="

vending: examples/vending/vending.dl $(DATALOG_SRC) $(QJSON_SRC)
	@echo "=== Vending machine ==="

clean:
	rm -f libdatalog.a test_datalog *.o
