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
      if (_mc->load_lock){
            ins = _mc->set_ins;
            pc = _mc->set_pc_2;
      }
      else{
            ins = _mc->IF_ID._ins;
            pc =  _mc->IF_ID._pc;
      }
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

#endif
      }

}
