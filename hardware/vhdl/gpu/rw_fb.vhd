library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.gpu_types.all;

entity rw_fb is
  generic (
    WIDTH        : positive;
    HEIGHT       : positive;
    READ_LATENCY : positive
  );
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
end entity rw_fb;

architecture rtl of rw_fb is

  constant FB_PIXELS : positive := WIDTH * HEIGHT;
  constant FB_SIZE   : positive := FB_PIXELS * 2;
  type     fb_t is array (0 to FB_SIZE - 1) of rgb_t;
  type     read_shift_t is array (0 to READ_LATENCY - 1) of rgb_t;

  signal fb : fb_t := (others => default_rgb);

  signal gpu_read_delay : read_shift_t := (others => default_rgb);
  signal gpu_reading    : boolean      := false;

  signal disp_read_delay : read_shift_t := (others => default_rgb);
  signal disp1_reading   : boolean      := false;
  signal disp_reading    : boolean      := false;

  function get_fb_addr(pos : fb_vec_t; reading : boolean; current_buffer : boolean) return fb_addr_t is
    variable addr : fb_addr_t := (others => '0');
  begin
    addr := pos.x + pos.y * WIDTH;
    if reading = current_buffer then
      addr := addr + to_unsigned(FB_PIXELS, addr'length);
    end if;
    return addr;
  end function;

begin

  disp_read_val <= disp_read_delay(0);
  gpu_read_val  <= gpu_read_delay(0);

  gpu_proc : process (all) is
    variable read_addr  : fb_addr_t := get_fb_addr(gpu_read_pos, gpu_reading, true);
    variable write_addr : fb_addr_t := get_fb_addr(gpu_write_pos, gpu_reading, true);
  begin
    if rising_edge(gpu_clk) then
      -- toggle reading flag
      if swap then
        gpu_reading <= not gpu_reading;
      end if;

      -- read delay shift reg
      for i in 0 to READ_LATENCY - 2 loop
        gpu_read_delay(i) <= gpu_read_delay(i + 1);
      end loop;
      -- read into last stage of shift reg
      gpu_read_delay(gpu_read_delay'high) <= fb(to_integer(read_addr));

      -- write if enabled
      if gpu_write then
        fb(to_integer(write_addr)) <= gpu_write_val;
      end if;
    end if;
  end process;

  disp_proc : process (all) is
    variable read_addr : fb_addr_t := get_fb_addr(disp_read_pos, gpu_reading, false);
  begin
    if rising_edge(disp_clk) then
      -- reading_gpu is in the gpu domain so we must double register
      disp1_reading <= gpu_reading;
      disp_reading  <= disp1_reading;

      -- read delay shift reg
      for i in 0 to READ_LATENCY - 2 loop
        disp_read_delay(i) <= disp_read_delay(i + 1);
      end loop;
      -- read into last stage of shift reg
      disp_read_delay(disp_read_delay'high) <= fb(to_integer(read_addr));
    end if;
  end process;

end architecture rtl;
