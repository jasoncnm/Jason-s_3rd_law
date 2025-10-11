#if !defined(ENGINE_TEXT_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

template <typename ...Args>
const char * EngineTextFormat(const char *text, Args... args);                                        // Text formatting with variables (sprintf() style)

#define ENGINE_TEXT_H
#endif
