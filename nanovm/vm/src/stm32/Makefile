#
# Makefile include for NanoVM core files
#

STM32_DIR = $(ROOT_DIR)/vm/src/stm32
STM32_OBJS = \
  stm32/native_stm32.o \
  stm32/native_impl.o \
  


OBJS += $(STM32_OBJS)

# convert and upload a class file

stm32/%.o:$(STM32_DIR)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

stm32/%.d:$(STM32_DIR)/%.c
	set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's#\($*\)\.o[ :]*#\1.o $@ : #g' > $@ ; \
	[ -s $@ ] || rm -f $@
