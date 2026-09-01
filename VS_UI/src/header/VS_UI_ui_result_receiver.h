#ifndef __VS_UI_UI_RESULT_RECEIVER_H__
#define __VS_UI_UI_RESULT_RECEIVER_H__

#include "typedef.h"
#include "SimpleDataList.h"
#include "VS_UI_UIMessage.h"


//
// left and right are intptr_t, not int, because several senders put a pointer
// in them - an MItem* or a c_str() - and int is 32 bits on 64-bit Windows, so
// the pointer arrived at the handler with its top half gone. The dispatch runs
// through a function pointer table, so every handler signature has to match
// this width or the assignment is a compile error, which is what keeps the two
// ends honest.
//
struct MESSAGE
{
	DWORD				message;
	intptr_t			left;
	intptr_t			right;
	void *			void_ptr;
};

//-----------------------------------------------------------------------------
// C_MESSAGE_QUEUE 
//
// 
//-----------------------------------------------------------------------------
class C_MESSAGE_QUEUE : public SimpleDataList<MESSAGE *>
{
public:
	C_MESSAGE_QUEUE();
	~C_MESSAGE_QUEUE();
};

/*-----------------------------------------------------------------------------
  Class VS UI - UI Result Receiver

  `  message   .   process 
    message     process kill   .
	 message message queue .
-----------------------------------------------------------------------------*/
class C_VS_UI_UI_RESULT_RECEIVER
{
private:
	C_MESSAGE_QUEUE			m_message_queue;

	void (*m_fp_result_receiver)(DWORD, intptr_t, intptr_t, void *);

public:
	C_VS_UI_UI_RESULT_RECEIVER();
	~C_VS_UI_UI_RESULT_RECEIVER();

	void _SendMessage(DWORD message, intptr_t left = 0, intptr_t right = 0, void *void_ptr = NULL);
	void	_DispatchMessage();

/*-----------------------------------------------------------------------------
  Set.
-----------------------------------------------------------------------------*/
	void SetResultReceiver(void (*fp)(DWORD, intptr_t, intptr_t, void *));

#ifndef _LIB
	int	GetMessageSize() const { return m_message_queue.Size(); }
#endif
};

#endif