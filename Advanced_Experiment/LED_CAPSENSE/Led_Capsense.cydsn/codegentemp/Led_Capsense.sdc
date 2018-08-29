# THIS FILE IS AUTOMATICALLY GENERATED
# Project: E:\Psoc\Advanced_Experiment\LED_CAPSENSE\Led_Capsense.cydsn\Led_Capsense.cyprj
# Date: Fri, 13 Jul 2018 09:24:53 GMT
#set_units -time ns
create_clock -name {CyILO} -period 10000 -waveform {0 5000} [list [get_pins {ClockBlock/ilo}] [get_pins {ClockBlock/clk_100k}] [get_pins {ClockBlock/clk_1k}] [get_pins {ClockBlock/clk_32k}]]
create_clock -name {CyIMO} -period 41.666666666666664 -waveform {0 20.8333333333333} [list [get_pins {ClockBlock/imo}]]
create_clock -name {CyPLL_OUT} -period 15.625 -waveform {0 7.8125} [list [get_pins {ClockBlock/pllout}]]
create_clock -name {CyMASTER_CLK} -period 15.625 -waveform {0 7.8125} [list [get_pins {ClockBlock/clk_sync}]]
create_generated_clock -name {CyBUS_CLK} -source [get_pins {ClockBlock/clk_sync}] -edges {1 2 3} [list [get_pins {ClockBlock/clk_bus_glb}]]
create_generated_clock -name {clock} -source [get_pins {ClockBlock/clk_sync}] -edges {1 641 1281} [list [get_pins {ClockBlock/dclk_glb_1}]]
create_generated_clock -name {CapSense_IntClock} -source [get_pins {ClockBlock/clk_sync}] -edges {1 7 11} -nominal_period 83.333333333333329 [list [get_pins {ClockBlock/dclk_glb_0}]]


# Component constraints for E:\Psoc\Advanced_Experiment\LED_CAPSENSE\Led_Capsense.cydsn\TopDesign\TopDesign.cysch
# Project: E:\Psoc\Advanced_Experiment\LED_CAPSENSE\Led_Capsense.cydsn\Led_Capsense.cyprj
# Date: Fri, 13 Jul 2018 09:24:34 GMT