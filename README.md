# espmod

ESPMOD is a SystemC model of the ESP32 based boards from Espressif. It provieds
a way to check firmware intended for an ESP32 board without programming the
board.

# Why a model

**Easier to debug**
Well, the answer is, let's say you are writing firmware for a project, you
program the board and you come up with a bug. You know the board is not behaving
as you would expect. Then you are left with a question: what is going on.
If you had been writing code to run on the local machine you can add debug
statements, run a local debugger (i.e. gdb) and single step the code until you
get to the place, and see what is going on. But what do you do when the
firmware is not on the local CPU.

One way to solve this problem is to use a debugging port of a CPU. Some systems
have quite sophisticated mechanisms to monitor internal registers of a CPU,
set break points or watch points and even single step through systems. This
is a way to get a view into the system, but they are cumbersome to use, and
sometimes quite expensive. Using a model, there is no issue as the firmware
is running on the local machine, so standard debugging environments can be
used.

**Easier to test**
An additional benefit for models is for enhancing testing. Let's say you have
a FIFO or buffer and you wrote code to implement it. You need to test all the
cases of this FIFO to make sure the code behaves properly when the FIFO is
empty, full, near full, and perhaps there are some other cases. On a finished
product that can be quite tedious if not impossible. The alternative though is
to do nothing and hope it does not break on your client or professor. With
a model, instead, you can quickly setup an environment to test several cases
that would take weeks or months to check manually. You can also use a
sophisticated testing environment if you need that kind of quality.

**Less chance of damage**
A model also can protect you from damaging equipment. If you are writing to
the flash and you accidentally overwrite the bootloader, you might as well
throuw the chip away. A good model though can catch this bug and flag it so
that you repair it before you program it again.

**Start testing without a board**
An additiona reason to use a model is to test even if the board is not
available. If the board has not arrived yet or it will be modified, a model
helps you validate the concept before it has been prepared. Perhaps it will
save some money too.

**A warning**
Of course the model is not a copy of the world. It does not perfectly do
everything the real system will do. There are places where the model does
a simplification of the real world and there are places where there has not
been time to finish implementing parts of the work. Therefore this is not
a substitution to the physical testing nor a guarantee that the system will
work at the end. The model is intended only to help find bugs in firmware
and remove as many as possible before doing the physical testing.

**Viewing Waveforms**
For those that like to see waveforms, the model provides a way to do that.
Waves can be exported to VCD format and viewed in GTK. Unfortunatelly, only
the pins and top level signals can be viewed now. Later, hopefully this will
be expanded.

**Attaching to other models**
In addition, the model can be attached to other models and simulators.
These can be digital or analog simulated. Unfortunatelly, for free tools this
is still limited. If you have a sophisticated tool like Cadence, Mentor
Graphics, or Synopsys VCD, it is quite simple to be done. For free tools, it
is possible with some effort to connect NGSPICE to this model using the d_hdl
XSPICE module, but it is not easy to use. I hope to be able to improve this
soon.

# Why SystemC

SystemC is a whole simulator language similar to Verilog and VHDL intended to
model circuits. It is implemented as a set of C++ classes, so the environment
can be downloaded and installed on a number of systems with a C++ compiler.
Being it is intended for system modeling it already provides an entire
infrastructure for modeling wires, tracing waveforms, modeling time, writing
test cases. In addirion being SystemC it is in C++ it can simply be attached
to the C/C++ firmware. One more benefit is SystemC can be downloaded for free
from the Accelera website.

More information on SystemC: https://en.wikipedia.org/wiki/SystemC
Quick Tutorial, a bit out of date: http://www.asic-world.com/systemc/tutorial.html
Official Website: https://accellera.org/community/systemc

# Instalation

To install it is quite simple.

First download the latest version of SystemC and follow the steps in the
instructions to install it. It should be noted that you have to enable C++11
as it is required by the Arduino-IDF ESP32 libraries. For the version of GCC
SystemC will automatically install with C++11 disabled, so I had to force it
to switch it on by setting SC_CPLUSPLUS=201103L and adding the -std=c++11
to the CXXFLAGS.

Download and install the esp IDF and the Arduino IDF. The esp IDF can be inside
the Arduino directory. It should work with the latest versions but I used
model Arduino 1.8.9 and ESP-IDF github from July 8th.

Next download the ESPMOD and set the Makefile.vars and Makefile to point to
your SystemC, Arduino and ESP directories. This is because the ESPMOD attempts
to not override all files but only the ones that were adapted to work with the
tool. Once done just do:

 % make

It should produce two .a lib files. Once done, enter one of the examples
in the examples directory and and compile it also using make:

 % cd examples

 % cd Blink

 % make

And execute the model:

 % ./Blink.x

You should see the simulation run. You can optionally use the +waveform option
to get a VCD file and view it using gtkwaves or some other viewer.

# Status

There is still a lot to implement, as of yet the following has been done:

* GPIOs (several subfunctions are not ready yet)
* WiFi (AP mode, STA mode)
* Multiple ports, sockets communication
* Flash (user data only and one predefined partition)
* ADC1 and ADC2
* UART0 and UART2
* I2C (partially done, implemented via cchan)

The model has been tested successfully with the Arduino or ESP-IDF libraries:

* Adafruit RTClib
* TaskScheduler
* Nextion and NeoNextion
* TFT_eSPI (requires a new driver for the ESP model)
* WebServer
* ESP32Encoder
* NTPClient

# Limitations

The system is not perfect. A model will imitate the real world but it is not
a copy of it. Below are some of them.

 * For performance, the CPU has not been modeled. The firmware then is run
   directly on the local computer just wrapped in the simulation.
 * There is no real way to tell how long a piece of firmware will run for.
   So the only thing that will advance the simulation time is the delay()
   or del1cycle() functions.
   Usually, this is not a problem as in embedded systems, usually time is
   dominated by delay of the delay commands. Plus this does not hinder the
   main motivation in the model to help find and debug problems.
 * We do not have an SRAM model. All code is ran from inside the computer's
   SRAM. So the model will not tell you if you are going to fill up limited
   resources on very small CPUs. This perhaps can be improved later but the
   limitation is still there.
 * The real ESP uses lots of memory mapped I/O (I/O operations are initiated
   by the CPU reading or writing to a location, usually marked by the C
   volatile keyword). This can't be done in the model. Therefore, those cases
   were replaced with functions. Perhaps there is a better way to do them, but
   for now it is a limitation.
 * Some of the interfaces are not yet modeled, just for lack of time. For
   example the Flash QSPI, the I2C and the serial connected to the WiFi
   module. For now, these are represented using what I called a cchan
   interface. This is like a 8 bit wide UART interface that passes characters
   each time. Then messages are being passed telling the model what to do.
   This should be replaced later but for now it is there.
 * As said above, this is a model, it is not a copy of reality. Therefore there
   are things that will have to be checked at the top level.

# Future Development

**Work in Progress**
The project is far from ready. There is still lots to do. Several interfaces are
not yet written. Some need to be ported as well. Others modeled. So
contributions are welcome. Below are some big issues still missing:

 * Needing to put in a real I2C master/slave model
 * Needing to put in a QSPI Flash model. As of now the CCHAN works well so the
   regular esp_partition functions will work, but it could be better. We are
   also missing a model of the partition table, currently a set compiled-in
   table is used.
 * Needing to put in a SPI interface model
 * Needing to put in a bus model to emulate memory mapped I/O accesses.
 * Needing to build the PCNT
 * Model Interrupts. Still lots to do here.
 * And of course, if you model the system you have to model the test interface
     too. There are some in place, like a WiFi client, flash chip and other
     blocks, but there is still a lot to do here.
 * Documentation is lacking still. Lots to come here.

**Contributing:**

Please do so. But read up on the basic of SystemC. The model is not doing
any real fancy SystemC stuff, the basic is good enough.
