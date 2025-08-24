-- Top-level wrapper for rw_fb for Verilator testing
-- Instantiates rw_fb with specific generics for testing

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.gpu_types.all;

entity rw_fb_test_top is
  port (
    -- display io
    disp_clk      : in std_logic;
    disp_read_pos : in fb_vec_t;
    disp_read_val : out rgb_t;

    -- gpu io
    gpu_clk       : in std_logic;
    swap          : in boolean; -- driven by clk_gpu domain
    gpu_read_pos  : in fb_vec_t;
    gpu_write_pos : in fb_vec_t;
    gpu_read_val  : out rgb_t;
    gpu_write_val : in rgb_t;
    gpu_write     : in boolean
  );
end entity rw_fb_test_top;

architecture wrapper of rw_fb_test_top is

begin

  -- Instantiate rw_fb with specific generics for testing
  u_rw_fb : entity work.rw_fb
    generic map (
      WIDTH        => 256,
      HEIGHT       => 256, 
      READ_LATENCY => 1
    )
    port map (
      disp_clk      => disp_clk,
      disp_read_pos => disp_read_pos,
      disp_read_val => disp_read_val,
      gpu_clk       => gpu_clk,
      swap          => swap,
      gpu_read_pos  => gpu_read_pos,
      gpu_write_pos => gpu_write_pos,
      gpu_read_val  => gpu_read_val,
      gpu_write_val => gpu_write_val,
      gpu_write     => gpu_write
    );

end architecture wrapper;