CC			= gcc
CFLAGS		= -Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address -Iheaders -fsanitize=undefined
LDFLAGS		= -Llib -lds
TARGETS		= pe_exchange pe_trader $(LIB_DS)

VPATH		= src:src/data_structs:src/helper

OBJ_PATH	= obj

OBJ_DS		= $(addprefix $(OBJ_PATH)/, hash_map.o llist.o int_queue.o)
LIB_DS		= libds.a	# data structures
OBJ_MALLOC	= $(OBJ_PATH)/malloc.o
OBJ_E		= $(addprefix $(OBJ_PATH)/, parser.o pex_helper.o product_book.o)
OBJ_T		= $(addprefix $(OBJ_PATH)/, parser.o)

UT_PATH		= tests/ut
UT_TESTS	= $(addprefix $(UT_PATH)/, map_ut queue_ut product_book_ut)
E2E_PATH	= tests/e2e
E2E_TRADERS	= $(addprefix $(E2E_PATH)/, t_autotrader t_amend t_cancel t_basic0 t_basic1 t_complex0 t_complex1)
LDTEST		= -Llib -lcmocka-static -lds

all: $(TARGETS)

pe_exchange: $(OBJ_PATH)/pe_exchange.o $(OBJ_E) $(OBJ_MALLOC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 

pe_trader: $(OBJ_PATH)/pe_trader.o $(OBJ_T)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(LIB_DS) : $(OBJ_DS)
	ar rcs lib/$(LIB_DS) $(OBJ_DS)

obj/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@


tests: $(TARGETS) $(UT_TESTS) $(E2E_TRADERS)

$(UT_PATH)/map_ut: $(UT_PATH)/src/map_ut.c
	$(CC) $(CFLAGS) $< $(LDTEST) -o $@

$(UT_PATH)/queue_ut: $(UT_PATH)/src/queue_ut.c
	$(CC) $(CFLAGS) $< $(LDTEST) -o $@

$(UT_PATH)/product_book_ut: $(UT_PATH)/src/product_book_ut.c $(OBJ_PATH)/product_book.o $(OBJ_MALLOC)
	$(CC) $(CFLAGS) $^ $(LDTEST) -o $@

$(E2E_PATH)/t_% : $(E2E_PATH)/src/t_%.c $(OBJ_T)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

run_tests:
	@bash tests/test.sh

.PHONY: clean cleands
clean:
	rm -f $(TARGETS)
	rm -f $(OBJ_PATH)/*.o
	rm -f $(UT_PATH)/*ut
	rm -f $(E2E_PATH)/t_*
	rm -f $(E2E_PATH)/trader*

clean_ds:
	rm -f lib/$(LIB_DS)
