--Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
--Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
----------------------------------------------------------------------------------
--Tool Version: Vivado v.2025.1 (win64) Build 6140274 Thu May 22 00:12:29 MDT 2025
--Date        : Thu Oct 29 12:00:37 2026
--Host        : LAB50-02 running 64-bit major release  (build 9200)
--Command     : generate_target design_microblaze_wrapper.bd
--Design      : design_microblaze_wrapper
--Purpose     : IP block netlist
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity design_microblaze_wrapper is
  port (
    clk_in1_0 : in STD_LOGIC;
    reset_rtl_0 : in STD_LOGIC;
    uart_rtl_0_rxd : in STD_LOGIC;
    uart_rtl_0_txd : out STD_LOGIC
  );
end design_microblaze_wrapper;

architecture STRUCTURE of design_microblaze_wrapper is
  component design_microblaze is
  port (
    uart_rtl_0_rxd : in STD_LOGIC;
    uart_rtl_0_txd : out STD_LOGIC;
    reset_rtl_0 : in STD_LOGIC;
    clk_in1_0 : in STD_LOGIC
  );
  end component design_microblaze;
begin
design_microblaze_i: component design_microblaze
     port map (
      clk_in1_0 => clk_in1_0,
      reset_rtl_0 => reset_rtl_0,
      uart_rtl_0_rxd => uart_rtl_0_rxd,
      uart_rtl_0_txd => uart_rtl_0_txd
    );
end STRUCTURE;
