#include "decode.h"

Decode::Decode (Mipc *mc)
{
   _mc = mc;
}

Decode::~Decode (void) {}

void
Decode::MainLoop (void)
{
   unsigned int ins;
   while (1) {
      AWAIT_P_PHI0;	// @posedge
      ins = _mc->IF_ID._ins;
      AWAIT_P_PHI1;	// @negedge
      _mc->Dec(ins);
      _mc->ID_EX._pc = _mc->_pc;
      _mc->ID_EX._ins = ins;
#ifdef MIPC_DEBUG
      fprintf(_mc->_debugLog, "<%llu> Decoded ins %#x\n", SIM_TIME, ins);
#endif
      }

}
