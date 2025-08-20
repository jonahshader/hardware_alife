library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.gpu_codes.all;
use work.custom_types.all;

-- gpu is the top level entity for the gpu. it takes in commands and outputs pixels.
-- only processes one command at a time. must wait for done to go high before issuing another command.
-- the gpu has five renderers: rect, circle, line, sprite, tile. here is how to use them:

-- rect:
--   renderer = rect
--   pos = top left corner of rectangle
--   size = width and height of rectangle
--   color = color of rectangle
--   enum = unused (TODO: maybe use for different rectangle types?)

-- circle:
--   renderer = circle
--   pos = center of circle
--   size.x = radius of circle
--   size.y = unused
--   color = color of circle
--   enum = unused (TODO: maybe use for different circle types?)

-- line:
--   renderer = line
--   pos = start of line
--   size = end of line
--   color = color of line
--   enum = unused (TODO: maybe use for different line types?)

-- sprite:
--   renderer = sprite
--   pos = screen position to draw sprite
--   size = width and height of sprite region to draw
--   color = unused (TODO: maybe use for sprite tint?)
--   enum = top left corner of sprite region to draw

-- tile:
--   renderer = tile
--   pos = top left corner of tile
--   size = unused
--   color = unused
--   enum = tile_id

entity gpu is
  port
  (
    -- gpu only runs one command at a time. wait for done to go high before issuing another command.
    clk         : in std_logic;
    reset       : in std_logic;
    instruction : in gpu_instruction_t;
    go          : in std_logic;
    pixel_out   : out pixel_t;
    pixel_valid : out std_logic;
    done        : out std_logic
  );
end entity gpu;

architecture gpu of gpu is
  type state_t is (idle, cmd_to_renderer, running);
  signal instruction_reg : gpu_instruction_t;

  signal pixel_out_preshift_reg   : pixel_t   := default_pixel;
  signal pixel_valid_preshift_reg : std_logic := '0';
  signal done_preshift_reg        : std_logic := '0';

  signal rect_go          : std_logic := '0';
  signal rect_pixel_out   : pixel_t;
  signal rect_pixel_valid : std_logic;
  signal rect_done        : std_logic;

  signal circle_go          : std_logic := '0';
  signal circle_pixel_out   : pixel_t;
  signal circle_pixel_valid : std_logic;
  signal circle_done        : std_logic;

  signal line_go          : std_logic := '0';
  signal line_pixel_out   : pixel_t;
  signal line_pixel_valid : std_logic;
  signal line_done        : std_logic;

  signal sprite_go          : std_logic := '0';
  signal sprite_pixel_out   : pixel_t;
  signal sprite_pixel_valid : std_logic;
  signal sprite_done        : std_logic;

  signal tile_go          : std_logic := '0';
  signal tile_pixel_out   : pixel_t;
  signal tile_pixel_valid : std_logic;
  signal tile_done        : std_logic;

begin

  gpu_proc : process (clk)
  begin
    if rising_edge(clk) then
      -- TODO: add reset (maybe)

      -- default go to low
      rect_go   <= '0';
      circle_go <= '0';
      line_go   <= '0';
      sprite_go <= '0';
      tile_go   <= '0';

      if go = '1' then
        instruction_reg <= instruction;

        -- make go high for the appropriate renderer
        case instruction.renderer is
          when rect =>
            rect_go <= '1';
          when circle =>
            circle_go <= '1';
          when line =>
            line_go <= '1';
          when sprite =>
            sprite_go <= '1';
          when tile =>
            tile_go <= '1';
          when others =>
            null;
        end case;
      end if;

      -- mux the outputs of the renderers
      case instruction_reg.renderer is
        when rect =>
          pixel_out_preshift_reg   <= rect_pixel_out;
          pixel_valid_preshift_reg <= rect_pixel_valid;
          done_preshift_reg        <= rect_done;
        when circle =>
          pixel_out_preshift_reg   <= circle_pixel_out;
          pixel_valid_preshift_reg <= circle_pixel_valid;
          done_preshift_reg        <= circle_done;
        when line =>
          pixel_out_preshift_reg   <= line_pixel_out;
          pixel_valid_preshift_reg <= line_pixel_valid;
          done_preshift_reg        <= line_done;
        when sprite =>
          pixel_out_preshift_reg   <= sprite_pixel_out;
          pixel_valid_preshift_reg <= sprite_pixel_valid;
          done_preshift_reg        <= sprite_done;
        when tile =>
          pixel_out_preshift_reg   <= tile_pixel_out;
          pixel_valid_preshift_reg <= tile_pixel_valid;
          done_preshift_reg        <= tile_done;
        when others =>
          pixel_out_preshift_reg   <= default_pixel;
          pixel_valid_preshift_reg <= '0';
          done_preshift_reg        <= '0';
      end case;
    end if;
  end process;

  pixel_translator_inst : entity work.pixel_translator
    port map
    (
      clk             => clk,
      reset           => reset,
      translation     => instruction.pos,
      go              => go,
      pixel_in        => pixel_out_preshift_reg,
      pixel_in_valid  => pixel_valid_preshift_reg,
      done_in         => done_preshift_reg,
      pixel_out       => pixel_out,
      pixel_out_valid => pixel_valid,
      done_out        => done
    );

  -- pixel_out <= pixel_out_preshift_reg;
  -- pixel_valid <= pixel_valid_preshift_reg;
  -- done <= done_preshift_reg;

  rect_renderer_inst : entity work.rect_renderer
    port
    map(
    clk         => clk,
    reset       => reset,
    go          => rect_go,
    size        => instruction_reg.size,
    color       => instruction_reg.color,
    pixel_out   => rect_pixel_out,
    pixel_valid => rect_pixel_valid,
    done        => rect_done
    );

  circle_renderer_inst : entity work.circle_renderer
    port
    map(
    clk         => clk,
    reset       => reset,
    go          => circle_go,
    radius      => instruction_reg.size.x,
    color       => instruction_reg.color,
    pixel_out   => circle_pixel_out,
    pixel_valid => circle_pixel_valid,
    done        => circle_done
    );

  line_renderer_inst : entity work.line_renderer
    port
    map(
    clk         => clk,
    reset       => reset,
    go          => line_go,
    endpoint    => instruction_reg.size,
    color       => instruction_reg.color,
    pixel_out   => line_pixel_out,
    pixel_valid => line_pixel_valid,
    done        => line_done
    );

  sprite_renderer_inst : entity work.sprite_renderer
    port
    map(
    clk         => clk,
    reset       => reset,
    go          => sprite_go,
    size        => instruction_reg.size,
    gpu_enum    => instruction_reg.enum,
    pixel_out   => sprite_pixel_out,
    pixel_valid => sprite_pixel_valid,
    done        => sprite_done
    );

  tile_renderer_inst : entity work.tile_renderer
    port
    map(
    clk         => clk,
    reset       => reset,
    go          => tile_go,
    tile_id     => unsigned(instruction_reg.enum(7 downto 0)),
    pixel_out   => tile_pixel_out,
    pixel_valid => tile_pixel_valid,
    done        => tile_done
    );

end gpu;