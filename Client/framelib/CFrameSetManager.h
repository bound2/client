//----------------------------------------------------------------------
// CFrameSetManager.h
//----------------------------------------------------------------------
//
// FramePack���� Ư�� Frame�� Load�� �� �ֵ��� ������ �����.
//
// <FrameID�� ������ List���>
//
// FramePack File�� �Ϻ� Frame ID�� �����Ѵ�. 
// 
// * FrameSet���� load�Ҷ� ����� index file�� ������ �ϴµ�,
//   ����� Frame ID�� �ش��ϴ� Frame�� File Position�� 
//   FramePack IndexFile���� ã�Ƽ� 
//   FrameSet index file�� �����ؾ� �Ѵ�.
//
// * ������ Frame ID�� ������ �ִ� ��� Sprite ID�� �����Ѵ�.
//   �ߺ��Ǹ� �ȵǹǷ� list�� �߰��Ҷ� �ߺ��� �ȵǵ��� �Ѵ�.
//   SpriteSet IndexFile�� �����ؾ� �ϴµ�, ������ SpritePack Index��
//   �̿��ؼ� �����Ѵ�.
//
//
// (!) ThingFrameSetManager�� CreatureFramePackManager�� �ΰ�����
//     ��ӹ޾Ƽ� ����ؾ� �� ���̴�
//----------------------------------------------------------------------
//
// Frame ID�� set�ϰ�
// FramePack�� File Position�� �����ؾ� �Ѵ�.  --> FrameSet Index File
// SpritePack�� File Position�� �����ؾ� �Ѵ�. --> SpriteSet Index File
//
//----------------------------------------------------------------------

#ifndef	__CFRAMESETMANAGER_H__
#define	__CFRAMESETMANAGER_H__


#include <fstream>
#include "DrawTypeDef.h"
#include "CSetManager.h"


class CFrameSetManager : public CSetManager<TYPE_FRAMEID, TYPE_FRAMEID> {
	public :
		CFrameSetManager();
		virtual ~CFrameSetManager();

		//--------------------------------------------------------
		// FramePack IndexFile�κ��� FrameSet IndexFile�� �����Ѵ�.
		//--------------------------------------------------------
		bool SaveFrameSetIndex(std::ofstream& setIndex, std::ifstream& packIndex);

		//--------------------------------------------------------
		// FrameSet�� ���õ� SpriteID�� �����ؼ�
		// SpritePack IndexFile�κ��� SpriteSet IndexFile�� �����Ѵ�.
		//--------------------------------------------------------
		//virtual bool		SaveSpriteSetIndex(class ofstream& setIndex, class ifstream& packIndex) = 0;
		

	protected :
		
};


#endif



