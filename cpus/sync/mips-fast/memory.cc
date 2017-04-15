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
    Bool flag;
   unsigned decodedDST;
   unsigned opResultLo, opResultHi;
   unsigned temp_prev1DST;
   void (*opControl)(Mipc*, unsigned);
      bool temp_isstore, temp_memwb;
         void (*memOp)(Mipc*);

   while (1) {
      AWAIT_P_PHI0;       // @posedge
         _mc->flag_toChangeOpresult = FALSE;
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
         _mc->tempMAR = _mc->EX_MEM._MAR;
         _mc->tempSubregOperand = _mc->EX_MEM._subregOperand;
         _mc->tempDecodedDST =_mc->EX_MEM._decodedDST;
         temp_isstore = _mc->EX_MEM.isstore;
         temp_memwb =  _mc->MEM_WB._opResultLo;
         memOp = _mc->EX_MEM._memOp;
         temp_prev1DST = _mc->prev2DST;
         unsigned temp =  _mc->_gpr[_mc->tempDecodedDST];
         flag = FALSE;
         AWAIT_P_PHI1;  // @negedge   
         if(temp_isstore){
            if(_mc->tempDecodedDST == temp_prev1DST){
               _mc->_gpr[_mc->tempDecodedDST] = temp_memwb;
               flag = TRUE;
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "MEM-MEM bypass %d to %d\n",_mc->tempDecodedDST, temp_prev1DST);
#endif  
            }
         }

         if (memControl) {
             memOp(_mc);

#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Accessing memory at address %#x for ins %#x\n", SIM_TIME, _mc->tempMAR, ins);
#endif
         }
         else {
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Memory has nothing to do for ins %#x\n", SIM_TIME, ins);

#endif
         }

      _mc->MEM_WB._pc = pc;
      _mc->MEM_WB._ins = ins;
      _mc->MEM_WB._writeREG=writeREG;
      _mc->MEM_WB._writeFREG=writeFREG;
      _mc->MEM_WB._loWPort=loWPort;
      _mc->MEM_WB._hiWPort=hiWPort;
      _mc->MEM_WB._decodedDST=decodedDST;
      if(!_mc->flag_toChangeOpresult){
         _mc->MEM_WB._opResultLo=opResultLo;
      }
      _mc->MEM_WB._opResultHi=opResultHi;
      _mc->MEM_WB._isSyscall=isSyscall;
      _mc->MEM_WB._isIllegalOp=isIllegalOp;
      _mc->MEM_WB._opControl=opControl;
      if (flag){
         _mc->_gpr[_mc->tempDecodedDST] = temp;
      }
   }
}
