#include "Vt1.h"
#include "verilated.h"

int
main(int argc, char **argv, char **env)
{
  Verilated::commandArgs(argc, argv);

  Vt1* top = new Vt1;
  while (!Verilated::gotFinish()) {
    top->eval(); 
  }

  delete top;
  exit(0);
}
