-- Listing 5.8 from book 2, modified to use numeric_std instead of std_logic_unsigned
-- also uses custom types.
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.custom_types.all;

entity plotline is
  port (
    clk       : in std_logic;
    reset     : in std_logic;
    go        : in std_logic;
    p0        : in frame_coord_t;
    p1        : in frame_coord_t;
    donep     : in std_logic; -- wtf are these?
    donep1    : out std_logic;
    goplot    : out std_logic;
    color     : in color_t;
    pixel_out : out pixel_t
  );
end entity plotline;

architecture plotline of plotline is

  type   state_t is (START, TEST1, PLOT, UPDATE, DONE);
  signal state  : state_t            := START;
  signal deltax : frame_pos_t        := default_frame_pos;
  signal deltay : frame_pos_t        := default_frame_pos;
  signal eps    : frame_pos_t        := default_frame_pos;
  signal ystep  : signed(1 downto 0) := (others => '0');
  signal xs     : frame_pos_t        := default_frame_pos;
  signal ys     : frame_pos_t        := default_frame_pos;
  signal x0s    : frame_pos_t        := default_frame_pos;
  signal y0s    : frame_pos_t        := default_frame_pos;
  signal x1s    : frame_pos_t        := default_frame_pos;
  signal y1s    : frame_pos_t        := default_frame_pos;
  signal steep  : std_logic          := '0';

begin

  setup_proc : process (p0, p1) is
    variable x0v, y0v, x1v, y1v : frame_pos_t;
    variable xm, ym             : signed(frame_pos_t'length - 1 downto 0);
    variable steepv             : std_logic;
    variable delxv, delyv, temp : frame_pos_t;
  begin
    x0v := p0.x;
    x1v := p1.x;
    y0v := p0.y;
    y1v := p1.y;
    xm  := signed(x1v) - signed(x0v);
    ym  := signed(y1v) - signed(y0v);

    if xm < 0 then
      xm := 0 - xm;
    end if;
    if ym < 0 then
      ym := 0 - ym;
    end if;
    if ym > xm then
      steepv := '1';
    else
      steepv := '0';
    end if;
    if steepv = '1' then
      temp := x0v;
      x0v  := y0v;
      y0v  := temp;
      temp := x1v;
      x1v  := y1v;
      y1v  := temp;
    end if;
    if x0v > x1v then
      temp := x0v;
      x0v  := x1v;
      x1v  := temp;
      temp := y0v;
      y0v  := y1v;
      y1v  := temp;
    end if;
    if y0v < y1v then
      ystep <= to_signed(1, ystep'length);
    else
      ystep <= to_signed(-1, ystep'length);
    end if;
    xm := signed(x1v) - signed(x0v);
    ym := signed(y1v) - signed(y0v);
    if xm < 0 then
      xm := 0 - xm;
    end if;
    if ym < 0 then
      ym := 0 - ym;
    end if;
    deltax <= unsigned(xm);
    deltay <= unsigned(ym);
    x0s    <= x0v;
    y0s    <= y0v;
    x1s    <= x1v;
    y1s    <= y1v;
    steep  <= steepv;
  end process;

  wloop_proc : process (clk, reset) is
    variable xv, yv             : frame_pos_t;
    variable epsv, delxv, delyv : signed(frame_pos_t'length - 1 downto 0);
  begin
    -- why do we have to do this bit-wise?
    for i in 0 to frame_pos_t'length - 1 loop
      delxv(i) := deltax(i);
      delyv(i) := deltay(i);
    end loop;

    if reset = '1' then
      state  <= START;
      goplot <= '0';
      donep1 <= '1';
      xs     <= x0s;
      ys     <= y0s;
    elsif rising_edge(clk) then
      case state is
        when START =>
          goplot <= '0';
          donep1 <= '0';              -- modified from book. we want done to go high for only one cycle at the end of computation.
          epsv   := to_signed(0, epsv'length);
          xs     <= x0s;
          ys     <= y0s;
          if go = '1' then
            donep1          <= '0';
            state           <= TEST1;
            pixel_out.color <= color; -- TODO might make this combinational if this module will be wrapped.
          else
            state <= START;
          end if;
        when TEST1 =>
          if xs <= x1s then
            state  <= PLOT;
            goplot <= '1';
          else
            state <= DONE;
          end if;
        when PLOT =>
          goplot <= '0';
          epsv   := epsv + delyv;
          state  <= UPDATE;
        when UPDATE =>
          if donep = '0' then
            state <= UPDATE;
          else
            if (epsv(epsv'length - 2 downto 0) & '0') >= delxv then
              ys   <= unsigned(signed(ys) + ystep);
              epsv := epsv - delxv;
            end if;
            xs    <= xs + 1;
            state <= TEST1;
          end if;
        when DONE =>
          donep1 <= '1';
          if go = '1' then
            state <= DONE;
          else
            state <= START;
          end if;
        when others =>
          null;
      end case;
    end if;

    for i in 0 to eps'length - 1 loop
      eps(i) <= epsv(i);
    end loop;
  end process;

  plotchk_proc : process (xs, ys, steep) is
    variable xv, yv, temp : frame_pos_t;
  begin
    xv := xs;
    yv := ys;
    if steep = '1' then
      temp := xv;
      xv   := yv;
      yv   := temp;
    end if;
    pixel_out.coord.x <= xv;
    pixel_out.coord.y <= yv;
  end process;

end architecture plotline;
