# For arg parsing to work Vitis HLS needs to be called as: vitis_hls this_script.tcl [args]
set arglen [llength $argv]
set arg_idx 0

# mode param | 0: all steps | 1: just syn & export
set mode 0
set target "pynq_z2"


#mode
if { $arg_idx < $arglen } {
  set mode [lindex $argv $arg_idx]
  incr arg_idx
}
puts "Mode : $mode"


#target
if { $arg_idx < $arglen } {
  set target [lindex $argv $arg_idx]
  incr arg_idx
}

set sol_name $target

switch -exact -- $target {
  "pynq_z2" {
    set target_id 0
    set part xc7z020clg484-1
    set period  10
    set clk_uncertainty  0
  }
  "zcu104" {
    set target_id 1
    set part xczu7ev-ffvc1156-2-e
    set period 5
    set clk_uncertainty  0
  }
  default {
    puts "Error: target id not supported"
    quit
  }
}


if { $arg_idx < $arglen } {
  set aux_arg [lindex $argv $arg_idx]
  if { $aux_arg != -1} {
    set period $aux_arg
  }
  incr arg_idx
}

set vivado_clock $period
if { $arg_idx < $arglen } {
  set aux_arg [lindex $argv $arg_idx]
  if { $aux_arg != -1} {
    set vivado_clock $aux_arg
  }
  incr arg_idx
}

set DD 8
if { $arg_idx < $arglen } {
  set aux_arg [lindex $argv $arg_idx]
  if { $aux_arg != -1} {
    set DD $aux_arg
  }
  incr arg_idx
}

set OP_CODE 0
if { $arg_idx < $arglen } {
  set aux_arg [lindex $argv $arg_idx]
  if { $aux_arg != -1} {
    set OP_CODE $aux_arg
  }
  incr arg_idx
}

set DATATYPE 0
if { $arg_idx < $arglen } {
  set aux_arg [lindex $argv $arg_idx]
  if { $aux_arg != -1} {
    set DATATYPE $aux_arg
  }
  incr arg_idx
}


switch -exact -- $DATATYPE {
  0 {
    set type_name "double"
  }
  1 {
    set type_name "float"
  }
  2 {
    set type_name "int64"
  }
  default {
    puts "Error: DATATYPE($DATATYPE) not supported"
    quit
  }
}

switch -exact -- $OP_CODE {
  0 {
    set op_name "add"
  }
  1 {
    set op_name "mul"
  }
  2 {
    set op_name "div"
  }
  default {
    puts "Error: OP_CODE($OP_CODE) not supported"
    quit
  }
}


set ip_version $target_id

puts "Target : $sol_name"
puts "Period : $period"
puts "Clk uncertainty : $clk_uncertainty"
puts "Part : $part"

puts "Mode : $mode"

set module raw_dep_core

cd build

puts [pwd]

open_project "${module}_DD${DD}_${op_name}_${type_name}.hls_prj"
set_top "${module}"
add_files ../core.cpp -cflags "-Wmissing-field-initializers -DDD=${DD} -DOP_CODE=${OP_CODE} -DDATATYPE=${DATATYPE}" -csimflags "-Wmissing-field-initializers  -DOP_CODE=${OP_CODE} -DDATATYPE=${DATATYPE}"
add_files ../../raw_dep.cpp -cflags "-Wmissing-field-initializers -DDD=${DD} -DOP_CODE=${OP_CODE} -DDATATYPE=${DATATYPE}" -csimflags "-Wmissing-field-initializers  -DOP_CODE=${OP_CODE} -DDATATYPE=${DATATYPE}"
add_files -tb ../../test/test.cpp -cflags "-Wno-unknown-pragmas -Wmissing-field-initializers -DOP_CODE=${OP_CODE} -DDATATYPE=${DATATYPE}" -csimflags "-std=c++14 -fexceptions -Wno-unknown-pragmas -Wmissing-field-initializers -DOP_CODE=${OP_CODE} -DDATATYPE=${DATATYPE}"
open_solution "${sol_name}" -flow_target vivado
set_part "$part"
create_clock -period ${period} -name default
set_clock_uncertainty ${clk_uncertainty}
config_compile -enable_auto_rewind=false
config_schedule  -verbose

config_export -format ip_catalog -rtl verilog -library loco_ans -vendor HPCN -version "1.0" -vivado_synth_strategy "Flow_PerfOptimized_high" -vivado_optimization_level 3 -vivado_impl_strategy "Performance_ExploreWithRemap" -vivado_phys_opt all -vivado_clock $vivado_clock  -vivado_report_level 2

# source "./directives.tcl"
if { $mode == 0 } {
  csim_design -clean
}

if { $mode == 1 } {
  csynth_design
}


if { $mode == 2 } {
  cosim_design -enable_dataflow_profiling -trace_level all
}

if { $mode == 3 } {
  export_design -format ip_catalog
}

if { $mode == 3 } {
  export_design -format ip_catalog
}

if { $mode == 4 } {
  export_design -format ip_catalog -flow syn
}

if { $mode == 5 } {
  export_design -format ip_catalog -flow impl
}
