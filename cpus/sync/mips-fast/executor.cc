#include "executor.h"
#include "iostream.h"
Exe::Exe (Mipc *mc)
{
   _mc = mc;
}

Exe::~Exe (void) {}

void
Exe::MainLoop (void)
{
   unsigned int ins,pc;
   Bool isSyscall, isIllegalOp, flag = FALSE;
   Bool writeREG;
   Bool writeFREG;
   Bool loWPort;
   Bool hiWPort, memControl;
   unsigned decodedDST;
   unsigned opResultLo, opResultHi,lastbd, btgt;
   int  btaken ,bd;
   void (*opControl)(Mipc*, unsigned);
   void (*memOp)(Mipc*);

   while (1) {
      AWAIT_P_PHI0;	// @posedge
         ins = _mc->ID_EX._ins;
         isSyscall = _mc->ID_EX._isSyscall;
         isIllegalOp = _mc->ID_EX._isIllegalOp;
         pc = _mc->ID_EX._pc;
         memControl = _mc->ID_EX._memControl;
         memOp = _mc->ID_EX._memOp;
         writeREG=_mc->ID_EX._writeREG;
         writeFREG=_mc->ID_EX._writeFREG;
         loWPort=_mc->ID_EX._loWPort;
         hiWPort=_mc->ID_EX._hiWPort;
         decodedDST=_mc->ID_EX._decodedDST;
         opControl=_mc->ID_EX._opControl;
         lastbd = _mc->ID_EX._lastbd;
         btgt = _mc->ID_EX._btgt;
         bd =_mc->ID_EX._bd;
         _mc->EX_MEM._btaken = 0;
         if (!isSyscall && !isIllegalOp) {
            if (_mc->ID_EX._opControl != NULL)
               _mc->ID_EX._opControl(_mc,ins);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Executed ins %#x\n", SIM_TIME, ins);
            // fprintf(_mc->_debugLog, "_ins = %d\n",_mc->ID_EX._ins);
            // fprintf(_mc->_debugLog, "_decodedSRC1 = %d\n",_mc->ID_EX._decodedSRC1);
            // fprintf(_mc->_debugLog, "_decodedSRC2 = %d\n",_mc->ID_EX._decodedSRC2);
            // fprintf(_mc->_debugLog, "_decodedDST = %d\n",_mc->ID_EX._decodedDST);
            // fprintf(_mc->_debugLog, "_subregOperand = %d\n",_mc->ID_EX._subregOperand);
            // fprintf(_mc->_debugLog, "_MAR = %d\n",_mc->ID_EX._MAR);
            // fprintf(_mc->_debugLog, "_opResultHi = %d\n",_mc->ID_EX._opResultHi);
            // fprintf(_mc->_debugLog, "_opResultLo = %d\n",_mc->ID_EX._opResultLo);
            // fprintf(_mc->_debugLog, "_memControl = %d\n",_mc->ID_EX._memControl);
            // fprintf(_mc->_debugLog, "_writeREG = %d\n",_mc->ID_EX._writeREG);
            // fprintf(_mc->_debugLog, "_writeFREG = %d\n",_mc->ID_EX._writeFREG);
            // fprintf(_mc->_debugLog, "_branchOffset = %d\n",_mc->ID_EX._branchOffset);
            // fprintf(_mc->_debugLog, "_hiWPort = %d\n",_mc->ID_EX._hiWPort);
            // fprintf(_mc->_debugLog, "_loWPort = %d\n",_mc->ID_EX._loWPort);
            // fprintf(_mc->_debugLog, "_decodedShiftAmt = %d\n",_mc->ID_EX._decodedShiftAmt);
            // fprintf(_mc->_debugLog, "_hi = %d\n",_mc->ID_EX._hi);
            // fprintf(_mc->_debugLog, "_lo = %d\n",_mc->ID_EX._lo);
            // fprintf(_mc->_debugLog, "_lastbd = %d\n",_mc->ID_EX._lastbd);
            // fprintf(_mc->_debugLog, "_btaken = %d\n",_mc->ID_EX._btaken);
            // fprintf(_mc->_debugLog, "_bd = %d\n",_mc->ID_EX._bd);
            // fprintf(_mc->_debugLog, "_btgt = %d\n",_mc->ID_EX._btgt);
             // fprintf(_mc->_debugLog, "_isSyscall = %d\n",isSyscall);
            // fprintf(_mc->_debugLog, "_isIllegalOp = %d\n",_mc->ID_EX._isIllegalOp);
#endif
         }
         else if (isSyscall) {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Deferring execution of syscall ins %#x\n", SIM_TIME, ins);
#endif
         }
         else {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Illegal ins %#x in execution stage at PC %#x\n", SIM_TIME, ins, _mc->_pc);
#endif

         }

         btaken = _mc->EX_MEM._btaken;
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> [EXEC]Branch Taken =  %#x\n", SIM_TIME, btaken );
#endif
         if (!isIllegalOp && !isSyscall) {
            if(!_mc->system_call_in_pipe){  
               if (btaken)
               {
                  _mc->_pc = btgt;
               }
               else
               {
                  _mc->_pc = _mc->_pc + 4;
               }
            }
      }
             
   AWAIT_P_PHI1; // @negedge
      
      _mc->EX_MEM._pc = pc;
      _mc->EX_MEM._ins = ins;
      _mc->EX_MEM._memControl = memControl;
      _mc->EX_MEM._memOp = memOp;
      _mc->EX_MEM._writeREG=writeREG;
      _mc->EX_MEM._writeFREG=writeFREG;
      _mc->EX_MEM._loWPort=loWPort;
      _mc->EX_MEM._hiWPort=hiWPort;
      _mc->EX_MEM._decodedDST=decodedDST;
      _mc->EX_MEM._isSyscall = isSyscall;
      _mc->EX_MEM._isIllegalOp=isIllegalOp;
      _mc->EX_MEM._opControl=opControl;
      _mc->EX_MEM._btaken = btaken;
      }
}
