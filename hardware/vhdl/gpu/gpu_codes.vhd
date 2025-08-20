library ieee;
use ieee.std_logic_1164.all;

package gpu_codes is
  subtype gpu_renderer_t is std_logic_vector(2 downto 0); -- max of 8 codes
  constant rect   : gpu_renderer_t := "000";
  constant circle : gpu_renderer_t := "001";
  constant line   : gpu_renderer_t := "010";
  constant sprite : gpu_renderer_t := "011";
  constant tile   : gpu_renderer_t := "100";
end gpu_codes;