LEVEL = ../../..
LIBRARYNAME = phase0-gth773s
LOADABLE_MODULE = 1
USEDLIBS =

# If we don't need RTTI or EH, there's no reason to export anything
# from the project zero plugin.
ifneq ($(REQUIRES_RTTI), 1)
ifneq ($(REQUIRES_EH), 1)
EXPORTED_SYMBOL_FILE = $(PROJ_SRC_DIR)/ProjectZero.exports
endif
endif

include $(LEVEL)/Makefile.common
