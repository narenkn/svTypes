
CFLAGS = -I.

% : %.sv %.cc
	vcs -sverilog -o $@ $+ ./svTypes.cc -CFLAGS "$(CFLAGS)"

