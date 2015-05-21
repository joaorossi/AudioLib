# Plugins subdirectories
DIRS = Filter GraphicEQ Delay Flanger

# Utilities
BUILDDIRS = $(DIRS:%=build-%)
CLEANDIRS = $(DIRS:%=clean-%)

all: $(BUILDDIRS)
$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	@$(MAKE) -C $(@:build-%=%)


clean: $(CLEANDIRS)
$(CLEANDIRS):
	@$(MAKE) -C $(@:clean-%=%) clean

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)

rebuild: clean all
