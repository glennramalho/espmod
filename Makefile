# Main Makefile to build static libraries for the ESPMOD.
#
# Copyright 2019 Glenn Ramalho - RFIDo Design
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

SYSTEMC_HOME=/opt/systemc/systemc-2.3.3
TARGET_ARCH=linux64
ARCH_SUFFIX=64-linux64
FLAGS_COMMON = -Wall -Wno-reorder -Wno-variadic-macros -Wno-parentheses \
   $(CPPSTANDARD) -fms-extensions -Wno-pedantic
FLAGS_STRICT = -Wno-long-long
FLAGS_WERROR =

SYSTEMC_INC_DIR=$(SYSTEMC_HOME)/include
SYSTEMC_LIB_DIR=$(SYSTEMC_HOME)/lib$(ARCH_SUFFIX)
SYSTEMC_CXXFLAGS=$(FLAGS_COMMON) $(FLAGS_STRICT) $(FLAGS_WERROR) -pthread
LDFLAG_RPATH=-Wl,-rpath=
SYSTEMC_LDFLAGS=-L $(SYSTEMC_LIB_DIR) $(LDFLAG_RPATH)$(SYSTEMC_LIB_DIR) \
   -lpthread
SYSTEMC_LIBS=-lsystemc -lm
LIBESPMOD=libespmod.a
LIBARDIDF=libardidfmod.a

OBJDIR := objs
DEPDIR := deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) -c
COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CPPFLAGS) -c

ESPMOD=src
include Makefile.vars
INCDIR=-I. $(INCLUDES)
DEFINES=-DESP32 -DSYSCMOD -DESP_PLATFORM
CFLAGS=-g $(INCDIR) $(DEFINES)
CPPFLAGS=$(CFLAGS) $(SYSTEMC_CXXFLAGS)

# Arduino ESP32 Core Files
CORES=$(CORESDIR)/Arduino.cpp $(CORESDIR)/HardwareSerial.cpp \
   $(CORESDIR)/Esp.cpp \
   $(ARDUINO)/cbuf.cpp \
   $(CORESDIR)/esp32-hal-adc.cpp \
   $(CORESDIR)/esp32-hal-log.cpp \
   $(CORESDIR)/esp32-hal-psram.cpp \
   $(CORESDIR)/esp32-hal-spi.cpp \
   $(CORESDIR)/esp32-hal-gpio.cpp \
   $(CORESDIR)/esp32-hal-ledc.c \
   $(CORESDIR)/esp32-hal-matrix.c \
   $(CORESDIR)/esp32-hal-uart.c \
   $(CORESDIR)/Print.cpp \
   $(CORESDIR)/Stream.cpp \
   $(ARDUINO)/StreamString.cpp \
   $(ARDUINO)/IPAddress.cpp \
   $(ARDUINO)/libb64/cencode.c \
   $(CORESDIR)/corebase64.cpp \
   $(CORESDIR)/WString.cpp $(CORESDIR)/stdlib_noniso.c

# Arduino Library Files
LIBRARIES=$(LIBDIR)/WebServer/WebServer.cpp \
   $(LIBDIR)/WebServer/detail/mimetable.cpp $(LIBDIR)/WebServer/Parsing.cpp \
   $(LIBDIR)/WiFi/WiFiAP.cpp $(LIBDIR)/WiFi/WiFi.cpp \
   $(LIBDIR)/WiFi/WiFiClient.cpp $(LIBDIR)/WiFi/WiFiServer.cpp \
   $(LIBDIR)/WiFi/WiFiGeneric.cpp $(LIBDIR)/WiFi/WiFiSTA.cpp \
   $(LIBDIR)/WiFi/WiFiUdp.cpp \
   $(ESPSDKDIR)/lwip/dns.cpp \
   $(LIBDIR)/Wire/Wire.cpp

# ESP IDF SDK Files
ESPSDK=$(ESPSDKDIR)/esp32/esp_system.cpp \
   $(ESPSDKDIR)/spi_flash/esp_spi_flash.cpp \
   $(ESPSDKDIR)/spi_flash/esp_partition.cpp \
   $(ESPSDKDIR)/http_parser.cpp \
   $(ESPSDKDIR)/esp-mqtt/mqtt_client.c \
   $(ESPSDKDIR)/esp-mqtt/lib/mqtt_msg.c \
   $(ESPSDKDIR)/esp-mqtt/lib/mqtt_outbox.c \
   $(ESPSDKDIR)/esp-mqtt/lib/platform_esp32_idf.c \
   $(ESPSDKDIR)/tcp_transport/esp_transport.c \
   $(ESPSDKDIR)/tcp_transport/esp_transport_tcp.c \
   $(ESPSDKDIR)/tcp_transport/esp_transport_utils.c \
   $(ESPSDKDIR)/mbedtls/library/mbedbase64.c \
   $(ESPSDKDIR)/mbedtls/library/sha1.c \
   $(ESPSDKDIR)/mbedtls/library/platform_util.c \
   $(ESPSDKDIR)/soc/gpio_periph.c \
   $(ESPSDKDIR)/freertos/task.cpp $(ESPSDKDIR)/freertos/semphr.cpp \
   $(ESPSDKDIR)/ledc.cpp $(ESPSDKDIR)/driver/pcnt.cpp \
   $(ESPSDKDIR)/driver/gpio.cpp $(ESPSDKDIR)/esp32/rom/romgpio.cpp \
   $(ESPSDKDIR)/lwip/sockets.cpp $(ESPSDKDIR)/driver/adc.cpp \
   $(ESPSDKDIR)/esp32/esp_wifi.cpp $(ESPSDKDIR)/esp32/esp_wifi.cpp \
   $(ESPSDKDIR)/tcpip_adapter.cpp $(ESPSDKDIR)/simnetdb.c \
   $(ESPSDKDIR)/esp32/panic.cpp $(ESPSDKDIR)/esp32/reset_reason.cpp \
   $(ESPSDKDIR)/esp32/intr_alloc.cpp \
   $(ESPSDKDIR)/esp32/rom/ets_sys.cpp \
   $(ESPSDKDIR)/esp32/rom/romrtc.cpp \
   $(ESPSDKDIR)/nvs_flash/src/nvs_api.cpp \
   $(ESPSDKDIR)/nvs_flash/src/nvs_item_hash_list.cpp \
   $(ESPSDKDIR)/nvs_flash/src/nvs_ops.cpp \
   $(ESPSDKDIR)/nvs_flash/src/nvs_page.cpp \
   $(ESPSDKDIR)/nvs_flash/src/nvs_pagemanager.cpp \
   $(ESPSDKDIR)/nvs_flash/src/nvs_storage.cpp \
   $(ESPSDKDIR)/nvs_flash/src/nvs_types.cpp \
   $(ESPSDKDIR)/nvs_flash/test_nvs_host/crc.cpp

# Generic C++ to SystemC interface support files
INTF=$(INTFDIR)/gpioset.cpp $(INTFDIR)/crccalc.cpp \
   $(INTFDIR)/TestSerial.cpp $(INTFDIR)/hfieldlist.cpp \
   $(INTFDIR)/pins_arduino.c $(INTFDIR)/adc_types.cpp $(INTFDIR)/update.cpp \
   $(INTFDIR)/clockpacer.cpp

# SystemC Module Files
MODULES=$(MODDIR)/cchan.cpp $(MODDIR)/cchanflash.cpp \
   $(MODDIR)/esp32adc1.cpp $(MODDIR)/esp32adc2.cpp \
   $(MODDIR)/doitesp32devkitv1.cpp $(MODDIR)/gn_pullupdn.cpp \
   $(MODDIR)/gn_mixed.cpp $(MODDIR)/ea_pullup.cpp $(MODDIR)/io_mux.cpp \
   $(MODDIR)/gpio_matrix.cpp $(MODDIR)/mux_pcnt.cpp $(MODDIR)/mux_in.cpp \
   $(MODDIR)/mux_out.cpp $(MODDIR)/pcntmod.cpp $(MODDIR)/clkgen.cpp \
   $(MODDIR)/ledcmod.cpp $(MODDIR)/netcon.cpp $(MODDIR)/uart.cpp \
   $(MODDIR)/spimod.cpp $(MODDIR)/espintr.cpp

# Test Interface Modules
TBMODULES=$(TBINTF)/tft.cpp $(TBINTF)/webclient.cpp $(TBINTF)/uartclient.cpp \
   $(TBINTF)/tpencoder.cpp $(TBINTF)/encoder.cpp $(TBINTF)/st7735.cpp

# We join the files into two sets of libraries. One with the Arduino IDF files
# and one with the rest.

# Arduino-IDF cores
ASRCS=$(CORES) $(LIBRARIES)
ASRCC=$(ASRCS:%.c=%.o)
ADEFC=$(ASRCS:%.c=%.d)
AOBJS=$(foreach a,$(notdir $(ASRCC:%.cpp=%.o)),$(OBJDIR)/$a)
ADEFS=$(foreach a,$(notdir $(ADEFC:%.cpp=%.d)),$(DEPDIR)/$a)

# ESPMOD files
ESRCS=$(ESPSDK) $(INTF) $(MODULES) $(TBMODULES)
EOBJC=$(ESRCS:%.c=%.o)
EDEFC=$(ESRCS:%.c=%.d)
EOBJS=$(foreach a,$(notdir $(EOBJC:%.cpp=%.o)),$(OBJDIR)/$a)
EDEFS=$(foreach a,$(notdir $(EDEFC:%.cpp=%.d)),$(DEPDIR)/$a)

# We join all dependencies into one set
DEPFILES=$(ADEFS) $(EDEFS)

################################################################################
# The VPATH needs to include all directories
################################################################################
# We could split up the file into multiple ones to break this up better or
# use some other trick, but for now this works.
VPATH=\
   $(CORESDIR):$(INTFDIR):$(LIBDIR)/WebServer:$(LIBDIR)/WebServer/detail:\
   $(LIBDIR)/WiFi:$(LIBDIR)/Wire:$(LIBDIR)/SPI:$(MODDIR):$(TBINTF):\
   $(ESPSDKDIR):$(ESPSDKDIR)/freertos:$(ESPSDKDIR)/esp32:$(ESPSDKDIR)/soc:\
   $(ESPSDKDIR)/tcp_transport:$(ESPSDKDIR)/spi_flash:\
   $(ESPSDKDIR)/nvs_flash/src:$(ESPSDKDIR)/nvs_flash/test_nvs_host:\
   $(ESPSDKDIR)/esp-mqtt:$(ESPSDKDIR)/esp-mqtt/lib:\
   $(ESPSDKDIR)/driver:$(ESPSDKDIR)/mbedtls/library:\
   $(ESPSDKDIR)/esp32/rom:$(ESPSDKDIR)/lwip:$(ARDUINO):$(ARDUINO)/libb64

################################################################################
# Targets
################################################################################

all: $(LIBARDIDF) $(LIBESPMOD)

# Library Files
$(LIBARDIDF): $(AOBJS)
	ar rcs $@ $(AOBJS)

$(LIBESPMOD): $(EOBJS)
	ar rcs $@ $(EOBJS)

# Build Rules
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d | $(DEPDIR) $(OBJDIR)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<
$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d | $(DEPDIR) $(OBJDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

# Object and dependency Directories
$(OBJDIR):
	mkdir -p $@
$(DEPDIR):
	mkdir -p $@

# Dependency rules
$(DEPFILES):
include $(wildcard $(DEPFILES))

# Cleanup rule
clean:
	rm -rf $(OBJDIR) $(DEPDIR)
	rm -f $(LIBARDIDF) $(LIBESPMOD)

%: RCS/%,v
