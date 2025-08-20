library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.custom_types.all;
use ieee.math_real.ceil;
use ieee.math_real.log2;

entity sprite_renderer is
  generic
  (
    SPRITESHEET_WIDTH_PX  : natural := 32;
    SPRITESHEET_HEIGHT_PX : natural := 128;
    -- SPRITESHEET_FILENAME : string := "..\..\src_hdl\rom\forest_tiles_fixed.rom"
    SPRITESHEET_FILENAME : string := ""
  );
  port
  (
    clk         : in std_logic;
    reset       : in std_logic;
    go          : in std_logic;
    size        : in frame_coord_t;
    gpu_enum    : in std_logic_vector(15 downto 0);
    pixel_out   : out pixel_t;
    pixel_valid : out std_logic;
    done        : out std_logic
  );
end entity sprite_renderer;

architecture sprite_renderer of sprite_renderer is
  constant SPRITESHEET_WIDTH_PX_LOG  : natural := integer(log2(real(SPRITESHEET_WIDTH_PX)));
  constant SPRITESHEET_HEIGHT_PX_LOG : natural := integer(log2(real(SPRITESHEET_HEIGHT_PX)));

  signal pixel_out_reg   : pixel_t   := default_pixel;
  signal pixel_valid_reg : std_logic := '0';
  signal done_reg        : std_logic := '0';

  signal x_start, y_start, x, y, x_delay, y_delay : frame_pos_t   := (others => '0');
  signal size_reg                                 : frame_coord_t := default_frame_coord;

  signal addr_b : unsigned(integer(ceil(log2(real(SPRITESHEET_WIDTH_PX * SPRITESHEET_HEIGHT_PX)))) - 1 downto 0) := (others => '0');
  signal dout_b : std_logic_vector(15 downto 0);

  type state_t is (idle, draw_sprite, last_pix);
  signal state_reg : state_t := idle;

begin
  pixel_out   <= pixel_out_reg;
  pixel_valid <= pixel_valid_reg;
  done        <= done_reg;

  spritesheet_prom : entity work.prom
    generic
    map(
    WIDTH     => 16,
    DEPTH     => SPRITESHEET_WIDTH_PX * SPRITESHEET_HEIGHT_PX,
    INIT_FILE => SPRITESHEET_FILENAME
    )
    port map
    (
      clk_b  => clk,
      en_b   => true,
      addr_b => addr_b,
      dout_b => dout_b
    );

  state_proc : process (clk)
    variable x_start_var : unsigned(7 downto 0);
    variable y_start_var : unsigned(7 downto 0);
  begin
    if rising_edge(clk) then
      if reset = '1' then
        state_reg       <= idle;
        x_start         <= (others => '0');
        y_start         <= (others => '0');
        x               <= (others => '0');
        y               <= (others => '0');
        x_delay         <= (others => '0');
        y_delay         <= (others => '0');
        pixel_out_reg   <= default_pixel;
        pixel_valid_reg <= '0';
        done_reg        <= '0';
      else
        case state_reg is
          when idle =>
            done_reg <= '0';
            if go = '1' then
              state_reg <= draw_sprite;
              x_start_var := unsigned(gpu_enum(7 downto 0));
              y_start_var := unsigned(gpu_enum(15 downto 8));
              x_start  <= resize(x_start_var, x_start'length);
              y_start  <= resize(y_start_var, y_start'length);
              size_reg <= size;

              addr_b <= resize(
                resize(x_start_var, addr_b'length) + shift_left(resize(y_start_var, addr_b'length), SPRITESHEET_WIDTH_PX_LOG),
                addr_b'length
                );
            end if;
          when draw_sprite =>
            -- navigate the sprite region
            if y = size_reg.y - 1 then
              if x = size_reg.x - 1 then
                -- we reached the end of the sprite. go to last_pix to allow pipeline to finish
                state_reg <= last_pix;
              else
                -- we aren't done with the row yet, so increment x
                x      <= x + 1;
                addr_b <= resize(
                  resize(x + x_start + 1, addr_b'length) + shift_left(resize(y + y_start, addr_b'length), SPRITESHEET_WIDTH_PX_LOG),
                  addr_b'length
                  );
              end if;
            else
              if x = size_reg.x - 1 then
                -- we finished the row, so reset x and increment y
                x      <= (others => '0');
                y      <= y + 1;
                addr_b <= resize(
                  resize(x + x_start, addr_b'length) + shift_left(resize(y + y_start + 1, addr_b'length), SPRITESHEET_WIDTH_PX_LOG),
                  addr_b'length
                  );
              else
                -- we aren't done with the row yet, so increment x
                x      <= x + 1;
                addr_b <= resize(
                  resize(x + x_start + 1, addr_b'length) + shift_left(resize(y + y_start, addr_b'length), SPRITESHEET_WIDTH_PX_LOG),
                  addr_b'length
                  );
              end if;
            end if;

            -- set pixel pos
            pixel_out_reg.coord.x <= x;
            pixel_out_reg.coord.y <= y;
            -- pixel only valid when opacity is non-zero
            pixel_valid_reg <= '0' when dout_b(15 downto 12) = "0000" else
              '1';
            -- set pixel color
            pixel_out_reg.color.r <= unsigned(dout_b(11 downto 8));
            pixel_out_reg.color.g <= unsigned(dout_b(7 downto 4));
            pixel_out_reg.color.b <= unsigned(dout_b(3 downto 0));
          when last_pix =>
            state_reg       <= idle;
            pixel_valid_reg <= '0';
            done_reg        <= '1';
          when others => null;
        end case;
      end if;
    end if;
  end process;

end sprite_renderer;