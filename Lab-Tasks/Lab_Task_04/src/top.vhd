-- Lab 04 - top level: static MicroBlaze system (u_sys) + reconfigurable module (u_rm)
-- The block-design wrapper is NOT the top module in this lab; it is a component here.
-- Port names of design_microblaze_wrapper: check them against your generated wrapper
-- (Sources -> design_microblaze_wrapper.vhd) and adapt if they differ.
library ieee;
use ieee.std_logic_1164.all;

entity top is
  port (CLK100MHZ    : in  std_logic;                      -- E3, 100 MHz board clock
        CPU_RESETN   : in  std_logic;                      -- C12, active low
        UART_TXD_IN  : in  std_logic;                      -- C4, PC -> FPGA
        UART_RXD_OUT : out std_logic;                      -- D4, FPGA -> PC
        LED          : out std_logic_vector(15 downto 0)); -- LD0..LD15 = module output
end entity;

architecture rtl of top is

  -- static region: block design from Lab 03 (MicroBlaze, memory, UART, GPIO)
  component design_microblaze_wrapper is
    port (clk_in1_0      : in  std_logic;
          reset_rtl_0    : in  std_logic;
          uart_rtl_0_rxd : in  std_logic;
          uart_rtl_0_txd : out std_logic;
          rm_clk         : out std_logic;
          rm_din_tri_o   : out std_logic_vector(15 downto 0);
          rm_dout_tri_i  : in  std_logic_vector(15 downto 0));
  end component;

  -- reconfigurable partition: rm_a is the first module; rm_b and rm_c are
  -- added later in the DFX wizard (identical ports)
  component rm_a is
    port (clk  : in  std_logic;
          din  : in  std_logic_vector(15 downto 0);
          dout : out std_logic_vector(15 downto 0));
  end component;

  signal rm_clk : std_logic;
  signal din    : std_logic_vector(15 downto 0);
  signal dout   : std_logic_vector(15 downto 0);

begin

  u_sys : design_microblaze_wrapper
    port map (clk_in1_0      => CLK100MHZ,
              reset_rtl_0    => CPU_RESETN,
              uart_rtl_0_rxd => UART_TXD_IN,
              uart_rtl_0_txd => UART_RXD_OUT,
              rm_clk         => rm_clk,
              rm_din_tri_o   => din,
              rm_dout_tri_i  => dout);

  -- this instance becomes the reconfigurable partition (name u_rm is used by the Pblock)
  u_rm : rm_a
    port map (clk  => rm_clk,
              din  => din,
              dout => dout);

  LED <= dout;

end architecture;
