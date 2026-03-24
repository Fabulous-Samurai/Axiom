// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#ifndef AXIOM_EXPORT_H
#define AXIOM_EXPORT_H

#ifdef _WIN32
  #ifdef AXIOM_ENGINE_EXPORTS
    #define AXIOM_EXPORT __declspec(dllexport)
  #else
    #define AXIOM_EXPORT __declspec(dllimport)
  #endif
#else
  #define AXIOM_EXPORT __attribute__((visibility("default")))
#endif

#endif // AXIOM_EXPORT_H

