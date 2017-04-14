#include "wb.h"
#include "iostream.h"
Writeback::Writeback (Mipc *mc)
{
   _mc = mc;
}

Writeback::~Writeback (void) {}

void
Writeback::MainLoop (void)
{
   unsigned int ins, pc;
   Bool writeReg;
   Bool writeFReg;
   Bool loWPort;
   Bool hiWPort;
   Bool isSyscall;
   Bool isIllegalOp;
   unsigned decodedDST;
   unsigned opResultLo, opResultHi;
   void (*opControl)(Mipc*, unsigned);

   while (1) {
      AWAIT_P_PHI0;	// @posedge
      // Sample the important signals
         pc =  _mc->MEM_WB._pc;
         writeReg = _mc->MEM_WB._writeREG;
         writeFReg = _mc->MEM_WB._writeFREG;
         loWPort = _mc->MEM_WB._loWPort;
         hiWPort = _mc->MEM_WB._hiWPort;
         decodedDST = _mc->MEM_WB._decodedDST;
         opResultLo = _mc->MEM_WB._opResultLo;
         opResultHi = _mc->MEM_WB._opResultHi;
         isIllegalOp = _mc->MEM_WB._isIllegalOp;
         isSyscall = _mc->MEM_WB._isSyscall;
         opControl = _mc->MEM_WB._opControl;
         ins = _mc->MEM_WB._ins;
         
         if (isSyscall) {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> SYSCALL! Trapping to emulation layer at PC %#x, ins %#x\n", SIM_TIME, pc, ins);
#endif      
            opControl(_mc, ins);
            _mc->set_pc  = pc + 4;
            _mc->system_call_in_pipe = FALSE;
         }
         else if (isIllegalOp) {
            printf("Illegal ins %#x at PC %#x. Terminating simulation!\n", ins, pc);
#ifdef MIPC_DEBUG
            fclose(_mc->_debugLog);
#endif
            printf("Register state on termination:\n\n");
            _mc->dumpregs();
            exit(0);
         }
         else {
            if (writeReg) {
               _mc->_gpr[decodedDST] = opResultLo;
#ifdef MIPC_DEBUG
               fprintf(_mc->_debugLog, "<%llu> Writing to reg %u, value: %#x, ins %#x\n", SIM_TIME, decodedDST, opResultLo,ins);
#endif
            }
            else if (writeFReg) {
               _mc->_fpr[(decodedDST)>>1].l[FP_TWIDDLE^((decodedDST)&1)] = opResultLo;
#ifdef MIPC_DEBUG
               fprintf(_mc->_debugLog, "<%llu> Writing to freg %u, value: %#x\n", SIM_TIME, decodedDST>>1, opResultLo);
#endif
            }
            else if (loWPort || hiWPort) {
               if (loWPort) {
                  _mc->EX_MEM._lo = opResultLo;
#ifdef MIPC_DEBUG
                  fprintf(_mc->_debugLog, "<%llu> Writing to Lo, value: %#x\n", SIM_TIME, opResultLo);
#endif
               }
               if (hiWPort) {
                  _mc->EX_MEM._hi = opResultHi;
#ifdef MIPC_DEBUG
                  fprintf(_mc->_debugLog, "<%llu> Writing to Hi, value: %#x\n", SIM_TIME, opResultHi);
#endif
               }
            }
         }
         _mc->_gpr[0] = 0;
         AWAIT_P_PHI1;       // @negedge

      }
   
}
