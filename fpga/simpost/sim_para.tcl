lappend auto_path "E:/lscc/radiant/2022.1/scripts/tcl/simulation"
package require simulation_generation
set ::bali::simulation::Para(DEVICEPM) {ice40tp}
set ::bali::simulation::Para(DEVICEFAMILYNAME) {iCE40UP}
set ::bali::simulation::Para(PROJECT) {simpost}
set ::bali::simulation::Para(PROJECTPATH) {E:/verilog/NRFICE04}
set ::bali::simulation::Para(FILELIST) {"E:/verilog/NRFICE04/impl_1/NRFICE04_impl_1_vo.vo" "E:/verilog/NRFICE04/testbench.v" }
set ::bali::simulation::Para(GLBINCLIST) {}
set ::bali::simulation::Para(INCLIST) {"none" "none" "none"}
set ::bali::simulation::Para(WORKLIBLIST) {"" "work" "" }
set ::bali::simulation::Para(COMPLIST) {"VERILOG" "VERILOG" "none" }
set ::bali::simulation::Para(LANGSTDLIST) {"" "Verilog 2001" "" }
set ::bali::simulation::Para(SIMLIBLIST) {pmi_work ovi_ice40up}
set ::bali::simulation::Para(MACROLIST) {}
set ::bali::simulation::Para(SIMULATIONTOPMODULE) {toptb}
set ::bali::simulation::Para(SIMULATIONINSTANCE) {/UUT}
set ::bali::simulation::Para(LANGUAGE) {VERILOG}
set ::bali::simulation::Para(SDFPATH)  {E:/verilog/NRFICE04/impl_1/NRFICE04_impl_1_vo.sdf}
set ::bali::simulation::Para(INSTALLATIONPATH) {E:/lscc/radiant/2022.1}
set ::bali::simulation::Para(MEMPATH) {E:/verilog/NRFICE04/ebr8x16}
set ::bali::simulation::Para(UDOLIST) {}
set ::bali::simulation::Para(ADDTOPLEVELSIGNALSTOWAVEFORM)  {1}
set ::bali::simulation::Para(RUNSIMULATION)  {1}
set ::bali::simulation::Para(SIMULATIONTIME)  {1}
set ::bali::simulation::Para(SIMULATIONTIMEUNIT)  {ms}
set ::bali::simulation::Para(ISRTL)  {0}
set ::bali::simulation::Para(HDLPARAMETERS) {}
::bali::simulation::ModelSim_Run
