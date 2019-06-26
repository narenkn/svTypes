.PHONY: all
CFLAGS = -I. -g

#% : %.sv %.cc
#	vcs -sverilog -o $@ $+ ./svTypes.cc -CFLAGS "$(CFLAGS)"

%: %.sv %.cc
	verilator --cc --exe -o $@ $*.sv $*.cc svTypes.cc verilator_main.cc -CFLAGS "$(CFLAGS)"
	make -C obj_dir -f Vt1.mk
