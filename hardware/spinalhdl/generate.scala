import spinal.core._
import spinal.core.sim._
import gpu._

object GenerateGpu extends App {
  // Generate VHDL for the GPU types
  SpinalConfig(
    targetDirectory = "generated/vhdl",
    defaultConfigForClockDomains = ClockDomainConfig(resetKind = SYNC)
  ).generateVhdl(new Component {
    val io = new Bundle {
      val instr = in(GpuInstr())
      val valid = in(Bool())
      // Add other GPU interface signals as needed
    }
    
    // Simple example showing union access
    when(io.valid) {
      switch(io.instr.renderer) {
        is(GpuRenderer.RECT) {
          val rectInstr = io.instr.asRect
          // Process rectangle instruction
        }
        is(GpuRenderer.CIRCLE) {
          val circleInstr = io.instr.asCircle
          // Process circle instruction
        }
        is(GpuRenderer.LINE) {
          val lineInstr = io.instr.asLine
          // Process line instruction
        }
        default {
          // Handle other cases
        }
      }
    }
    
    setDefinitionName("GpuCore")
  })
  
  println("VHDL generated in generated/vhdl/")
}