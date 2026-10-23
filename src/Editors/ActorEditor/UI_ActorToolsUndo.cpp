#include "stdafx.h"         
                                                                                
                                        
#include "UI_ActorTools.h"           
                                           
void CActorTools::UndoClear()
{                                             
	while( !m_RedoStack.empty() ){
		unlink( m_RedoStack.back().m_FileName );
		m_RedoStack.pop_back(); }
	while( !m_UndoStack.empty() ){
		unlink( m_UndoStack.back().m_FileName );
		m_UndoStack.pop_back(); }
}

void CActorTools::UndoSave()
{
	EContext.UI->RedrawScene();
    if (0==EPrefs->scene_undo_level) return;

	UndoItem item;
	item.m_FileName[0] = 0;
	// A unique name in the editor temp folder; GetTempFileName exists only on Windows.
	static u32 UndoSerial = 0;
	string_path TempFolder;
	FS.update_path(TempFolder, _temp_, "");
	xr_sprintf(item.m_FileName, "%sundo_%08u_%u.tmp", TempFolder, (u32)std::time(nullptr), ++UndoSerial);

	Save( item.m_FileName, true );
	m_UndoStack.push_back( item );

	while( !m_RedoStack.empty() ){
		unlink( m_RedoStack.back().m_FileName );
		m_RedoStack.pop_back(); }

	if( m_UndoStack.size() > EPrefs->scene_undo_level){
		unlink( m_UndoStack.front().m_FileName );
		m_UndoStack.pop_front(); }
}

bool CActorTools::Undo()
{
//	if( !m_UndoStack.empty() ){
	if( m_UndoStack.size()>1 ){
		m_RedoStack.push_back( m_UndoStack.back() );
		m_UndoStack.pop_back();

		if( m_RedoStack.size() > EPrefs->scene_undo_level){
			unlink( m_RedoStack.front().m_FileName );
			m_RedoStack.pop_front();
        }

		if( !m_UndoStack.empty() ){
			Clear();
         	Load( m_UndoStack.back().m_FileName );
        }

        Modified();
        
		return true;
	}
	return false;
}

bool CActorTools::Redo()
{
	if( !m_RedoStack.empty() ){
        Clear();
		Load( m_RedoStack.back().m_FileName );

		m_UndoStack.push_back( m_RedoStack.back() );
		m_RedoStack.pop_back();

		if( m_UndoStack.size() > EPrefs->scene_undo_level ){
			unlink( m_UndoStack.front().m_FileName );
			m_UndoStack.pop_front(); 
        }

        Modified();

		return true;
	}
	return false;
}

//----------------------------------------------------

 