
import "DPI-C" function void t1(output bit [63:0] b);

module test;

	bit [63:0] b1;
	int in;

	initial begin
		in = $urandom;
		b1[31:0] = in;
		in = $urandom;
		b1[63:32] = in;
		$display("From SV b1:%x", b1);
		t1(b1);
	end

endmodule

