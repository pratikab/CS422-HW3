#include "executor.h"

Exe::Exe (Mipc *mc)
{
   _mc = mc;
}

Exe::~Exe (void) {}

void
Exe::MainLoop (void)
{
   unsigned int ins;
   Bool isSyscall, isIllegalOp;

   while (1) {
      AWAIT_P_PHI0;	// @posedge
         ins = _mc->ID_EX._ins;
         isSyscall = _mc->ID_EX._isSyscall;
         isIllegalOp = _mc->ID_EX._isIllegalOp;
         // AWAIT_P_PHI1;	// @negedge
         if (!isSyscall && !isIllegalOp) {
            _mc->ID_EX._opControl(_mc,ins);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Executed ins %#x\n", SIM_TIME, ins);
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

         if (!isIllegalOp && !isSyscall) {
            if (_mc->ID_EX._lastbd && _mc->ID_EX._btaken)
            {
               _mc->_pc = _mc->ID_EX._btgt;
            }
            else
            {
               _mc->_pc = _mc->_pc + 4;
            }
            _mc->EX_MEM._lastbd = _mc->ID_EX._bd; ////////////DOUBT
         }
      }
      _mc->EX_MEM._pc = _mc->_pc;
      _mc->EX_MEM._ins = ins;
      _mc->EX_MEM._memControl = _mc->ID_EX._memControl;
      _mc->EX_MEM._memOp = _mc->ID_EX._memOp;
   
}
