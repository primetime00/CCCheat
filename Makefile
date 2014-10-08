export TOP_DIR = $(shell pwd)
include Makefile.common
all: ccapi ui
#	$(MAKE) -C CCAPI
#	$(MAKE) -C UI

trainer:
	$(MAKE) -C Trainer

ui:
	$(MAKE) -C UI
	\mkdir -p bin
	\mv $(UI_DIR)/UI/$(TARGET) ./bin

ccapi:
	$(MAKE) -C CCAPI

clean:
	$(MAKE) -C CCAPI clean
	$(MAKE) -C UI clean
	\rm -f $(TARGET)
