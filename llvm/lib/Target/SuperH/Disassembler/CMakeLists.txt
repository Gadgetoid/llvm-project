add_llvm_component_library(LLVMSuperHDisassembler
  SuperHDisassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  MC
  SuperHDesc
  SuperHInfo
  Support

  ADD_TO_COMPONENT
  SuperH
  )
