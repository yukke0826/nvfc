//==-- NvfcISelLowering.h - Nvfc DAG Lowering Interface ------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Nvfc uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_NVFUC_ISELLOWERING_H
#define LLVM_TARGET_NVFUC_ISELLOWERING_H

#include "Nvfc.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Target/TargetLowering.h"

namespace llvm {
  namespace NvfcISD {
    enum {
      FIRST_NUMBER = ISD::BUILTIN_OP_END,

      /// Return with a flag operand. Operand 0 is the chain operand.
      RET_FLAG,

      /// Same as RET_FLAG, but used for returning from ISRs.
      RETI_FLAG,

      /// Y = R{R,L}A X, rotate right (left) arithmetically
      RRA, RLA,

      /// Y = RRC X, rotate right via carry
      RRC,

      /// CALL - These operations represent an abstract call
      /// instruction, which includes a bunch of information.
      CALL,

      /// Wrapper - A wrapper node for TargetConstantPool, TargetExternalSymbol,
      /// and TargetGlobalAddress.
      Wrapper,

      /// CMP - Compare instruction.
      CMP,

      /// SetCC - Operand 0 is condition code, and operand 1 is the flag
      /// operand produced by a CMP instruction.
      SETCC,

      /// Nvfc conditional branches. Operand 0 is the chain operand, operand 1
      /// is the block to branch if condition is true, operand 2 is the
      /// condition code, and operand 3 is the flag operand produced by a CMP
      /// instruction.
      BR_CC,

      /// SELECT_CC - Operand 0 and operand 1 are selection variable, operand 3
      /// is condition code and operand 4 is flag operand.
      SELECT_CC,

      /// XBIT_FLAG - Extract a single bit of flag register.
      XBIT_FLAG,

      /// XBIT - Extract a single bit of a specified register.
      XBIT
    };
  }

  class NvfcSubtarget;
  class NvfcTargetMachine;

  class NvfcTargetLowering : public TargetLowering {
  public:
    explicit NvfcTargetLowering(NvfcTargetMachine &TM);

    /// LowerOperation - Provide custom lowering hooks for some operations.
    virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const;

    /// getTargetNodeName - This method returns the name of a target specific
    /// DAG node.
    virtual const char *getTargetNodeName(unsigned Opcode) const;

    /// getFunctionAlignment - Return the Log2 alignment of this function.
    virtual unsigned getFunctionAlignment(const Function *F) const;

    SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerExternalSymbol(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSETCC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSIGN_EXTEND(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerRETURNADDR(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const;
    SDValue getReturnAddressFrameIndex(SelectionDAG &DAG) const;

    TargetLowering::ConstraintType
    getConstraintType(const std::string &Constraint) const;
    std::pair<unsigned, const TargetRegisterClass*>
    getRegForInlineAsmConstraint(const std::string &Constraint, EVT VT) const;

    /// isTruncateFree - Return true if it's free to truncate a value of type
    /// Ty1 to type Ty2. e.g. On nvfc it's free to truncate a i32 value in
    /// register R15W to i16 by referencing its sub-register R15B.
    virtual bool isTruncateFree(const Type *Ty1, const Type *Ty2) const;
    virtual bool isTruncateFree(EVT VT1, EVT VT2) const;

    /// isZExtFree - Return true if any actual instruction that defines a value
    /// of type Ty1 implicit zero-extends the value to Ty2 in the result
    /// register. This does not necessarily include registers defined in unknown
    /// ways, such as incoming arguments, or copies from unknown virtual
    /// registers. Also, if isTruncateFree(Ty2, Ty1) is true, this does not
    /// necessarily apply to truncate instructions. e.g. on nvfc, all
    /// instructions that define 8-bit values implicit zero-extend the result
    /// out to 16 bits.
    virtual bool isZExtFree(const Type *Ty1, const Type *Ty2) const;
    virtual bool isZExtFree(EVT VT1, EVT VT2) const;

    MachineBasicBlock* EmitInstrWithCustomInserter(MachineInstr *MI,
                                                   MachineBasicBlock *BB) const;

  private:
    SDValue LowerCCCCallTo(SDValue Chain, SDValue Callee,
                           CallingConv::ID CallConv, bool isVarArg,
                           bool isTailCall,
                           const SmallVectorImpl<ISD::OutputArg> &Outs,
                           const SmallVectorImpl<SDValue> &OutVals,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           DebugLoc dl, SelectionDAG &DAG,
                           SmallVectorImpl<SDValue> &InVals) const;

    SDValue LowerCCCArguments(SDValue Chain,
                              CallingConv::ID CallConv,
                              bool isVarArg,
                              const SmallVectorImpl<ISD::InputArg> &Ins,
                              DebugLoc dl,
                              SelectionDAG &DAG,
                              SmallVectorImpl<SDValue> &InVals) const;

    SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                            CallingConv::ID CallConv, bool isVarArg,
                            const SmallVectorImpl<ISD::InputArg> &Ins,
                            DebugLoc dl, SelectionDAG &DAG,
                            SmallVectorImpl<SDValue> &InVals) const;

    virtual SDValue
      LowerFormalArguments(SDValue Chain,
                           CallingConv::ID CallConv, bool isVarArg,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           DebugLoc dl, SelectionDAG &DAG,
                           SmallVectorImpl<SDValue> &InVals) const;
    virtual SDValue
      LowerCall(SDValue Chain, SDValue Callee,
                CallingConv::ID CallConv, bool isVarArg, bool &isTailCall,
                const SmallVectorImpl<ISD::OutputArg> &Outs,
                const SmallVectorImpl<SDValue> &OutVals,
                const SmallVectorImpl<ISD::InputArg> &Ins,
                DebugLoc dl, SelectionDAG &DAG,
                SmallVectorImpl<SDValue> &InVals) const;

    virtual SDValue
      LowerReturn(SDValue Chain,
                  CallingConv::ID CallConv, bool isVarArg,
                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                  const SmallVectorImpl<SDValue> &OutVals,
                  DebugLoc dl, SelectionDAG &DAG) const;

    virtual bool getPostIndexedAddressParts(SDNode *N, SDNode *Op,
                                            SDValue &Base,
                                            SDValue &Offset,
                                            ISD::MemIndexedMode &AM,
                                            SelectionDAG &DAG) const;

    const NvfcSubtarget &Subtarget;
    const NvfcTargetMachine &TM;
    const TargetData *TD;
  };
} // namespace llvm

#endif // LLVM_TARGET_NVFUC_ISELLOWERING_H
