
# Nexys A7-100T - 100 MHz Clock
set_property PACKAGE_PIN E3 [get_ports {clk_in1_0}]
set_property IOSTANDARD LVCMOS33 [get_ports {clk_in1_0}]

# CPU Reset Button (active low)
set_property PACKAGE_PIN C12 [get_ports {reset_rtl_0}]
set_property IOSTANDARD LVCMOS33 [get_ports {reset_rtl_0}]

# UART RX - Data received from PC
set_property PACKAGE_PIN C4 [get_ports {uart_rtl_0_rxd}]
set_property IOSTANDARD LVCMOS33 [get_ports {uart_rtl_0_rxd}]

# UART TX - Data sent to PC
set_property PACKAGE_PIN D4 [get_ports {uart_rtl_0_txd}]
set_property IOSTANDARD LVCMOS33 [get_ports {uart_rtl_0_txd}]

# 100 MHz primary clock (10 ns period)
create_clock -name sys_clk_pin -period 10.000 [get_ports {clk_in1_0}]
