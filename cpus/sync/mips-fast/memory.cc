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
   unsigned int ins,pc;
   Bool writeREG;
   Bool writeFREG;
   Bool loWPort;
   Bool hiWPort;
   Bool isSyscall;
   Bool isIllegalOp;
   unsigned decodedDST;
   unsigned opResultLo, opResultHi;
   void (*opControl)(Mipc*, unsigned);
   while (1) {
      AWAIT_P_PHI0;       // @posedge
         memControl = _mc->EX_MEM._memControl;
         pc = _mc->EX_MEM._pc;
         ins = _mc->EX_MEM._ins;
         writeREG=_mc->EX_MEM._writeREG;
         writeFREG=_mc->EX_MEM._writeFREG;
         loWPort=_mc->EX_MEM._loWPort;
         hiWPort=_mc->EX_MEM._hiWPort;
         decodedDST=_mc->EX_MEM._decodedDST;
         opResultLo=_mc->EX_MEM._opResultLo;
         opResultHi=_mc->EX_MEM._opResultHi;
         isSyscall=_mc->EX_MEM._isSyscall;
         isIllegalOp=_mc->EX_MEM._isIllegalOp;
         opControl=_mc->EX_MEM._opControl;         
         if (memControl) {
            _mc->EX_MEM._memOp(_mc);

#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Accessing memory at address %#x for ins %#x\n", SIM_TIME, _mc->EX_MEM._MAR, _mc->EX_MEM._ins);
            // fprintf(_mc->_debugLog, "_ins = %d\n",_mc->EX_MEM._ins);
            // fprintf(_mc->_debugLog, "_decodedSRC1 = %d\n",_mc->EX_MEM._decodedSRC1);
            // fprintf(_mc->_debugLog, "_decodedSRC2 = %d\n",_mc->EX_MEM._decodedSRC2);
            // fprintf(_mc->_debugLog, "_decodedDST = %d\n",_mc->EX_MEM._decodedDST);
            // fprintf(_mc->_debugLog, "_subregOperand = %d\n",_mc->EX_MEM._subregOperand);
            // fprintf(_mc->_debugLog, "_MAR = %d\n",_mc->EX_MEM._MAR);
            // fprintf(_mc->_debugLog, "_opResultHi = %d\n",_mc->EX_MEM._opResultHi);
            // fprintf(_mc->_debugLog, "_opResultLo = %d\n",_mc->EX_MEM._opResultLo);
            // fprintf(_mc->_debugLog, "_memControl = %d\n",_mc->EX_MEM._memControl);
            // fprintf(_mc->_debugLog, "_writeREG = %d\n",_mc->EX_MEM._writeREG);
            // fprintf(_mc->_debugLog, "_writeFREG = %d\n",_mc->EX_MEM._writeFREG);
            // fprintf(_mc->_debugLog, "_branchOffset = %d\n",_mc->EX_MEM._branchOffset);
            // fprintf(_mc->_debugLog, "_hiWPort = %d\n",_mc->EX_MEM._hiWPort);
            // fprintf(_mc->_debugLog, "_loWPort = %d\n",_mc->EX_MEM._loWPort);
            // fprintf(_mc->_debugLog, "_decodedShiftAmt = %d\n",_mc->EX_MEM._decodedShiftAmt);
            // fprintf(_mc->_debugLog, "_hi = %d\n",_mc->EX_MEM._hi);
            // fprintf(_mc->_debugLog, "_lo = %d\n",_mc->EX_MEM._lo);
            // fprintf(_mc->_debugLog, "_lastbd = %d\n",_mc->EX_MEM._lastbd);
            // fprintf(_mc->_debugLog, "_btaken = %d\n",_mc->EX_MEM._btaken);
            // fprintf(_mc->_debugLog, "_bd = %d\n",_mc->EX_MEM._bd);
            // fprintf(_mc->_debugLog, "_btgt = %d\n",_mc->EX_MEM._btgt);
            // fprintf(_mc->_debugLog, "_isSyscall = %d\n",_mc->EX_MEM._isSyscall);
            // fprintf(_mc->_debugLog, "_isIllegalOp = %d\n",_mc->EX_MEM._isIllegalOp);
#endif
         }
         else {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Memory has nothing to do for ins %#x\n", SIM_TIME, _mc->EX_MEM._ins);
#endif
         }

      AWAIT_P_PHI1;  // @negedge
      _mc->MEM_WB._pc = pc;
      _mc->MEM_WB._ins = ins;
      _mc->MEM_WB._writeREG=writeREG;
      _mc->MEM_WB._writeFREG=writeFREG;
      _mc->MEM_WB._loWPort=loWPort;
      _mc->MEM_WB._hiWPort=hiWPort;
      _mc->MEM_WB._decodedDST=decodedDST;
      _mc->MEM_WB._opResultLo=opResultLo;
      _mc->MEM_WB._opResultHi=opResultHi;
      _mc->MEM_WB._isSyscall=isSyscall;
      _mc->MEM_WB._isIllegalOp=isIllegalOp;
      _mc->MEM_WB._opControl=opControl;
   }
}
