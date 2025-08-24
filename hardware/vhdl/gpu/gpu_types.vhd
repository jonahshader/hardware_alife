library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package gpu_types is

  subtype  gpu_renderer_t is std_logic_vector(2 downto 0); -- max of 8 codes
  constant RECT   : gpu_renderer_t := "000";
  constant CIRCLE : gpu_renderer_t := "001";
  constant LINE   : gpu_renderer_t := "010";
  constant SPRITE : gpu_renderer_t := "011";
  constant TILE   : gpu_renderer_t := "100";

  subtype raw_instr_body_t is std_logic_vector(69 downto 0);

  subtype color_component_t is unsigned(3 downto 0);
  function default_color_component return color_component_t;
  -- 12 bits
  type rgb_t is record
    r : color_component_t;
    g : color_component_t;
    b : color_component_t;
  end record rgb_t;
  function default_rgb return rgb_t;

  -- 16 bits
  type rgba_t is record
    r : color_component_t;
    g : color_component_t;
    b : color_component_t;
    a : color_component_t;
  end record rgba_t;
  function default_rgba return rgba_t;

  subtype world_comp_t is unsigned(11 downto 0);
  type    world_vec_t is record
    x : world_comp_t;
    y : world_comp_t;
  end record world_vec_t;
  function default_world_vec return world_vec_t;

  subtype render_comp_t is signed(8 downto 0);
  -- 18 bits
  type render_vec_t is record
    x : render_comp_t;
    y : render_comp_t;
  end record render_vec_t;
  function default_screen_vec return render_vec_t;

  subtype fb_comp_t is unsigned(7 downto 0);
  type fb_vec_t is record
    x : fb_comp_t;
    y : fb_comp_t;
  end record fb_vec_t;
  function default_fb_vec return fb_vec_t;

  subtype fb_addr_t is unsigned(16 downto 0); -- 17 bits, 2^17 / 2 = 256x256 (max res)

  subtype length_t is unsigned(7 downto 0);
  -- 16 bits
  type dims_t is record
    width  : length_t;
    height : length_t;
  end record dims_t;

  type gpu_instr_t is record
    renderer : gpu_renderer_t;
    data     : raw_instr_body_t;
  end record gpu_instr_t;

  -- 50 bits
  type rect_instr_t is record
    pos   : render_vec_t; -- 18 bits
    size  : dims_t;       -- 16 bits
    color : rgba_t;       -- 16 bits
  end record rect_instr_t;
  function default_rect_instr return rect_instr_t;
  function to_raw(instr : rect_instr_t) return raw_instr_body_t;
  function from_raw(raw : raw_instr_body_t) return rect_instr_t;

  -- 50 bits
  type circle_instr_t is record
    pos          : render_vec_t; -- 18 bits
    outer_radius : length_t;     -- 8 bits
    inner_radius : length_t;     -- 8 bits
    color        : rgba_t;       -- 16 bits
  end record circle_instr_t;
  function default_circle_instr return circle_instr_t;
  function to_raw(instr : circle_instr_t) return raw_instr_body_t;
  function from_raw(raw : raw_instr_body_t) return circle_instr_t;

  -- 52 bits
  type line_instr_t is record
    start_pos : render_vec_t; -- 18 bits
    end_pos   : render_vec_t; -- 18 bits
    color     : rgba_t;       -- 16 bits
  end record line_instr_t;
  function default_line_instr return line_instr_t;
  function to_raw(instr : line_instr_t) return raw_instr_body_t;
  function from_raw(raw : raw_instr_body_t) return line_instr_t;

  -- 70 bits
  type triangle_instr_t is record
    p1    : render_vec_t; -- 18 bits
    p2    : render_vec_t; -- 18 bits
    p3    : render_vec_t; -- 18 bits
    color : rgba_t;       -- 16 bits
  end record triangle_instr_t;
  function default_triangle_instr return triangle_instr_t;
  function to_raw(instr : triangle_instr_t) return raw_instr_body_t;
  function from_raw(raw : raw_instr_body_t) return triangle_instr_t;

end package gpu_types;

package body gpu_types is

  function default_color_component return color_component_t is
  begin
    return (others => '0');
  end function;

  function default_rgb return rgb_t is
  begin
    return (r => (others => '0'), g => (others => '0'), b => (others => '0'));
  end function;

  function default_rgba return rgba_t is
  begin
    return (r => (others => '0'), g => (others => '0'), b => (others => '0'), a => (others => '0'));
  end function;

  function default_world_vec return world_vec_t is
  begin
    return (x => (others => '0'), y => (others => '0'));
  end function;

  function default_screen_vec return render_vec_t is
  begin
    return (x => (others => '0'), y => (others => '0'));
  end function;

  function default_fb_vec return fb_vec_t is
  begin
    return (x => (others => '0'), y => (others => '0'));
  end function;

  function default_rect_instr return rect_instr_t is
  begin
    return (pos => default_screen_vec, size => (width => (others => '0'), height => (others => '0')), color => default_rgba);
  end function;

  -- rect_instr_t: pos (18) + size (16) + color (16) = 50 bits
  function to_raw(instr : rect_instr_t) return raw_instr_body_t is
    variable result : raw_instr_body_t := (others => '0');
  begin
    result(69 downto 52) := std_logic_vector(instr.pos.x);     -- 9 bits at top
    result(51 downto 43) := std_logic_vector(instr.pos.y);     -- 9 bits
    result(42 downto 35) := std_logic_vector(instr.size.width); -- 8 bits
    result(34 downto 27) := std_logic_vector(instr.size.height); -- 8 bits
    result(26 downto 23) := std_logic_vector(instr.color.r);    -- 4 bits
    result(22 downto 19) := std_logic_vector(instr.color.g);    -- 4 bits
    result(18 downto 15) := std_logic_vector(instr.color.b);    -- 4 bits
    result(14 downto 11) := std_logic_vector(instr.color.a);    -- 4 bits
    return result;
  end function;

  function from_raw(raw : raw_instr_body_t) return rect_instr_t is
    variable result : rect_instr_t;
  begin
    result.pos.x := signed(raw(69 downto 52));
    result.pos.y := signed(raw(51 downto 43));
    result.size.width := unsigned(raw(42 downto 35));
    result.size.height := unsigned(raw(34 downto 27));
    result.color.r := unsigned(raw(26 downto 23));
    result.color.g := unsigned(raw(22 downto 19));
    result.color.b := unsigned(raw(18 downto 15));
    result.color.a := unsigned(raw(14 downto 11));
    return result;
  end function;

  function default_circle_instr return circle_instr_t is
  begin
    return (pos => default_screen_vec, outer_radius => (others => '0'), inner_radius => (others => '0'), color => default_rgba);
  end function;

  -- circle_instr_t: pos (18) + outer_radius (8) + inner_radius (8) + color (16) = 50 bits
  function to_raw(instr : circle_instr_t) return raw_instr_body_t is
    variable result : raw_instr_body_t := (others => '0');
  begin
    result(69 downto 52) := std_logic_vector(instr.pos.x);      -- 9 bits
    result(51 downto 43) := std_logic_vector(instr.pos.y);      -- 9 bits
    result(42 downto 35) := std_logic_vector(instr.outer_radius); -- 8 bits
    result(34 downto 27) := std_logic_vector(instr.inner_radius); -- 8 bits
    result(26 downto 23) := std_logic_vector(instr.color.r);     -- 4 bits
    result(22 downto 19) := std_logic_vector(instr.color.g);     -- 4 bits
    result(18 downto 15) := std_logic_vector(instr.color.b);     -- 4 bits
    result(14 downto 11) := std_logic_vector(instr.color.a);     -- 4 bits
    return result;
  end function;

  function from_raw(raw : raw_instr_body_t) return circle_instr_t is
    variable result : circle_instr_t;
  begin
    result.pos.x := signed(raw(69 downto 52));
    result.pos.y := signed(raw(51 downto 43));
    result.outer_radius := unsigned(raw(42 downto 35));
    result.inner_radius := unsigned(raw(34 downto 27));
    result.color.r := unsigned(raw(26 downto 23));
    result.color.g := unsigned(raw(22 downto 19));
    result.color.b := unsigned(raw(18 downto 15));
    result.color.a := unsigned(raw(14 downto 11));
    return result;
  end function;

  function default_line_instr return line_instr_t is
  begin
    return (start_pos => default_screen_vec, end_pos => default_screen_vec, color => default_rgba);
  end function;

  -- line_instr_t: start_pos (18) + end_pos (18) + color (16) = 52 bits
  function to_raw(instr : line_instr_t) return raw_instr_body_t is
    variable result : raw_instr_body_t := (others => '0');
  begin
    result(69 downto 52) := std_logic_vector(instr.start_pos.x); -- 9 bits
    result(51 downto 43) := std_logic_vector(instr.start_pos.y); -- 9 bits
    result(42 downto 34) := std_logic_vector(instr.end_pos.x);   -- 9 bits
    result(33 downto 25) := std_logic_vector(instr.end_pos.y);   -- 9 bits
    result(24 downto 21) := std_logic_vector(instr.color.r);     -- 4 bits
    result(20 downto 17) := std_logic_vector(instr.color.g);     -- 4 bits
    result(16 downto 13) := std_logic_vector(instr.color.b);     -- 4 bits
    result(12 downto 9)  := std_logic_vector(instr.color.a);     -- 4 bits
    return result;
  end function;

  function from_raw(raw : raw_instr_body_t) return line_instr_t is
    variable result : line_instr_t;
  begin
    result.start_pos.x := signed(raw(69 downto 52));
    result.start_pos.y := signed(raw(51 downto 43));
    result.end_pos.x := signed(raw(42 downto 34));
    result.end_pos.y := signed(raw(33 downto 25));
    result.color.r := unsigned(raw(24 downto 21));
    result.color.g := unsigned(raw(20 downto 17));
    result.color.b := unsigned(raw(16 downto 13));
    result.color.a := unsigned(raw(12 downto 9));
    return result;
  end function;

  function default_triangle_instr return triangle_instr_t is
  begin
    return (p1 => default_screen_vec, p2 => default_screen_vec, p3 => default_screen_vec, color => default_rgba);
  end function;

  -- triangle_instr_t: p1 (18) + p2 (18) + p3 (18) + color (16) = 70 bits (uses full width)
  function to_raw(instr : triangle_instr_t) return raw_instr_body_t is
    variable result : raw_instr_body_t := (others => '0');
  begin
    result(69 downto 61) := std_logic_vector(instr.p1.x);       -- 9 bits
    result(60 downto 52) := std_logic_vector(instr.p1.y);       -- 9 bits
    result(51 downto 43) := std_logic_vector(instr.p2.x);       -- 9 bits
    result(42 downto 34) := std_logic_vector(instr.p2.y);       -- 9 bits
    result(33 downto 25) := std_logic_vector(instr.p3.x);       -- 9 bits
    result(24 downto 16) := std_logic_vector(instr.p3.y);       -- 9 bits
    result(15 downto 12) := std_logic_vector(instr.color.r);    -- 4 bits
    result(11 downto 8)  := std_logic_vector(instr.color.g);    -- 4 bits
    result(7 downto 4)   := std_logic_vector(instr.color.b);    -- 4 bits
    result(3 downto 0)   := std_logic_vector(instr.color.a);    -- 4 bits
    return result;
  end function;

  function from_raw(raw : raw_instr_body_t) return triangle_instr_t is
    variable result : triangle_instr_t;
  begin
    result.p1.x := signed(raw(69 downto 61));
    result.p1.y := signed(raw(60 downto 52));
    result.p2.x := signed(raw(51 downto 43));
    result.p2.y := signed(raw(42 downto 34));
    result.p3.x := signed(raw(33 downto 25));
    result.p3.y := signed(raw(24 downto 16));
    result.color.r := unsigned(raw(15 downto 12));
    result.color.g := unsigned(raw(11 downto 8));
    result.color.b := unsigned(raw(7 downto 4));
    result.color.a := unsigned(raw(3 downto 0));
    return result;
  end function;

end package body gpu_types;
