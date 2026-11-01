library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_pwm is end entity;

architecture sim of tb_pwm is
  constant W : positive := 4;                  -- 4 bits -> one period = 16 clocks
  type int_array is array (natural range <>) of natural;
  constant TESTS : int_array := (0, 4, 8, 15); -- duty values to try
  signal clk  : std_logic := '0';
  signal done : std_logic := '0';
  signal rst_n, enable, pwm_out : std_logic := '0';
  signal duty : unsigned(W-1 downto 0) := (others => '0');
begin
  clk <= not clk after 5 ns when done = '0' else '0';   -- 100 MHz, stops at the end

  dut : entity work.pwm_core
    generic map (G_WIDTH => W)
    port map (clk => clk, rst_n => rst_n, enable => enable,
              duty => duty, pwm_out => pwm_out);

  process
    variable n : natural;
  begin
    wait for 20 ns;
    rst_n <= '1'; enable <= '1';
    for t in TESTS'range loop
      duty <= to_unsigned(TESTS(t), W);
      for i in 1 to 2**W loop wait until rising_edge(clk); end loop;  -- settle one period
      n := 0;
      for i in 1 to 2**W loop                                         -- count one period
        wait until rising_edge(clk);
        if pwm_out = '1' then n := n + 1; end if;
      end loop;
      assert n = TESTS(t)
        report "Duty error: duty=" & integer'image(TESTS(t)) &
               " high=" & integer'image(n) severity failure;
      report "duty=" & integer'image(TESTS(t)) & " OK: " &
             integer'image(n) & "/" & integer'image(2**W) & " cycles high";
    end loop;
    report "TEST PASSED";
    done <= '1';
    wait;
  end process;
end architecture;