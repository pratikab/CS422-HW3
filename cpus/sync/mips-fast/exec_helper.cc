#include <math.h>
#include "mips.h"
#include "opcodes.h"
#include <assert.h>
#include "app_syscall.h"
#include "iostream.h"

/*------------------------------------------------------------------------
 *
 *  Instruction exec 
 *
 *------------------------------------------------------------------------
 */



void
Mipc::Dec (unsigned int ins)
{
   MipsInsn i;
   signed int a1, a2;
   unsigned int ar1, ar2, s1, s2, r1, r2, t1, t2;
   LL addr;
   unsigned int val;
   LL value, mask;
   int sa,j;
   Word dummy;
   ID_EX.src1reg = 100;
   ID_EX.src2reg = 100;

   ID_EX._isIllegalOp = FALSE;
   ID_EX._isSyscall = FALSE;
   isLoad = FALSE;
   load_lock = FALSE;
   toUpdateBranch = FALSE;
   isStore = FALSE;

   i.data = ins;
  
#define SIGN_EXTEND_BYTE(x)  do { x <<= 24; x >>= 24; } while (0)
#define SIGN_EXTEND_IMM(x)   do { x <<= 16; x >>= 16; } while (0)
   switch (i.reg.op) {
   case 0:
      // SPECIAL (ALU format)
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = _gpr[i.reg.rt];
      ID_EX.src2reg = i.reg.rt;
      ID_EX._decodedDST = i.reg.rd;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;

      switch (i.reg.func) {
      case 0x20:			// add
      case 0x21:			// addu
         ID_EX._opControl = func_add_addu;
	 break;

      case 0x24:			// and
         ID_EX._opControl = func_and;
	 break;

      case 0x27:			// nor
         ID_EX._opControl = func_nor;
	 break;

      case 0x25:			// or
         ID_EX._opControl = func_or;
	 break;

      case 0:			// sll
         ID_EX._opControl = func_sll;
         ID_EX._decodedShiftAmt = i.reg.sa;
	 break;

      case 4:			// sllv
         ID_EX._opControl = func_sllv;
	 break;

      case 0x2a:			// slt
         ID_EX._opControl = func_slt;
	 break;

      case 0x2b:			// sltu
         ID_EX._opControl = func_sltu;
	 break;

      case 0x3:			// sra
         ID_EX._opControl = func_sra;
         ID_EX._decodedShiftAmt = i.reg.sa;
	 break;

      case 0x7:			// srav
         ID_EX._opControl = func_srav;
	 break;

      case 0x2:			// srl
         ID_EX._opControl = func_srl;
         ID_EX._decodedShiftAmt = i.reg.sa;
	 break;

      case 0x6:			// srlv
         ID_EX._opControl = func_srlv;
	 break;

      case 0x22:			// sub
      case 0x23:			// subu
	 // no overflow check
         ID_EX._opControl = func_sub_subu;
	 break;

      case 0x26:			// xor
         ID_EX._opControl = func_xor;
	 break;

      case 0x1a:			// div
         ID_EX._opControl = func_div;
         ID_EX._hiWPort = TRUE;
         ID_EX._loWPort = TRUE;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = FALSE;
	 break;

      case 0x1b:			// divu
         ID_EX._opControl = func_divu;
         ID_EX._hiWPort = TRUE;
         ID_EX._loWPort = TRUE;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = FALSE;
	 break;

      case 0x10:			// mfhi
         ID_EX._opControl = func_mfhi;
	 break;

      case 0x12:			// mflo
         ID_EX._opControl = func_mflo;
	 break;

      case 0x11:			// mthi
         ID_EX._opControl = func_mthi;
         ID_EX._hiWPort = TRUE;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = FALSE;
	 break;

      case 0x13:			// mtlo
         ID_EX._opControl = func_mtlo;
         ID_EX._loWPort = TRUE;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = FALSE;
	 break;

      case 0x18:			// mult
         ID_EX._opControl = func_mult;
         ID_EX._hiWPort = TRUE;
         ID_EX._loWPort = TRUE;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = FALSE;
	 break;

      case 0x19:			// multu
         ID_EX._opControl = func_multu;
         ID_EX._hiWPort = TRUE;
         ID_EX._loWPort = TRUE;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = FALSE;
	 break;

      case 9:			// jalr
         ID_EX._opControl = func_jalr;
         toUpdateBranch = TRUE;
         ID_EX._btgt = ID_EX._decodedSRC1;
         ID_EX._bd = 1;
         break;

      case 8:			// jr
         ID_EX._opControl = func_jr;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = FALSE;
         ID_EX._btgt = ID_EX._decodedSRC1;
         toUpdateBranch = TRUE;
         ID_EX._bd = 1;
	 break;

      case 0xd:			// await/break
         ID_EX._opControl = func_await_break;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = FALSE;
	 break;

      case 0xc:			// syscall
         ID_EX._opControl = func_syscall;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = FALSE;
         ID_EX._isSyscall = TRUE;
	 break;

      default:
	        ID_EX._isIllegalOp = TRUE;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = FALSE;
	 break;
      }
      break;	// ALU format

   case 8:			// addi
   case 9:			// addiu
      // ignore overflow: no exceptions
      ID_EX._opControl = func_addi_addiu;
      ID_EX._decodedSRC1 = _gpr[i.imm.rs];
      ID_EX.src1reg = i.imm.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.imm.rt;
      ID_EX._writeREG = TRUE;
       ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      break;

   case 0xc:			// andi
      ID_EX._opControl = func_andi;
      ID_EX._decodedSRC1 = _gpr[i.imm.rs];
      ID_EX.src1reg = i.imm.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.imm.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      break;

   case 0xf:			// lui
      ID_EX._opControl = func_lui;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.imm.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      break;

   case 0xd:			// ori
      ID_EX._opControl = func_ori;
      ID_EX._decodedSRC1 = _gpr[i.imm.rs];
      ID_EX.src1reg = i.imm.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.imm.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      break;

   case 0xa:			// slti
      ID_EX._opControl = func_slti;
      ID_EX._decodedSRC1 = _gpr[i.imm.rs];
      ID_EX.src1reg = i.imm.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.imm.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      break;

   case 0xb:			// sltiu
      ID_EX._opControl = func_sltiu;
      ID_EX._decodedSRC1 = _gpr[i.imm.rs];
      ID_EX.src1reg = i.imm.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.imm.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      break;

   case 0xe:			// xori
      ID_EX._opControl = func_xori;
      ID_EX._decodedSRC1 = _gpr[i.imm.rs];
      ID_EX.src1reg = i.imm.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.imm.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      break;

   case 4:			// beq
      ID_EX._opControl = func_beq;
      ID_EX._decodedSRC1 = _gpr[i.imm.rs];
      ID_EX.src1reg = i.imm.rs;
      ID_EX._decodedSRC2 = _gpr[i.imm.rt];
      ID_EX._branchOffset = i.imm.imm;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      ID_EX._branchOffset <<= 16; 
      ID_EX._branchOffset >>= 14; 
      ID_EX._bd = 1;
      ID_EX._btgt = (unsigned)((signed)_pc+ID_EX._branchOffset+4);
      break;

   case 1:
      // REGIMM
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;      
      ID_EX._branchOffset = i.imm.imm;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;

      switch (i.reg.rt) {
      case 1:			// bgez
         ID_EX._opControl = func_bgez;
         ID_EX._branchOffset <<= 16;
         ID_EX._branchOffset >>= 14;
         ID_EX._bd = 1;
         ID_EX._btgt = (unsigned)((signed)_pc+ID_EX._branchOffset+4);
	 break;

      case 0x11:			// bgezal
         ID_EX._opControl = func_bgezal;
         ID_EX._decodedDST = 31;
         ID_EX._writeREG = TRUE;
         ID_EX._branchOffset <<= 16;
         ID_EX._branchOffset >>= 14;
         ID_EX._bd = 1;
         ID_EX._btgt = (unsigned)((signed)_pc+ID_EX._branchOffset+4);
	 break;

      case 0x10:			// bltzal
         ID_EX._opControl = func_bltzal;
         ID_EX._decodedDST = 31;
         ID_EX._writeREG = TRUE;
         ID_EX._branchOffset <<= 16;
         ID_EX._branchOffset >>= 14;
         ID_EX._bd = 1;
         ID_EX._btgt = (unsigned)((signed)_pc+ID_EX._branchOffset+4);
	 break;

      case 0x0:			// bltz
         ID_EX._opControl = func_bltz;
         ID_EX._branchOffset <<= 16;
         ID_EX._branchOffset >>= 14;
         ID_EX._bd = 1;
         ID_EX._btgt = (unsigned)((signed)_pc+ID_EX._branchOffset+4);
	 break;

      default:
	 ID_EX._isIllegalOp = TRUE;
	 break;
      }
      break;

   case 7:			// bgtz
      ID_EX._opControl = func_bgtz;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._branchOffset = i.imm.imm;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      ID_EX._branchOffset <<= 16;
      ID_EX._branchOffset >>= 14;
      ID_EX._bd = 1;
      ID_EX._btgt = (unsigned)((signed)_pc+ID_EX._branchOffset+4);
      break;

   case 6:			// blez
      ID_EX._opControl = func_blez;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._branchOffset = i.imm.imm;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      ID_EX._branchOffset <<= 16;
      ID_EX._branchOffset >>= 14;
      ID_EX._bd = 1;
      ID_EX._btgt = (unsigned)((signed)_pc+ID_EX._branchOffset+4);
      break;

   case 5:			// bne
      ID_EX._opControl = func_bne;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = _gpr[i.reg.rt];
      ID_EX.src2reg = i.reg.rt;
      ID_EX._branchOffset = i.imm.imm;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      ID_EX._branchOffset <<= 16;
      ID_EX._branchOffset >>= 14;
      ID_EX._bd = 1;
      ID_EX._btgt = (unsigned)((signed)_pc+ID_EX._branchOffset+4);
      break;

   case 2:			// j
      ID_EX._opControl = func_j;
      ID_EX._branchOffset = i.tgt.tgt;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      ID_EX._btgt = ((_pc+4) & 0xf0000000) | (ID_EX._branchOffset<<2);
      ID_EX._bd = 1;
      break;

   case 3:			// jal
      ID_EX._opControl = func_jal;
      ID_EX._branchOffset = i.tgt.tgt;
      ID_EX._decodedDST = 31;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      ID_EX._btgt = ((_pc+4) & 0xf0000000) | (ID_EX._branchOffset<<2);
      ID_EX._bd = 1;
      break;

   case 0x20:			// lb  
      ID_EX._opControl = func_lb;
      ID_EX._memOp = mem_lb;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isLoad = TRUE;
      break;

   case 0x24:			// lbu
      ID_EX._opControl = func_lbu;
      ID_EX._memOp = mem_lbu;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isLoad = TRUE;
      break;

   case 0x21:			// lh
      ID_EX._opControl = func_lh;
      ID_EX._memOp = mem_lh;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isLoad = TRUE;
      break;

   case 0x25:			// lhu
      ID_EX._opControl = func_lhu;
      ID_EX._memOp = mem_lhu;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isLoad = TRUE;
      break;

   case 0x22:			// lwl
      ID_EX._opControl = func_lwl;
      ID_EX._memOp = mem_lwl;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._subregOperand = _gpr[i.reg.rt];
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isLoad = TRUE;
      break;

   case 0x23:			// lw
      ID_EX._opControl = func_lw;
      ID_EX._memOp = mem_lw;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isLoad = TRUE;
      break;

   case 0x26:			// lwr
      ID_EX._opControl = func_lwr;
      ID_EX._memOp = mem_lwr;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._subregOperand = _gpr[i.reg.rt];
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = TRUE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isLoad = TRUE;
      break;

   case 0x31:			// lwc1
      ID_EX._opControl = func_lwc1;
      ID_EX._memOp = mem_lwc1;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = TRUE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isLoad = TRUE;
      break;

   case 0x39:			// swc1
      ID_EX._opControl = func_swc1;
      ID_EX._memOp = mem_swc1;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isStore = TRUE;
      break;

   case 0x28:			// sb
      ID_EX._opControl = func_sb;
      ID_EX._memOp = mem_sb;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isStore = TRUE;

      break;

   case 0x29:			// sh  store half word
      ID_EX._opControl = func_sh;
      ID_EX._memOp = mem_sh;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isStore = TRUE;

      break;

   case 0x2a:			// swl
      ID_EX._opControl = func_swl;
      ID_EX._memOp = mem_swl;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isStore = TRUE;

      break;

   case 0x2b:			// sw
      ID_EX._opControl = func_sw;
      ID_EX._memOp = mem_sw;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isStore = TRUE;

      break;

   case 0x2e:			// swr
      ID_EX._opControl = func_swr;
      ID_EX._memOp = mem_swr;
      ID_EX._decodedSRC1 = _gpr[i.reg.rs];
      ID_EX.src1reg = i.reg.rs;
      ID_EX._decodedSRC2 = i.imm.imm;
      ID_EX._decodedDST = i.reg.rt;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = TRUE;
      isStore = TRUE;

      break;

   case 0x11:			// floating-point
      _fpinst++;
      switch (i.freg.fmt) {
      case 4:			// mtc1
         ID_EX._opControl = func_mtc1;
         ID_EX._decodedSRC1 = _gpr[i.freg.ft];
         ID_EX._decodedDST = i.freg.fs;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = TRUE;
         ID_EX._hiWPort = FALSE;
         ID_EX._loWPort = FALSE;
         ID_EX._memControl = FALSE;
	 break;

      case 0:			// mfc1
         ID_EX._opControl = func_mfc1;
         ID_EX._decodedSRC1 = _fpr[(i.freg.fs)>>1].l[FP_TWIDDLE^((i.freg.fs)&1)];
         ID_EX._decodedDST = i.freg.ft;
         ID_EX._writeREG = TRUE;
         ID_EX._writeFREG = FALSE;
         ID_EX._hiWPort = FALSE;
         ID_EX._loWPort = FALSE;
         ID_EX._memControl = FALSE;
	 break;
      default:
         ID_EX._isIllegalOp = TRUE;
         ID_EX._writeREG = FALSE;
         ID_EX._writeFREG = FALSE;
         ID_EX._hiWPort = FALSE;
         ID_EX._loWPort = FALSE;
         ID_EX._memControl = FALSE;
	 break;
      }
      break;
   default:
      ID_EX._isIllegalOp = TRUE;
      ID_EX._writeREG = FALSE;
      ID_EX._writeFREG = FALSE;
      ID_EX._hiWPort = FALSE;
      ID_EX._loWPort = FALSE;
      ID_EX._memControl = FALSE;
      break;
   }
   prev2DST = prev1DST;
   prev1DST = currDST;
   currDST = ID_EX._decodedDST;
   prev_isLoad = currLoad; 
   currLoad = isLoad;
   prevMEM = currMEM;
   currMEM = ID_EX._memControl;
   isStore3 = isStore2;
   isStore2 = isStore1;
   isStore1 = isStore;

// #ifdef MIPC_DEBUG
//             // fprintf(_debugLog, "***subreg1 = %d *** subreg2 = %d ** %#x\n",ID_EX.src1reg,ID_EX.src2reg,currDST);

// #endif
// #ifdef MIPC_DEBUG
//             fprintf(_debugLog, " ********Current Load instruction status %d ** %d ** %d\n", currLoad,prev_isLoad);
//             // fprintf(_debugLog, "<%llu> Previous Memory instruction status %#x\n", SIM_TIME, prevMEM);

// #endif
}


/*
 *
 * Debugging: print registers
 *
 */
void 
Mipc::dumpregs (void)
{
   int i;

   printf ("\n--- PC = %08x ---\n", _pc);
   for (i=0; i < 32; i++) {
      if (i < 10)
	 printf (" r%d: %08x (%ld)\n", i, _gpr[i], _gpr[i]);
      else
	 printf ("r%d: %08x (%ld)\n", i, _gpr[i], _gpr[i]);
   }
   // printf ("taken: %d, bd: %d\n", _btaken, _bd);
   // printf ("target: %08x\n", );
}

void
Mipc::func_add_addu (Mipc *mc, unsigned ins)
{
   mc->opRLo = (unsigned)(mc->ID_EX._decodedSRC1 + mc->ID_EX._decodedSRC2);
}

void
Mipc::func_and (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC1 & mc->ID_EX._decodedSRC2;
}

void
Mipc::func_nor (Mipc *mc, unsigned ins)
{
   mc->opRLo = ~(mc->ID_EX._decodedSRC1 | mc->ID_EX._decodedSRC2);
}

void
Mipc::func_or (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC1 | mc->ID_EX._decodedSRC2;
}

void
Mipc::func_sll (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC2 << mc->ID_EX._decodedShiftAmt;
}

void
Mipc::func_sllv (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC2 << (mc->ID_EX._decodedSRC1);
}

void
Mipc::func_slt (Mipc *mc, unsigned ins)
{
   if (mc->ID_EX._decodedSRC1 < mc->ID_EX._decodedSRC2) {
      mc->opRLo = 1;
   }
   else {
      mc->opRLo = 0;
   }
}

void
Mipc::func_sltu (Mipc *mc, unsigned ins)
{
   if ((unsigned)mc->ID_EX._decodedSRC1 < (unsigned)mc->ID_EX._decodedSRC2) {
      mc->opRLo = 1;
   }
   else {
      mc->opRLo = 0;
   }
}

void
Mipc::func_sra (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC2 >> mc->ID_EX._decodedShiftAmt;
}

void
Mipc::func_srav (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC2 >> (mc->ID_EX._decodedSRC1 & 0x1f);
}

void
Mipc::func_srl (Mipc *mc, unsigned ins)
{
   mc->opRLo = (unsigned)mc->ID_EX._decodedSRC2 >> mc->ID_EX._decodedShiftAmt;
}

void
Mipc::func_srlv (Mipc *mc, unsigned ins)
{
   mc->opRLo = (unsigned)mc->ID_EX._decodedSRC2 >> (mc->ID_EX._decodedSRC1 & 0x1f);
}

void
Mipc::func_sub_subu (Mipc *mc, unsigned ins)
{
   mc->opRLo = (unsigned)mc->ID_EX._decodedSRC1 - (unsigned)mc->ID_EX._decodedSRC2;
}

void
Mipc::func_xor (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC1 ^ mc->ID_EX._decodedSRC2;
}

void
Mipc::func_div (Mipc *mc, unsigned ins)
{
   if (mc->ID_EX._decodedSRC2 != 0) {
      mc->opRHi = (unsigned)(mc->ID_EX._decodedSRC1 % mc->ID_EX._decodedSRC2);
      mc->opRLo = (unsigned)(mc->ID_EX._decodedSRC1 / mc->ID_EX._decodedSRC2);
   }
   else {
      mc->opRHi = 0x7fffffff;
      mc->opRLo = 0x7fffffff;
   }
}

void
Mipc::func_divu (Mipc *mc, unsigned ins)
{
   if ((unsigned)mc->ID_EX._decodedSRC2 != 0) {
      mc->opRHi = (unsigned)(mc->ID_EX._decodedSRC1) % (unsigned)(mc->ID_EX._decodedSRC2);
      mc->opRLo = (unsigned)(mc->ID_EX._decodedSRC1) / (unsigned)(mc->ID_EX._decodedSRC2);
   }
   else {
      mc->opRHi = 0x7fffffff;
      mc->opRLo = 0x7fffffff;
   }
}

void
Mipc::func_mfhi (Mipc *mc, unsigned ins)
{
   mc->opRHi = mc->ID_EX._hi;
}

void
Mipc::func_mflo (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._lo;
}

void
Mipc::func_mthi (Mipc *mc, unsigned ins)
{
   mc->opRHi = mc->ID_EX._decodedSRC1;
}

void
Mipc::func_mtlo (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC2;
}

void
Mipc::func_mult (Mipc *mc, unsigned ins)
{
   unsigned int ar1, ar2, s1, s2, r1, r2, t1, t2;
                                                                                
   ar1 = mc->ID_EX._decodedSRC1;
   ar2 = mc->ID_EX._decodedSRC2;
   s1 = ar1 >> 31; if (s1) ar1 = 0x7fffffff & (~ar1 + 1);
   s2 = ar2 >> 31; if (s2) ar2 = 0x7fffffff & (~ar2 + 1);
                                                                                
   t1 = (ar1 & 0xffff) * (ar2 & 0xffff);
   r1 = t1 & 0xffff;              // bottom 16 bits
                                                                                
   // compute next set of 16 bits
   t1 = (ar1 & 0xffff) * (ar2 >> 16) + (t1 >> 16);
   t2 = (ar2 & 0xffff) * (ar1 >> 16);
                                                                                
   r1 = r1 | (((t1+t2) & 0xffff) << 16); // bottom 32 bits
   r2 = (ar1 >> 16) * (ar2 >> 16) + (t1 >> 16) + (t2 >> 16) +
            (((t1 & 0xffff) + (t2 & 0xffff)) >> 16);
                                                                                
   if (s1 ^ s2) {
      r1 = ~r1;
      r2 = ~r2;
      r1++;
      if (r1 == 0)
         r2++;
   }
   mc->opRHi = r2;
   mc->opRLo = r1;
}

void
Mipc::func_multu (Mipc *mc, unsigned ins)
{
   unsigned int ar1, ar2, s1, s2, r1, r2, t1, t2;
                                                                                
   ar1 = mc->ID_EX._decodedSRC1;
   ar2 = mc->ID_EX._decodedSRC2;
                                                                                
   t1 = (ar1 & 0xffff) * (ar2 & 0xffff);
   r1 = t1 & 0xffff;              // bottom 16 bits
                                                                                
   // compute next set of 16 bits
   t1 = (ar1 & 0xffff) * (ar2 >> 16) + (t1 >> 16);
   t2 = (ar2 & 0xffff) * (ar1 >> 16);
                                                                                
   r1 = r1 | (((t1+t2) & 0xffff) << 16); // bottom 32 bits
   r2 = (ar1 >> 16) * (ar2 >> 16) + (t1 >> 16) + (t2 >> 16) +
            (((t1 & 0xffff) + (t2 & 0xffff)) >> 16);
                            
   mc->opRHi = r2;
   mc->opRLo = r1;                                                    
}

void
Mipc::func_jalr (Mipc *mc, unsigned ins)
{
   mc->b_taken = 1;
   mc->_num_jal++;
   mc->opRLo = mc->_pc + 8;
}

void
Mipc::func_jr (Mipc *mc, unsigned ins)
{
   mc->b_taken = 1;
   mc->_num_jr++;
}

void
Mipc::func_await_break (Mipc *mc, unsigned ins)
{
}

void
Mipc::func_syscall (Mipc *mc, unsigned ins)
{
   mc->fake_syscall (ins);
}

void
Mipc::func_addi_addiu (Mipc *mc, unsigned ins)
{
    SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
    mc->opRLo =(unsigned)(mc->ID_EX._decodedSRC1 + mc->ID_EX._decodedSRC2);
}

void
Mipc::func_andi (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC1 & mc->ID_EX._decodedSRC2;
}

void
Mipc::func_lui (Mipc *mc, unsigned ins)
{
  	mc->opRLo = (mc->ID_EX._decodedSRC2)<<16;
}

void
Mipc::func_ori (Mipc *mc, unsigned ins)
{
	mc->opRLo = (mc->ID_EX._decodedSRC1 | mc->ID_EX._decodedSRC2);   
}

void
Mipc::func_slti (Mipc *mc, unsigned ins)
{
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   if (mc->ID_EX._decodedSRC1 < mc->ID_EX._decodedSRC2) {
      mc->opRLo = 1;
   }
   else {
      mc->opRLo = 0;
   }
}

void
Mipc::func_sltiu (Mipc *mc, unsigned ins)
{
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   if ((unsigned)mc->ID_EX._decodedSRC1 < (unsigned)mc->ID_EX._decodedSRC2) {
      mc->opRLo = 1;
   }
   else {
      mc->opRLo = 0;
   }
}

void
Mipc::func_xori (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC1 ^ mc->ID_EX._decodedSRC2;
}

void
Mipc::func_beq (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->b_taken = (mc->ID_EX._decodedSRC1 == mc->ID_EX._decodedSRC2)? 1 : 0;   
}

void
Mipc::func_bgez (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->b_taken = !(mc->ID_EX._decodedSRC1 >> 31);
}

void
Mipc::func_bgezal (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->b_taken = !(mc->ID_EX._decodedSRC1 >> 31);
   mc->opRLo = mc->_pc + 8;
}

void
Mipc::func_bltzal (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->b_taken = (mc->ID_EX._decodedSRC1 >> 31);
   mc->opRLo = mc->_pc + 8;
}

void
Mipc::func_bltz (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->b_taken = (mc->ID_EX._decodedSRC1 >> 31);
}

void
Mipc::func_bgtz (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->b_taken = (mc->ID_EX._decodedSRC1 > 0);
}

void
Mipc::func_blez (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->b_taken = (mc->ID_EX._decodedSRC1 <= 0);
}

void
Mipc::func_bne (Mipc *mc, unsigned ins)
{
   mc->_num_cond_br++;
   mc->b_taken = (mc->ID_EX._decodedSRC1 != mc->ID_EX._decodedSRC2);
}

void
Mipc::func_j (Mipc *mc, unsigned ins)
{
   mc->b_taken = 1;
}

void
Mipc::func_jal (Mipc *mc, unsigned ins)
{
   	mc->_num_jal++;
	  mc->b_taken = 1;
      mc->opRLo = mc->_pc + 8;
}

void
Mipc::func_lb (Mipc *mc, unsigned ins)
{
   signed int a1;

   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_lbu (Mipc *mc, unsigned ins)
{
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_lh (Mipc *mc, unsigned ins)
{
   signed int a1;
                                                                                
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_lhu (Mipc *mc, unsigned ins)
{
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_lwl (Mipc *mc, unsigned ins)
{
   signed int a1;
   unsigned s1;
                                                                                
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_lw (Mipc *mc, unsigned ins)
{
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_lwr (Mipc *mc, unsigned ins)
{
   unsigned ar1, s1;
                                                                                
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_lwc1 (Mipc *mc, unsigned ins)
{
   mc->_num_load++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_swc1 (Mipc *mc, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_sb (Mipc *mc, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_sh (Mipc *mc, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_swl (Mipc *mc, unsigned ins)
{
   unsigned ar1, s1;
                                                                                
   mc->_num_store++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_sw (Mipc *mc, unsigned ins)
{
   mc->_num_store++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_swr (Mipc *mc, unsigned ins)
{
   unsigned ar1, s1;
                                                                                
   mc->_num_store++;
   SIGN_EXTEND_IMM(mc->ID_EX._decodedSRC2);
   mc->mar = (unsigned)(mc->ID_EX._decodedSRC1+mc->ID_EX._decodedSRC2);
}

void
Mipc::func_mtc1 (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC1;
}

void
Mipc::func_mfc1 (Mipc *mc, unsigned ins)
{
   mc->opRLo = mc->ID_EX._decodedSRC1;
}



void
Mipc::mem_lb (Mipc *mc)
{
   signed int a1;

   a1 = mc->_mem->BEGetByte(mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7));
   SIGN_EXTEND_BYTE(a1);
   mc->MEM_WB._opResultLo = a1;
   mc->flag_toChangeOpresult = TRUE;
}

void
Mipc::mem_lbu (Mipc *mc)
{
   mc->MEM_WB._opResultLo = mc->_mem->BEGetByte(mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7));
      mc->flag_toChangeOpresult = TRUE;

}

void
Mipc::mem_lh (Mipc *mc)
{
   signed int a1;

   a1 = mc->_mem->BEGetHalfWord(mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7));
   SIGN_EXTEND_IMM(a1);
   mc->MEM_WB._opResultLo = a1;
      mc->flag_toChangeOpresult = TRUE;

}

void
Mipc::mem_lhu (Mipc *mc)
{
   mc->MEM_WB._opResultLo = mc->_mem->BEGetHalfWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7));
      mc->flag_toChangeOpresult = TRUE;

}

void
Mipc::mem_lwl (Mipc *mc)
{
   signed int a1;
   unsigned s1;

   a1 = mc->_mem->BEGetWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7));
   s1 = (mc->EX_MEM._MAR & 3) << 3;
   mc->MEM_WB._opResultLo = (a1 << s1) | (mc->EX_MEM._subregOperand & ~(~0UL << s1));
      mc->flag_toChangeOpresult = TRUE;

}

void
Mipc::mem_lw (Mipc *mc)
{
   mc->MEM_WB._opResultLo = mc->_mem->BEGetWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7));
      mc->flag_toChangeOpresult = TRUE;

}

void
Mipc::mem_lwr (Mipc *mc)
{
   unsigned ar1, s1;

   ar1 = mc->_mem->BEGetWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7));
   s1 = (~mc->EX_MEM._MAR & 3) << 3;
   mc->MEM_WB._opResultLo = (ar1 >> s1) | (mc->EX_MEM._subregOperand & ~(~(unsigned)0 >> s1));
      mc->flag_toChangeOpresult = TRUE;

}

void
Mipc::mem_lwc1 (Mipc *mc)
{
   mc->MEM_WB._opResultLo = mc->_mem->BEGetWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7));
      mc->flag_toChangeOpresult = TRUE;

}

void
Mipc::mem_swc1 (Mipc *mc)
{
   mc->_mem->Write(mc->EX_MEM._MAR & ~(LL)0x7, mc->_mem->BESetWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7), mc->_fpr[mc->EX_MEM._decodedDST>>1].l[FP_TWIDDLE^(mc->EX_MEM._decodedDST&1)]));
}

void
Mipc::mem_sb (Mipc *mc)
{
   mc->_mem->Write(mc->EX_MEM._MAR & ~(LL)0x7, mc->_mem->BESetByte (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7), mc->_gpr[mc->EX_MEM._decodedDST] & 0xff));
}

void
Mipc::mem_sh (Mipc *mc)
{
   mc->_mem->Write(mc->EX_MEM._MAR & ~(LL)0x7, mc->_mem->BESetHalfWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7), mc->_gpr[mc->EX_MEM._decodedDST] & 0xffff));
}

void
Mipc::mem_swl (Mipc *mc)
{
   unsigned ar1, s1;

   ar1 = mc->_mem->BEGetWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7));
   s1 = (mc->EX_MEM._MAR & 3) << 3;
   ar1 = (mc->_gpr[mc->EX_MEM._decodedDST] >> s1) | (ar1 & ~(~(unsigned)0 >> s1));
   mc->_mem->Write(mc->EX_MEM._MAR & ~(LL)0x7, mc->_mem->BESetWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7), ar1));
}

void
Mipc::mem_sw (Mipc *mc)
{
   mc->_mem->Write(mc->EX_MEM._MAR & ~(LL)0x7, mc->_mem->BESetWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7), mc->_gpr[mc->EX_MEM._decodedDST]));
}

void
Mipc::mem_swr (Mipc *mc)
{
   unsigned ar1, s1;

   ar1 = mc->_mem->BEGetWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7));
   s1 = (~mc->EX_MEM._MAR & 3) << 3;
   ar1 = (mc->_gpr[mc->EX_MEM._decodedDST] << s1) | (ar1 & ~(~0UL << s1));
   mc->_mem->Write(mc->EX_MEM._MAR & ~(LL)0x7, mc->_mem->BESetWord (mc->EX_MEM._MAR, mc->_mem->Read(mc->EX_MEM._MAR & ~(LL)0x7), ar1));
}
