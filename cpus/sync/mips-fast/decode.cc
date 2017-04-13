#include "decode.h"

Decode::Decode (Mipc *mc)
{
   _mc = mc;
}

Decode::~Decode (void) {}

void
Decode::MainLoop (void)
{
   unsigned int ins,pc;
   while (1) {
      AWAIT_P_PHI0;	// @posedge
      ins = _mc->IF_ID._ins;
      pc =  _mc->IF_ID._pc;;
      AWAIT_P_PHI1;	// @negedge
            _mc->Dec(ins);

      _mc->ID_EX._pc = pc;
      _mc->ID_EX._ins = ins;
      if(_mc->ID_EX._isSyscall){
#ifdef MIPC_DEBUG
      fprintf(_mc->_debugLog, "<%llu> SYS CALL FOUND in ins %#x\n", SIM_TIME, ins);
#endif
            _mc->IF_ID.reset();
            _mc->system_call_in_pipe = TRUE;
      }
#ifdef MIPC_DEBUG
      fprintf(_mc->_debugLog, "<%llu> Decoded ins %#x\n", SIM_TIME, ins);
      // fprintf(_mc->_debugLog, "_ins = %d\n",_mc->IF_ID._ins);
      // fprintf(_mc->_debugLog, "_decodedSRC1 = %d\n",_mc->IF_ID._decodedSRC1);
      // fprintf(_mc->_debugLog, "_decodedSRC2 = %d\n",_mc->IF_ID._decodedSRC2);
      // fprintf(_mc->_debugLog, "_decodedDST = %d\n",_mc->IF_ID._decodedDST);
      // fprintf(_mc->_debugLog, "_subregOperand = %d\n",_mc->IF_ID._subregOperand);
      // fprintf(_mc->_debugLog, "_MAR = %d\n",_mc->IF_ID._MAR);
      // fprintf(_mc->_debugLog, "_opResultHi = %d\n",_mc->IF_ID._opResultHi);
      // fprintf(_mc->_debugLog, "_opResultLo = %d\n",_mc->IF_ID._opResultLo);
      // fprintf(_mc->_debugLog, "_memControl = %d\n",_mc->IF_ID._memControl);
      // fprintf(_mc->_debugLog, "_writeREG = %d\n",_mc->IF_ID._writeREG);
      // fprintf(_mc->_debugLog, "_writeFREG = %d\n",_mc->IF_ID._writeFREG);
      // fprintf(_mc->_debugLog, "_branchOffset = %d\n",_mc->IF_ID._branchOffset);
      // fprintf(_mc->_debugLog, "_hiWPort = %d\n",_mc->IF_ID._hiWPort);
      // fprintf(_mc->_debugLog, "_loWPort = %d\n",_mc->IF_ID._loWPort);
      // fprintf(_mc->_debugLog, "_decodedShiftAmt = %d\n",_mc->IF_ID._decodedShiftAmt);
      // fprintf(_mc->_debugLog, "_hi = %d\n",_mc->IF_ID._hi);
      // fprintf(_mc->_debugLog, "_lo = %d\n",_mc->IF_ID._lo);
      // fprintf(_mc->_debugLog, "_lastbd = %d\n",_mc->IF_ID._lastbd);
      // fprintf(_mc->_debugLog, "_btaken = %d\n",_mc->IF_ID._btaken);
      // fprintf(_mc->_debugLog, "_bd = %d\n",_mc->IF_ID._bd);
      fprintf(_mc->_debugLog, "_btgt = %#x\n",_mc->ID_EX._btgt);
      // fprintf(_mc->_debugLog, "_isSyscall = %d\n",_mc->ID_EX._isSyscall);
      // fprintf(_mc->_debugLog, "_isIllegalOp = %d\n",_mc->IF_ID._isIllegalOp);
#endif
      }

}
