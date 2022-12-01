
CONTIKI_PROJECT = client_comm
all: $(CONTIKI_PROJECT)

WERROR := 1

CFLAGS += -O3

CUSTOM_CFLAGS := -Wall
CUSTOM_CFLAGS += -Wformat
CUSTOM_CFLAGS += -std=c11
CUSTOM_CFLAGS += -Wno-uninitialized

define CUSTOM_RULE_C_TO_O
endef

define CUSTOM_RULE_CPP_TO_O
endef

define CUSTOM_RULE_C_TO_S
endef

define CUSTOM_RULE_C_TO_E
endef

define CUSTOM_RULE_CPP_TO_E
endef



CONTIKI = /home/vagrant/contiki-ng

MAKE_NET = MAKE_NET_NULLNET

include $(CONTIKI)/Makefile.include



CUSTOM_CFLAGS += $(filter-out -Wno-unused-const-variable,$(CFLAGS))

%.o: %.c
	@echo "Using custom rule for project .c files"
	$(TRACE_CC)
	$(Q)$(CC) $(CUSTOM_CFLAGS) -c $< -o $@


%.o: %.cpp
	@echo "Using custom rule for project .cpp files"
	$(TRACE_CXX)
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@


%.s: %.c
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) -S $< -o $@


%.e: %.c
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) -E $< -o $@


%.e: %.cpp
	$(TRACE_CXX)
	$(Q)$(CXX) $(CXXFLAGS) -E $< -o $@



# CONTIKI_PROJECT = DCT
# all: $(CONTIKI_PROJECT)


# TARGET_LIBFILES = -lm
# CONTIKI = ../..
# include $(CONTIKI)/Makefile.include
