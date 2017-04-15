#include "executor.h"
#include "iostream.h"
Exe::Exe (Mipc *mc)
{
   _mc = mc;
}

Exe::~Exe (void) {}

void load_interlock(Mipc *mc){
   mc->ID_EX.reset();
   mc->set_pc_3 = mc->_pc;
   mc->set_pc_2 = mc->IF_ID._pc;
   mc->set_ins = mc->IF_ID._ins;
}

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
   int source1, source2;
   unsigned mar_;
   bool temp_isstore;

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
         opControl=_mc->ID_EX._opControl;
         lastbd = _mc->ID_EX._lastbd;
         bd =_mc->ID_EX._bd;
         temp_isstore = _mc->ID_EX.isstore;
         _mc->b_taken = 0;
         _mc->temppc_ = pc;

         source1 = _mc->ID_EX.src1reg;
         source2 = _mc->ID_EX.src2reg;           
         if(_mc->prev_isLoad){
            if(source1 == _mc->prev1DST ){
               load_interlock(_mc);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "LOAD INTERLOCK\n");
#endif            
            }
            else if(source2 == _mc->prev1DST){
               load_interlock(_mc);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "LOAD INTERLOCK\n");
#endif
            }    
         }
         bool flag = FALSE;
         if(!_mc->prevMEM){
            if(source1 == _mc->prev1DST ){
               flag = TRUE;
               _mc->ID_EX._decodedSRC1 = _mc->EX_MEM._opResultLo;
               if(_mc->toUpdateBranch){
                   _mc->ID_EX._btgt = _mc->ID_EX._decodedSRC1;
               }
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "EX-EX bypass from %d to %d\n",  _mc->prev1DST,source1);
#endif            
            }
            else if(source2 == _mc->prev1DST){
               flag = TRUE;
               _mc->ID_EX._decodedSRC2 = _mc->EX_MEM._opResultLo;
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "EX-EX bypass from %d to %d\n", _mc->prev1DST,source2);
#endif
            }  
         }
         if(!_mc->isStore3 && !flag){
            if(source1 == _mc->prev2DST){
               _mc->ID_EX._decodedSRC1 = _mc->MEM_WB._opResultLo;
               if(_mc->toUpdateBranch){
                   _mc->ID_EX._btgt = _mc->MEM_WB._decodedSRC1;
               }
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "MEM-EX bypass from %d to %d\n", _mc->prev2DST,source1);
#endif            
            }
            else if(source2 == _mc->prev2DST){
               _mc->ID_EX._decodedSRC2 = _mc->MEM_WB._opResultLo;
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "MEM-EX bypass from %d to %d\n",_mc->prev2DST,source2);
#endif
            }  
         }        
         btgt = _mc->ID_EX._btgt;
         if (!isSyscall && !isIllegalOp) {
            if (_mc->ID_EX._opControl != NULL)
               _mc->ID_EX._opControl(_mc,ins);
#ifdef MIPC_DEBUG
            fprintf(_mc->_debugLog, "<%llu> Executed ins %#x OutPut = %#x\n", SIM_TIME, ins,_mc->opRLo);
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

         btaken = _mc->b_taken;
         decodedDST=_mc->ID_EX._decodedDST;
         mar_ = _mc->mar;
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
      _mc->EX_MEM._opResultHi = _mc->opRHi;
      _mc->EX_MEM._opResultLo = _mc->opRLo;
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
      _mc->EX_MEM._MAR = mar_;
      _mc->EX_MEM.isstore = temp_isstore;
      _mc->EX_MEM.src1reg = source1;
      }
}
