// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#ifndef AXIOM_EXPORT_H
#define AXIOM_EXPORT_H

#if defined(_WIN32) && defined(AXIOM_BUILD_DLL)
  #ifdef AXIOM_ENGINE_EXPORTS
    #define AXIOM_EXPORT __declspec(dllexport)
  #else
    #define AXIOM_EXPORT __declspec(dllimport)
  #endif
#else
  #if defined(__GNUC__)
    #define AXIOM_EXPORT __attribute__((visibility("default")))
  #else
    #define AXIOM_EXPORT
  #endif
#endif

#endif // AXIOM_EXPORT_H

