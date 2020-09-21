OUT			:=	../nitrofiles/sound

WAVFILES	:=	$(foreach file,$(notdir $(wildcard *.wav)),$(file))

MODFILES	:=	$(foreach file,$(notdir $(wildcard *.mod)),$(file))
S3MFILES	:=	$(foreach file,$(notdir $(wildcard *.s3m)),$(file))
ITFILES		:=	$(foreach file,$(notdir $(wildcard *.it)),$(file))
XMFILES		:=	$(foreach file,$(notdir $(wildcard *.xm)),$(file))

TARGETS		:= $(foreach file,$(MODFILES:.mod=.msl),$(addprefix $(OUT)/, $(file)))
TARGETS		+= $(foreach file,$(S3MFILES:.s3m=.msl),$(addprefix $(OUT)/, $(file)))
TARGETS		+= $(foreach file,$(ITFILES:.it=.msl),$(addprefix $(OUT)/, $(file)))
TARGETS		+= $(foreach file,$(XMFILES:.xm=.msl),$(addprefix $(OUT)/, $(file)))
TARGETS		+= $(OUT)/sfx.msl

.PHONY	:	all clean

all	:	$(OUT)/sfx.msl $(TARGETS)

#---------------------------------------------------------------------------------
# Rules to build soundbanks from music files
#---------------------------------------------------------------------------------
$(OUT)/%.msl	:	$(WAVFILES) %.mod
#---------------------------------------------------------------------------------
	@echo $$(basename $@)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@mmutil $^ -d -o$@ 1>/dev/null

#---------------------------------------------------------------------------------
$(OUT)/%.msl	:	$(WAVFILES) %.s3m
#---------------------------------------------------------------------------------
	@echo $$(basename $@)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@mmutil $^ -d -o$@ 1>/dev/null

#---------------------------------------------------------------------------------
$(OUT)/%.msl	:	$(WAVFILES) %.it
#---------------------------------------------------------------------------------
	@echo $$(basename $@)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@mmutil $^ -d -o$@ 1>/dev/null

#---------------------------------------------------------------------------------
$(OUT)/%.msl	:	$(WAVFILES) %.xm
#---------------------------------------------------------------------------------
	@echo $$(basename $@)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@mmutil $^ -d -o$@ 1>/dev/null

#---------------------------------------------------------------------------------
$(OUT)/%.msl	:	$(WAVFILES)
#---------------------------------------------------------------------------------
	@echo $$(basename $@)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	@mmutil $^ -d -o$@ 1>/dev/null

clean	:
	@echo clean ...
	@rm -f $(TARGETS)
