DIRS = AudioBase Plugins TestUnit AudioHost

BUILDDIRS = $(DIRS:%=build-%)
CLEANDIRS = $(DIRS:%=clean-%)

all: $(BUILDDIRS)
$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	@$(MAKE) -C $(@:build-%=%)

build-Plugins: build-AudioBase

clean: $(CLEANDIRS)
$(CLEANDIRS):
	@$(MAKE) -C $(@:clean-%=%) clean

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)

rebuild: clean all
