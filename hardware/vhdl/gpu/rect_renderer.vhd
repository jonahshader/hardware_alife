library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.gpu_types.all;

entity rect_renderer is
  generic (
    FB_READ_LATENCY : positive
  );
  port (
    clk : in std_logic;
    reset : in boolean;
    go : in boolean;

    instr : rect_instr_t
    pixel_in
  );
end entity;

architecture rtl of rect_renderer is

begin



end architecture;
