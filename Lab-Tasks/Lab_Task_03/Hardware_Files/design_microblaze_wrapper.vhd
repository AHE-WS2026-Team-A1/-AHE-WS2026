--Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
--Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
----------------------------------------------------------------------------------
--Tool Version: Vivado v.2025.2 (win64) Build 6299465 Fri Nov 14 19:35:11 GMT 2025
--Date        : Thu Nov  5 11:19:00 2026
--Host        : Jubair running 64-bit major release  (build 9200)
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
    an_tri_o : out STD_LOGIC_VECTOR ( 7 downto 0 );
    clk_in1_0 : in STD_LOGIC;
    led_tri_o : out STD_LOGIC_VECTOR ( 15 downto 0 );
    pwm_b : out STD_LOGIC;
    pwm_g : out STD_LOGIC;
    pwm_r : out STD_LOGIC;
    reset_rtl_0 : in STD_LOGIC;
    seg_tri_o : out STD_LOGIC_VECTOR ( 7 downto 0 );
    sw_tri_i : in STD_LOGIC_VECTOR ( 15 downto 0 );
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
    clk_in1_0 : in STD_LOGIC;
    sw_tri_i : in STD_LOGIC_VECTOR ( 15 downto 0 );
    led_tri_o : out STD_LOGIC_VECTOR ( 15 downto 0 );
    seg_tri_o : out STD_LOGIC_VECTOR ( 7 downto 0 );
    an_tri_o : out STD_LOGIC_VECTOR ( 7 downto 0 );
    pwm_g : out STD_LOGIC;
    pwm_r : out STD_LOGIC;
    pwm_b : out STD_LOGIC
  );
  end component design_microblaze;
begin
design_microblaze_i: component design_microblaze
     port map (
      an_tri_o(7 downto 0) => an_tri_o(7 downto 0),
      clk_in1_0 => clk_in1_0,
      led_tri_o(15 downto 0) => led_tri_o(15 downto 0),
      pwm_b => pwm_b,
      pwm_g => pwm_g,
      pwm_r => pwm_r,
      reset_rtl_0 => reset_rtl_0,
      seg_tri_o(7 downto 0) => seg_tri_o(7 downto 0),
      sw_tri_i(15 downto 0) => sw_tri_i(15 downto 0),
      uart_rtl_0_rxd => uart_rtl_0_rxd,
      uart_rtl_0_txd => uart_rtl_0_txd
    );
end STRUCTURE;