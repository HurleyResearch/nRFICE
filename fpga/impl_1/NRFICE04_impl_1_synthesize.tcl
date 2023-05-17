if {[catch {

# define run engine funtion
source [file join {E:/lscc/radiant/2022.1} scripts tcl flow run_engine.tcl]
# define global variables
global para
set para(gui_mode) 1
set para(prj_dir) "E:/verilog/NRFICE04"
# synthesize IPs
# synthesize VMs
# propgate constraints
file delete -force -- NRFICE04_impl_1_cpe.ldc
run_engine_newmsg cpe -f "NRFICE04_impl_1.cprj" "ebr8x16.cprj" -a "iCE40UP"  -o NRFICE04_impl_1_cpe.ldc
# synthesize top design
file delete -force -- NRFICE04_impl_1.vm NRFICE04_impl_1.ldc
run_engine_newmsg synthesis -f "NRFICE04_impl_1_lattice.synproj"
run_postsyn [list -a iCE40UP -p iCE40UP5K -t SG48 -sp High-Performance_1.2V -oc Industrial -top -w -o NRFICE04_impl_1_syn.udb NRFICE04_impl_1.vm] "E:/verilog/NRFICE04/impl_1/NRFICE04_impl_1.ldc"

} out]} {
   runtime_log $out
   exit 1
}
