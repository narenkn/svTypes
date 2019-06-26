
import "DPI-C" function void t1(inout bit [63:0] b);

module test;

   bit [63:0] b1;
   int 	      in;

   initial begin
      b1[31:0] = 32'habcd_0123;
      b1[63:32] = 32'h4567_89ab;
      $display("Sending b1:%x", b1);
      t1(b1);
      $display("Received b1:%x", b1);
      $finish;
   end

endmodule

