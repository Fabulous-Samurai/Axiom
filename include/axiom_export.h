#ifndef AXIOM_EXPORT_H
#define AXIOM_EXPORT_H

#if defined(AXIOM_STATIC)
  #define AXIOM_EXPORT
#elif defined(_WIN32)
  #if defined(AXIOM_ENGINE_EXPORTS)
    #define AXIOM_EXPORT __declspec(dllexport)
  #else
    #define AXIOM_EXPORT __declspec(dllimport)
  #endif
#else
  #define AXIOM_EXPORT __attribute__((visibility("default")))
#endif

#endif // AXIOM_EXPORT_H

