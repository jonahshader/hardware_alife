ThisBuild / version := "1.0"
ThisBuild / scalaVersion := "2.13.12"
ThisBuild / organization := "com.hardware_alife"

val spinalVersion = "1.10.2a"

lazy val root = (project in file("."))
  .settings(
    name := "hardware_alife_spinal",
    libraryDependencies ++= Seq(
      "com.github.spinalhdl" %% "spinalhdl-core" % spinalVersion,
      "com.github.spinalhdl" %% "spinalhdl-lib" % spinalVersion,
      compilerPlugin("com.github.spinalhdl" %% "spinalhdl-idsl-plugin" % spinalVersion),
      "org.scalatest" %% "scalatest" % "3.2.17" % Test
    ),
    Compile / scalaSource := baseDirectory.value / "src" / "main" / "scala",
    Test / scalaSource := baseDirectory.value / "src" / "test" / "scala"
  )

fork := true