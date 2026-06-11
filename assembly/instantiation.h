#ifndef INSTANTIATION_H
#define INSTANTIATION_H
#endif

#undef INSTANTIATION_H

// ============================================================
// INSTANTIATION macro — two modes depending on context:
//
//   Mode 1 (normal include, INSTANTIATION_DEFINE not set):
//     INSTANTIATION(Type, name, ...)
//     expands to:
//       extern Type name;
//     → tells other translation units "this object exists somewhere"
//
//   Mode 2 (define mode, INSTANTIATION_DEFINE is set):
//     INSTANTIATION(Type, name, arg1, arg2, ...)
//     expands to:
//       Type name(arg1, arg2, ...);
//     → actually constructs the object (calls the constructor)
// ============================================================

#if defined(INSTANTIATION_DEFINE)
#define INSTANTIATION(Type, name, ...) Type name(__VA_ARGS__);
#else
#define INSTANTIATION(Type, name, ...) extern Type name;
#endif
