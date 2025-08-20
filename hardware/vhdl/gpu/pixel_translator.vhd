library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.custom_types.all;

entity pixel_translator is
  generic
  (
    FRAME_WIDTH  : natural := 320;
    FRAME_HEIGHT : natural := 180
  );
  port
  (
    clk             : in std_logic;
    reset           : in std_logic;
    translation     : in translation_t;
    go              : in std_logic;
    pixel_in        : in pixel_t;
    pixel_in_valid  : in std_logic;
    done_in         : in std_logic;
    pixel_out       : out pixel_t   := default_pixel;
    pixel_out_valid : out std_logic := '0';
    done_out        : out std_logic := '0'
  );
end entity pixel_translator;

architecture pixel_translator of pixel_translator is
  signal translation_reg : translation_t := default_translation;
  signal pixel_reg       : pixel_t       := default_pixel;
  signal pixel_valid_reg : std_logic     := '0';
  signal done_reg        : std_logic     := '0';
begin

  reg_proc : process (clk)
  begin
    if rising_edge(clk) then
      if reset = '1' then
        translation_reg <= default_translation;
        pixel_reg       <= default_pixel;
        pixel_valid_reg <= '0';
        done_reg        <= '0';
      else
        if go = '1' then
          translation_reg <= translation;
        end if;
        pixel_reg       <= pixel_in;
        pixel_valid_reg <= pixel_in_valid;
        done_reg        <= done_in;
      end if;
    end if;
  end process;

  translate_proc : process (clk)
    variable xt : integer;
    variable yt : integer;
  begin
    if rising_edge(clk) then
      xt := to_integer(pixel_reg.coord.x) + to_integer(translation_reg.x);
      yt := to_integer(pixel_reg.coord.y) + to_integer(translation_reg.y);

      if xt >= 0 and xt < FRAME_WIDTH and yt >= 0 and yt < FRAME_HEIGHT then
        pixel_out.coord.x <= to_unsigned(xt, pixel_out.coord.x'length);
        pixel_out.coord.y <= to_unsigned(yt, pixel_out.coord.y'length);
        pixel_out.color   <= pixel_reg.color;
        pixel_out_valid   <= pixel_valid_reg;
      else
        pixel_out_valid <= '0';
      end if;

      done_out <= done_reg;
    end if;
  end process;

end pixel_translator;