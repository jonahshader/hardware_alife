package gpu

import spinal.core._
import spinal.lib._

object GpuRenderer extends SpinalEnum {
  val RECT, CIRCLE, LINE, SPRITE, TILE = newElement()
}

case class ColorComponent() extends Bundle {
  val value = UInt(4 bits)
}

case class RgbColor() extends Bundle {
  val r = ColorComponent()
  val g = ColorComponent()
  val b = ColorComponent()
}

case class RgbaColor() extends Bundle {
  val r = ColorComponent()
  val g = ColorComponent()
  val b = ColorComponent()
  val a = ColorComponent()
}

case class WorldVec() extends Bundle {
  val x = UInt(12 bits)
  val y = UInt(12 bits)
}

case class RenderVec() extends Bundle {
  val x = SInt(9 bits)
  val y = SInt(9 bits)
}

case class FbVec() extends Bundle {
  val x = UInt(8 bits)
  val y = UInt(8 bits)
}

case class Dims() extends Bundle {
  val width = UInt(8 bits)
  val height = UInt(8 bits)
}

// Individual instruction types
case class RectInstr() extends Bundle {
  val pos = RenderVec()
  val size = Dims()
  val color = RgbaColor()
}

case class CircleInstr() extends Bundle {
  val pos = RenderVec()
  val outer_radius = UInt(8 bits)
  val inner_radius = UInt(8 bits)
  val color = RgbaColor()
}

case class LineInstr() extends Bundle {
  val start_pos = RenderVec()
  val end_pos = RenderVec()
  val color = RgbaColor()
}

case class SpriteInstr() extends Bundle {
  val pos = RenderVec()
  val sprite_id = UInt(8 bits)
  val size = Dims()
  val color = RgbaColor()
}

case class TileInstr() extends Bundle {
  val pos = RenderVec()
  val tile_id = UInt(8 bits)
  val size = Dims()
  val color = RgbaColor()
}

// Union type using SpinalHDL's Bundle discrimination
case class GpuInstr() extends Bundle {
  val renderer = GpuRenderer()
  val payload = Bits(70 bits) // Raw payload, will be interpreted based on renderer
  
  // Helper methods to get typed instructions
  def asRect: RectInstr = {
    val instr = RectInstr()
    instr.assignFromBits(payload(49 downto 0))
    instr
  }
  
  def asCircle: CircleInstr = {
    val instr = CircleInstr()
    instr.assignFromBits(payload(49 downto 0))
    instr
  }
  
  def asLine: LineInstr = {
    val instr = LineInstr()
    instr.assignFromBits(payload(51 downto 0))
    instr
  }
  
  def asSprite: SpriteInstr = {
    val instr = SpriteInstr()
    instr.assignFromBits(payload(57 downto 0))
    instr
  }
  
  def asTile: TileInstr = {
    val instr = TileInstr()
    instr.assignFromBits(payload(57 downto 0))
    instr
  }
}

object GpuInstr {
  def apply(renderer: GpuRenderer.E, instr: Bundle): GpuInstr = {
    val gpuInstr = GpuInstr()
    gpuInstr.renderer := renderer
    gpuInstr.payload := instr.asBits.resized
    gpuInstr
  }
}