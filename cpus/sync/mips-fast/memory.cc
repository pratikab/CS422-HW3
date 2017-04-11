#include "memory.h"

Memory::Memory (Mipc *mc)
{
   _mc = mc;
}

Memory::~Memory (void) {}

void
Memory::MainLoop (void)
{
   Bool memControl;

   while (1) {
      AWAIT_P_PHI0;	// @posedge
         memControl = _mc->EX_MEM._memControl;
         // AWAIT_P_PHI1;       // @negedge
         if (memControl) {
            _mc->EX_MEM._memOp(_mc);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Accessing memory at address %#x for ins %#x\n", SIM_TIME, _mc->EX_MEM._MAR, _mc->EX_MEM._ins);
#endif
         }
         else {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Memory has nothing to do for ins %#x\n", SIM_TIME, _mc->EX_MEM._ins);
#endif
         }
      _mc->MEM_WB._pc = _mc->_pc;
      _mc->MEM_WB._ins = _mc->EX_MEM._ins;
      _mc->MEM_WB._writeREG=_mc->EX_MEM._writeREG;
      _mc->MEM_WB._writeFREG=_mc->EX_MEM._writeFREG;
      _mc->MEM_WB._loWPort=_mc->EX_MEM._loWPort;
      _mc->MEM_WB._hiWPort=_mc->EX_MEM._hiWPort;
      _mc->MEM_WB._decodedDST=_mc->EX_MEM._decodedDST;
      _mc->MEM_WB._opResultLo=_mc->EX_MEM._opResultLo;
      _mc->MEM_WB._opResultHi=_mc->EX_MEM._opResultHi;
      _mc->MEM_WB._isSyscall=_mc->EX_MEM._isSyscall;
      _mc->MEM_WB._isIllegalOp=_mc->EX_MEM._isIllegalOp;

   }
}
