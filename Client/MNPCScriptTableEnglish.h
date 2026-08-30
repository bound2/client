//----------------------------------------------------------------------
// MNPCScriptTableEnglish.h
//----------------------------------------------------------------------
// The English text for Data/Info/NPCScript.inf, which ships in Korean.
// The table itself is generated - see MNPCScriptTableEnglish.cpp.
//----------------------------------------------------------------------

#ifndef __MNPCSCRIPTTABLEENGLISH_H__
#define	__MNPCSCRIPTTABLEENGLISH_H__

//----------------------------------------------------------------------
// Replace the loaded NPC script text with the English translation.
// Call after MNPCScriptTable::LoadFromFile(), and only when the client is
// running in English - see UseEnglishText().
//----------------------------------------------------------------------
extern void				ApplyEnglishNPCScriptTable();

#endif
