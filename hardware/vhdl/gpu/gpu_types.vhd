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

end package body gpu_types;
