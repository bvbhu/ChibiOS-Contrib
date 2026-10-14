# Required platform files.
PLATFORMSRC_CONTRIB := $(CHIBIOS)/os/hal/ports/common/ARMCMx/nvic.c \
                       $(CHIBIOS_CONTRIB)/os/hal/ports/N32/N32H48x/n32_isr.c \
                       $(CHIBIOS_CONTRIB)/os/hal/ports/N32/N32H48x/hal_lld.c

# Required include directories.
PLATFORMINC_CONTRIB := $(CHIBIOS)/os/hal/ports/common/ARMCMx \
                       $(CHIBIOS_CONTRIB)/os/hal/ports/N32/N32H48x

# Optional platform files.
ifeq ($(USE_SMART_BUILD),yes)

# Configuration files directory
ifeq ($(HALCONFDIR),)
  ifeq ($(CONFDIR),)
    HALCONFDIR = .
  else
    HALCONFDIR := $(CONFDIR)
  endif
endif

HALCONF := $(strip $(shell cat $(HALCONFDIR)/halconf.h $(HALCONFDIR)/halconf_community.h | egrep -e "\#define"))
endif

# Drivers compatible with the platform.
include $(CHIBIOS_CONTRIB)/os/hal/ports/N32/LLD/GPIOv1/driver.mk
include $(CHIBIOS_CONTRIB)/os/hal/ports/N32/LLD/SYSTICKv1/driver.mk
include $(CHIBIOS_CONTRIB)/os/hal/ports/N32/LLD/ADCv1/driver.mk
include $(CHIBIOS_CONTRIB)/os/hal/ports/N32/LLD/USBOTGv1/driver.mk

# Shared variables
ALLCSRC += $(PLATFORMSRC_CONTRIB)
ALLINC  += $(PLATFORMINC_CONTRIB)
