#ifndef CxAODTools_ReturnCheck_H
#define CxAODTools_ReturnCheck_H

#define TOOL_CHECK( CONTEXT, EXP )                            \
  if (!EXP.isSuccess()) {                                     \
    Error( CONTEXT, "Failed to execute: %s. Exiting.", #EXP); \
    return EL::StatusCode::FAILURE;                           \
  }

#define EL_CHECK( CONTEXT, EXP )                              \
  if (EXP !=  EL::StatusCode::SUCCESS) {                      \
    Error( CONTEXT, "Failed to execute: %s. Exiting.", #EXP); \
    return EL::StatusCode::FAILURE;                           \
  }

#define CP_CHECK( CONTEXT, EXP, DEBUG )                                               \
  if (DEBUG) {                                                                        \
    Info(CONTEXT, "Calling: %s", #EXP);                                               \
  }                                                                                   \
  if (EXP ==  CP::CorrectionCode::Error) {                                            \
    Error( CONTEXT, "Failed to execute: %s. Exiting.", #EXP);                         \
    return EL::StatusCode::FAILURE;                                                   \
  }

#endif
