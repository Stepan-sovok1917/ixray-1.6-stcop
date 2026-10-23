#pragma once

class ESceneDummyTool: public ESceneCustomOTool
{
	typedef ESceneCustomOTool inherited;
protected:
    // controls
    virtual void 		CreateControls			();
	virtual void 		RemoveControls			();
public:
						ESceneDummyTool			():ESceneCustomOTool(OBJCLASS_DUMMY){;}
                        
    virtual void		Clear					(bool bSpecific=false){inherited::Clear(bSpecific);}
	// definition
    // The placeholder tool has no name of its own; the callers treat an empty name as
    // "no tool" instead of aborting the frame.
    IC const char*			ClassName				(){return "";}
    IC const char*			ClassDesc				(){return "";}
    IC int				RenderPriority			(){return -1; }

    virtual bool   		IsNeedSave				(){return false;}

    virtual CCustomObject* CreateObject			(LPVOID data, const char* name){return 0;};
};
