export TOP_DIR = $(shell pwd)
include Makefile.common
all: ccapi ui
#	$(MAKE) -C CCAPI
#	$(MAKE) -C UI

ui:
	$(MAKE) -C UI
	\cp $(UI_DIR)/UI/$(TARGET) ./

ccapi:
	$(MAKE) -C CCAPI

clean:
	$(MAKE) -C CCAPI clean
	$(MAKE) -C UI clean
	\rm -f $(TARGET)
