
RED=\e[31m
GREEN=\e[32m
NC=\e[0m

#parts:
# 0 - pynq z2
# 1 - zcu104
PART=zcu104

sources := core.cpp ../raw_dep.cpp
test_sources :=  ../test/test.cpp
DEPS := $(sources) ../raw_dep.hpp

TARGET := raw_dep_core
TEST_TARGET := $(addsuffix _test, $(TARGET))

HLS_PERIOD ?= -1
IMPL_PERIOD ?= -1
DEP_DIST ?= -1
OP_CODE ?= -1
DATATYPE ?= -1

.NOTPARALLEL:

all: $(TARGET)

all_modules: all

csim: folders $(TARGET).hls_prj/$(PART)/csim

syn: folders $(TARGET).hls_prj/$(PART)/syn

vivado_synth: folders $(TARGET).hls_prj/$(PART)/impl/verilog/project.runs/synth_1/bd_0_wrapper_utilization_synth.rpt

impl: folders $(TARGET).hls_prj/$(PART)/impl/verilog/project.runs/impl_1/bd_0_wrapper_timing_summary_postroute_physopted.rpt

cosim: folders $(TARGET).hls_prj/$(PART)/sim

test: $(TEST_TARGET)

complete: $(TEST_TARGET) $(TARGET)

profiling_steps:  syn cosim impl

fast_profiling_steps:  syn cosim vivado_synth

.PHONY: clean all $(TARGET)

$(TARGET): %: folders %.hls_prj/$(PART)/syn  %.hls_prj/$(PART)/impl/ip/

$(TEST_TARGET): %_test: folders hls.tcl %.hls_prj/$(PART)/csim %.hls_prj/$(PART)/sim


#generic rules

folders: logs build

logs:
	mkdir -p logs

build:
	mkdir -p build

#csim  FIX: the created build folder has changed
%.hls_prj/$(PART)/csim: hls.tcl  $(DEPS) $(test_sources)
	@echo  "$(GREEN) #########  C simulation: $* #########$(NC)"
	vitis_hls hls.tcl 0 $(PART) $(HLS_PERIOD) $(IMPL_PERIOD) $(DEP_DIST) $(OP_CODE) $(DATATYPE)  > logs/$*_$(PART)_DD$(DEP_DIST)_OP$(OP_CODE)_TYPE$(DATATYPE)_csim.log

#synthesis  FIX: the created build folder has changed
%.hls_prj/$(PART)/syn: hls.tcl  $(DEPS)
	@echo  "$(GREEN) #########  HLS synthesis: $* #########$(NC)"
	vitis_hls hls.tcl 1 $(PART) $(HLS_PERIOD) $(IMPL_PERIOD) $(DEP_DIST) $(OP_CODE) $(DATATYPE)  > logs/$*_$(PART)_DD$(DEP_DIST)_OP$(OP_CODE)_TYPE$(DATATYPE)_syn.log

#cosim  FIX: the created build folder has changed
%.hls_prj/$(PART)/sim: hls.tcl  %.hls_prj/$(PART)/syn $(DEPS) $(test_sources)
	@echo  "$(GREEN) #########  RTL co-simulation: $* #########$(NC)"
	vitis_hls hls.tcl 2 $(PART) $(HLS_PERIOD) $(IMPL_PERIOD) $(DEP_DIST) $(OP_CODE) $(DATATYPE)  > logs/$*_$(PART)_DD$(DEP_DIST)_OP$(OP_CODE)_TYPE$(DATATYPE)_cosim.log

#export  FIX: the created build folder has changed
%.hls_prj/$(PART)/impl/ip/: hls.tcl  %.hls_prj/$(PART)/syn
	@echo  "$(GREEN) #########  Export IP : $* #########$(NC)"
	vitis_hls hls.tcl 3 $(PART) $(HLS_PERIOD) $(IMPL_PERIOD) $(DEP_DIST) $(OP_CODE) $(DATATYPE)  > logs/$*_$(PART)_DD$(DEP_DIST)_OP$(OP_CODE)_TYPE$(DATATYPE)_export.log

#export synth  FIX: the created build folder has changed
%.hls_prj/$(PART)/impl/verilog/project.runs/synth_1/bd_0_wrapper_utilization_synth.rpt: hls.tcl  %.hls_prj/$(PART)/syn
	@echo  "$(GREEN) #########  Vivado synthesis IP : $* #########$(NC)"
	vitis_hls hls.tcl 4 $(PART) $(HLS_PERIOD) $(IMPL_PERIOD) $(DEP_DIST) $(OP_CODE) $(DATATYPE)  > logs/$*_$(PART)_DD$(DEP_DIST)_OP$(OP_CODE)_TYPE$(DATATYPE)_vivadosyn.log

#export impl  FIX: the created build folder has changed
%.hls_prj/$(PART)/impl/verilog/project.runs/impl_1/bd_0_wrapper_timing_summary_postroute_physopted.rpt: hls.tcl  %.hls_prj/$(PART)/syn
	@echo  "$(GREEN) #########  Vivado Implementation IP : $* #########$(NC)"
	vitis_hls hls.tcl 5 $(PART) $(HLS_PERIOD) $(IMPL_PERIOD) $(DEP_DIST) $(OP_CODE) $(DATATYPE)  > logs/$*_$(PART)_DD$(DEP_DIST)_OP$(OP_CODE)_TYPE$(DATATYPE)_impl.log

CFLAGS := -I/tools/Xilinx/Vitis_HLS/2021.1/include/
clang_sim: $(sources) $(test_sources)
	clang++ $(sources) $(test_sources) $(CFLAGS) -o $@

clean:
	rm -rf logs *.log build  clang_sim
