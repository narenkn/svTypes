
import "DPI-C" function void t1(output bit [63:0] b);

module test;

   bit [63:0] b1;
   int 	      in;

   initial begin
      b1[31:0] = 32'habcd_0123;
      b1[63:32] = 32'h4567_89ab;
      $display("From SV b1:%x", b1);
      t1(b1);
   end

endmodule

