#include "mips.h"
#include <assert.h>
#include "mips-irix5.h"
#include "iostream.h"

Mipc::Mipc (Mem *m) : _l('M')
{
   _mem = m;
   _sys = new MipcSysCall (this);	// Allocate syscall layer

#ifdef MIPC_DEBUG
   _debugLog = fopen("mipc.debug", "w");
   assert(_debugLog != NULL);
#endif
   
   Reboot (ParamGetString ("Mipc.BootROM"));
}

Mipc::~Mipc (void)
{

}

void 
Mipc::MainLoop (void)
{
   LL addr;
   unsigned int ins;	// Local instruction register

   Assert (_boot, "Mipc::MainLoop() called without boot?");

   _nfetched = 0;

   while (!_sim_exit) {
    AWAIT_P_PHI0;
    AWAIT_P_PHI1;	// @posedge
    addr = _pc;
    ins = _mem->BEGetWord (addr, _mem->Read(addr & ~(LL)0x7));
    // AWAIT_P_PHI1;	// @negedge
#ifdef MIPC_DEBUG
      fprintf(_debugLog, "<%llu> Fetched ins %#x from PC %#x\n", SIM_TIME, ins, _pc);
      // fprintf(_debugLog, "_ins = %d\n",_ins);
      // fprintf(_debugLog, "_decodedSRC1 = %d\n",_decodedSRC1);
      // fprintf(_debugLog, "_decodedSRC2 = %d\n",_decodedSRC2);
      // fprintf(_debugLog, "_decodedDST = %d\n",_decodedDST);
      // fprintf(_debugLog, "_subregOperand = %d\n",_subregOperand);
      // fprintf(_debugLog, "_MAR = %d\n",_MAR);
      // fprintf(_debugLog, "_opResultHi = %d\n",_opResultHi);
      // fprintf(_debugLog, "_opResultLo = %d\n",_opResultLo);
      // fprintf(_debugLog, "_memControl = %d\n",_memControl);
      // fprintf(_debugLog, "_writeREG = %d\n",_writeREG);
      // fprintf(_debugLog, "_writeFREG = %d\n",_writeFREG);
      // fprintf(_debugLog, "_branchOffset = %d\n",_branchOffset);
      // fprintf(_debugLog, "_hiWPort = %d\n",_hiWPort);
      // fprintf(_debugLog, "_loWPort = %d\n",_loWPort);
      // fprintf(_debugLog, "_decodedShiftAmt = %d\n",_decodedShiftAmt);
      // fprintf(_debugLog, "_hi = %d\n",_hi);
      // fprintf(_debugLog, "_lo = %d\n",_lo);
      // fprintf(_debugLog, "_lastbd = %d\n",_lastbd);
      // fprintf(_debugLog, "_btaken = %d\n",_btaken);
      // fprintf(_debugLog, "_bd = %d\n",_bd);
      // fprintf(_debugLog, "_btgt = %d\n",_btgt);
      // fprintf(_debugLog, "_isSyscall = %d\n",_isSyscall);
      // fprintf(_debugLog, "_isIllegalOp = %d\n",_isIllegalOp);
#endif
    IF_ID._ins = ins;
    IF_ID._pc = _pc;
    _nfetched++;
    IF_ID._bd = 0;
   }

   MipcDumpstats();
   Log::CloseLog();
   
#ifdef MIPC_DEBUG
   assert(_debugLog != NULL);
   fclose(_debugLog);
#endif

   exit(0);
}

void
Mipc::MipcDumpstats()
{
  Log l('*');
  l.startLogging = 0;

  l.print ("");
  l.print ("************************************************************");
  l.print ("");
  l.print ("Number of instructions: %llu", _nfetched);
  l.print ("Number of simulated cycles: %llu", SIM_TIME);
  l.print ("CPI: %.2f", ((double)SIM_TIME)/_nfetched);
  l.print ("Int Conditional Branches: %llu", _num_cond_br);
  l.print ("Jump and Link: %llu", _num_jal);
  l.print ("Jump Register: %llu", _num_jr);
  l.print ("Number of fp instructions: %llu", _fpinst);
  l.print ("Number of loads: %llu", _num_load);
  l.print ("Number of syscall emulated loads: %llu", _sys->_num_load);
  l.print ("Number of stores: %llu", _num_store);
  l.print ("Number of syscall emulated stores: %llu", _sys->_num_store);
  l.print ("");

}

void 
Mipc::fake_syscall (unsigned int ins)
{
   _sys->pc = _pc;
   _sys->quit = 0;
   _sys->EmulateSysCall ();
   if (_sys->quit)
      _sim_exit = 1;
}

/*------------------------------------------------------------------------
 *
 *  Mipc::Reboot --
 *
 *   Reset processor state
 *
 *------------------------------------------------------------------------
 */
void 
Mipc::Reboot (char *image)
{
   FILE *fp;
   Log l('*');

   _boot = 0;

   if (image) {
      _boot = 1;
      printf ("Executing %s\n", image);
      fp = fopen (image, "r");
      if (!fp) {
	 fatal_error ("Could not open `%s' for booting host!", image);
      }
      _mem->ReadImage(fp);
      fclose (fp);

      // Reset state
      WB_._ins = 0;
      _num_load = 0;
      _num_store = 0;
      _fpinst = 0;
      _num_cond_br = 0;
      _num_jal = 0;
      _num_jr = 0;

      WB_._lastbd = 0;
      WB_._bd = 0;
      WB_._btaken = 0;
      WB_._btgt = 0xdeadbeef;
      _sim_exit = 0;
      _pc = ParamGetInt ("Mipc.BootPC");	// Boom! GO
   }
}

LL
MipcSysCall::GetDWord(LL addr)
{
   _num_load++;      
   return m->Read (addr);
}

void
MipcSysCall::SetDWord(LL addr, LL data)
{
  
   m->Write (addr, data);
   _num_store++;
}

Word 
MipcSysCall::GetWord (LL addr) 
{ 
  
   _num_load++;   
   return m->BEGetWord (addr, m->Read (addr & ~(LL)0x7)); 
}

void 
MipcSysCall::SetWord (LL addr, Word data) 
{ 
  
   m->Write (addr & ~(LL)0x7, m->BESetWord (addr, m->Read(addr & ~(LL)0x7), data)); 
   _num_store++;
}
  
void 
MipcSysCall::SetReg (int reg, LL val) 
{ 
   _ms->_gpr[reg] = val; 
}

LL 
MipcSysCall::GetReg (int reg) 
{
   return _ms->_gpr[reg]; 
}

LL
MipcSysCall::GetTime (void)
{
  return SIM_TIME;
}

pipeline_reg::pipeline_reg(void){
   _ins = 0;   // instruction register
   _decodedSRC1 = 0; 
   _decodedSRC2 = 0;   // Reg fetch output (source values)
   _decodedDST = 0;         // Decoder output (dest reg no)
   _subregOperand = 0;         // Needed for lwl and lwr
   _MAR = 0;          // Memory address register
   _opResultHi = 0; _opResultLo = 0;  // Result of operation
   _memControl = FALSE;         // Memory instruction?
   _writeREG = FALSE; _writeFREG = FALSE;     // WB control
   _branchOffset = 0;
   _hiWPort = FALSE; _loWPort = FALSE;     // WB control
   _decodedShiftAmt = 0;    // Shift amount
   _hi = 0; _lo = 0;        // mult, div destination
   _lastbd = 0;         // branch delay state
   _btaken = 0;          // taken branch (1 if taken, 0 if fall-through)
   _bd = 0;           // 1 if the next ins is delay slot
   _btgt = 0;            // branch target

   _isSyscall = FALSE;       // 1 if system call
   _isIllegalOp = FALSE;        // 1 if illegal opcode

   _opControl = NULL;
}
pipeline_reg::~pipeline_reg(void){
}
