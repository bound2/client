// VS_UI_Descriptor.cpp

#include "Client_PCH.h"
#include <assert.h>
#include "VS_UI_descriptor.h"

DescriptorManager	g_descriptor_manager;

//----------------------------------------------------------------------------
// Operations
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// DescriptorManager::DescriptorManager
//
// 
//-----------------------------------------------------------------------------
DescriptorManager::DescriptorManager()
{
	m_fp_show = NULL;
	m_fp_show_param.rect.Set(0, 0, 0, 0);
	m_fp_show_param.void_ptr = NULL;
	m_fp_show_param.left = 0;
	m_fp_show_param.right = 0;
	m_p_void_ptr2 = NULL;
}

//-----------------------------------------------------------------------------
// DescriptorManager::~DescriptorManager
//
// 
//-----------------------------------------------------------------------------
DescriptorManager::~DescriptorManager()
{
	DESCRIBED_UNIT * data;
	for (int i=0; i < Size(); i++)
		if (Data(i, data))
			delete data;
}

//-----------------------------------------------------------------------------
// DescriptorManager::Set
//
// void_ptr2 carries a pointer that will not survive left/right, which are long
// and therefore 32 bits on 64-bit Windows.
//
// The early return below is load-bearing for it: while a descriptor is showing,
// every Set() is dropped, so m_p_void_ptr2 cannot change under the descriptor
// that Show() is drawing. Anyone relaxing that guard - letting a new hover
// replace the current descriptor is the obvious future change - has to move the
// secondary pointer into FP_SHOW_PARAM at the same time, or a descriptor will be
// drawn with the previous one's payload.
//-----------------------------------------------------------------------------
void	DescriptorManager::Set(id_t id, int x, int y, void * void_ptr, long left, long right, void * void_ptr2)
{
	if (m_fp_show) // doing?
		return;

	// Nothing is showing, so no descriptor owns the old value any more. Clearing
	// it here rather than only on the matched-id path below means a Set() for an
	// id that has no unit registered cannot leave a stale pointer behind either.
	m_p_void_ptr2 = NULL;

	DESCRIBED_UNIT * data;
	for (int i=0; i < Size(); i++)
		if (Data(i, data))
			if (data->id == id)
			{
				assert(!gpC_base->m_p_DDSurface_back->IsLock());
				// Set before the calculator runs: the calculator reads it back
				// through GetSecondaryPtr() to size the descriptor.
				m_p_void_ptr2 = void_ptr2;
				data->fp_rect_calculator(data->fp_show, x, y, void_ptr, left, right);
				return;
			}
}

//-----------------------------------------------------------------------------
// DescriptorManager::Unset
//
// 
//-----------------------------------------------------------------------------
//void	DescriptorManager::Unset()
//{
//	m_fp_show = NULL;
//}

//-----------------------------------------------------------------------------
// DescriptorManager::Unset 
//
// by sigi
// 

//-----------------------------------------------------------------------------
void	DescriptorManager::Unset(void* pPtr)
{
	if (pPtr==NULL)
	{
		m_fp_show = NULL;
		m_p_void_ptr2 = NULL;
		return;
	}

	if (m_fp_show)
	{

		// Both payloads have to be tested. This is the item destruction hook -
		// UI_RemoveDescriptor() calls it with the item about to be deleted - and
		// the two payloads are different objects: the gear tooltip shows a
		// socketed core zap from one slot alongside the item in another, so
		// destroying either one has to take the descriptor down. Matching only
		// the primary would leave the secondary dangling and dereferenced by the
		// next Show().
		if (pPtr==m_fp_show_param.void_ptr || pPtr==m_p_void_ptr2)
		{
			m_fp_show = NULL;
			m_p_void_ptr2 = NULL;
		}
	}
}


//-----------------------------------------------------------------------------
// DescriptorManager::Show
//
// 
//-----------------------------------------------------------------------------
void	DescriptorManager::Show()
{
	assert(!gpC_base->m_p_DDSurface_back->IsLock());
	if (m_fp_show)
		m_fp_show(m_fp_show_param.rect, m_fp_show_param.void_ptr, m_fp_show_param.left, m_fp_show_param.right);
}

//-----------------------------------------------------------------------------
// DescriptorManager::GetSecondaryPtr
//
// The pointer handed to the last dispatched Set(), for the calculator and show
// functions that need a payload wider than long. NULL unless that Set() passed
// one. The caller owns the lifetime and must NULL-check.
//-----------------------------------------------------------------------------
void *	DescriptorManager::GetSecondaryPtr() const
{
	return m_p_void_ptr2;
}

//-----------------------------------------------------------------------------
// DescriptorManager::AddDescribedUnit
//
// 
//-----------------------------------------------------------------------------
void	DescriptorManager::AddDescribedUnit(id_t id, void (*fp_rect_calculator)(void (*fp_show)(Rect, void *, long, long), int, int, void *, long, long), void (*fp_show)(Rect, void *, long, long), bool bl_immediate)
{
	// same id?
	DESCRIBED_UNIT * data;
	for (int i=0; i < Size(); i++)
		if (Data(i, data))
			if (data->id == id)
				return;

	DESCRIBED_UNIT * p_unit = new DESCRIBED_UNIT;

	p_unit->bl_immediate = bl_immediate;
	p_unit->fp_rect_calculator = fp_rect_calculator;
	p_unit->fp_show = fp_show;
	p_unit->id = id;

	Add(p_unit);
}

//-----------------------------------------------------------------------------
// DescriptorManager::RectCalculationFinished
//
// 
//-----------------------------------------------------------------------------
void	DescriptorManager::RectCalculationFinished(void (*fp_show)(Rect, void *, long, long), Rect rect, void * void_ptr, long left, long right)
{
	m_fp_show = fp_show;
	m_fp_show_param.rect = rect;
	m_fp_show_param.void_ptr = void_ptr;
	m_fp_show_param.left = left;
	m_fp_show_param.right = right;
}