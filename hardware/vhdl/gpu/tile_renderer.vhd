library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.custom_types.all;
use ieee.math_real.ceil;
use ieee.math_real.log2;

entity tile_renderer is
  generic
  (
    -- these all must be powers of 2
    TILE_WIDTH_PX         : natural := 16;
    TILE_HEIGHT_PX        : natural := 16;
    SPRITESHEET_WIDTH_PX  : natural := 32;
    SPRITESHEET_HEIGHT_PX : natural := 128;
    -- SPRITESHEET_FILENAME : string := "..\..\src_hdl\rom\forest_tiles_fixed.rom"
    -- SPRITESHEET_FILENAME : string := "F:/Documents/Vivado/nexys_a7/ece5710/vampire_survivors_vhdl/src_hdl/rom/forest_tiles_fixed.rom"
    SPRITESHEET_FILENAME : string := "../../src_hdl/rom/forest_tiles_fixed.rom"
  );
  port
  (
    clk         : in std_logic;
    reset       : in std_logic;
    go          : in std_logic;
    tile_id     : in unsigned(7 downto 0);
    pixel_out   : out pixel_t;
    pixel_valid : out std_logic;
    done        : out std_logic
  );
end entity tile_renderer;

architecture tile_renderer of tile_renderer is
  constant SPRITESHEET_WIDTH_TILES     : natural := integer(SPRITESHEET_WIDTH_PX / TILE_WIDTH_PX);
  constant SPRITESHEET_HEIGHT_TILES    : natural := integer(SPRITESHEET_HEIGHT_PX / TILE_HEIGHT_PX);
  constant SPRITESHEET_WIDTH_TILES_LOG : natural := integer(log2(real(SPRITESHEET_WIDTH_TILES)));
  constant SPRITESHEET_WIDTH_PX_LOG    : natural := integer(log2(real(SPRITESHEET_WIDTH_PX)));
  constant SPRITESHEET_HEIGHT_PX_LOG   : natural := integer(log2(real(SPRITESHEET_HEIGHT_PX)));
  constant TILE_WIDTH_LOG              : natural := integer(log2(real(TILE_WIDTH_PX)));
  constant TILE_HEIGHT_LOG             : natural := integer(log2(real(TILE_HEIGHT_PX)));

  signal pixel_out_reg   : pixel_t   := default_pixel;
  signal pixel_valid_reg : std_logic := '0';
  signal done_reg        : std_logic := '0';

  signal x_start, y_start, x, y, x_delay, y_delay : unsigned(8 downto 0) := (others => '0'); -- 9 bits should be plenty

  -- rom ports
  -- signal en_b : boolean := false;
  signal addr_b : unsigned(integer(ceil(log2(real(SPRITESHEET_WIDTH_PX * SPRITESHEET_HEIGHT_PX)))) - 1 downto 0) := (others => '0');
  signal dout_b : std_logic_vector(15 downto 0);

  type state_t is (idle, draw_tile, last_pix);
  signal state_reg : state_t := idle;

begin
  pixel_out   <= pixel_out_reg;
  pixel_valid <= pixel_valid_reg;
  done        <= done_reg;

  spritesheet_prom : entity work.prom
    generic
    map (
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
    variable id_x : unsigned(SPRITESHEET_WIDTH_TILES_LOG - 1 downto 0);
    variable id_y : unsigned(tile_id'length - SPRITESHEET_WIDTH_TILES_LOG - 1 downto 0);

    variable x_start_var : unsigned(x_start'length - 1 downto 0);
    variable y_start_var : unsigned(y_start'length - 1 downto 0);
  begin
    if rising_edge(clk) then
      if reset = '1' then
        state_reg       <= idle;
        x_start         <= (others => '0');
        y_start         <= (others => '0');
        x               <= (others => '0');
        y               <= (others => '0');
        pixel_out_reg   <= default_pixel;
        pixel_valid_reg <= '0';
        done_reg        <= '0';
      else
        case state_reg is
          when idle =>
            done_reg <= '0';
            if go = '1' then
              state_reg <= draw_tile;
              id_x        := tile_id(SPRITESHEET_WIDTH_TILES_LOG - 1 downto 0);
              id_y        := tile_id(tile_id'length - 1 downto SPRITESHEET_WIDTH_TILES_LOG);
              x_start_var := shift_left(resize(id_x, x_start'length), TILE_WIDTH_LOG);
              y_start_var := shift_left(resize(id_y, y_start'length), TILE_HEIGHT_LOG);
              x_start <= x_start_var;
              y_start <= y_start_var;
              x       <= (others => '0');
              y       <= (others => '0');

              addr_b <= resize(
                resize(x_start_var, addr_b'length) + shift_left(resize(y_start_var, addr_b'length), SPRITESHEET_WIDTH_PX_LOG),
                addr_b'length
                );
            end if;
          when draw_tile =>
            -- navigate the tile
            if y = TILE_WIDTH_PX - 1 then
              if x = TILE_HEIGHT_PX - 1 then
                -- we reached the end of the tile. go to last_pix to allow pipeline to finish
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
              if x = TILE_WIDTH_PX - 1 then
                -- we finished the row, so go to the next row and reset x
                x      <= (others => '0');
                y      <= y + 1;
                addr_b <= resize(
                  resize(x_start, addr_b'length) + shift_left(resize(y + y_start + 1, addr_b'length), SPRITESHEET_WIDTH_PX_LOG),
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
            -- pixel_valid_reg <= '1'; -- temp
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
end tile_renderer;