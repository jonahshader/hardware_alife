library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.custom_types.all;

entity line_renderer is
  port
  (
    clk         : in std_logic;
    reset       : in std_logic;
    go          : in std_logic;
    endpoint    : in frame_coord_t;
    color       : in color_t;
    pixel_out   : out pixel_t;
    pixel_valid : out std_logic;
    done        : out std_logic
  );
end entity line_renderer;

architecture line_renderer of line_renderer is
begin

  plotline_inst : entity work.plotline
    port map
    (
      clk       => clk,
      reset     => reset,
      go        => go,
      p0 => (x => to_unsigned(0, frame_pos_t'length), y => to_unsigned(0, frame_pos_t'length)),
      p1        => endpoint,
      donep     => '1',
      donep1    => done,
      goplot    => pixel_valid,
      color     => color,
      pixel_out => pixel_out
    );

end line_renderer;