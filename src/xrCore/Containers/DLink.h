/* Copyright (C) Tom Forsyth, 2001. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Tom Forsyth, 2001"
 */
// Some standard useful classes, templates, etc.
#pragma once

// Defines a link
#define DlinkDefine(classname,linkname)\
	classname *linkname##pNextItem;\
	classname *linkname##pPrevItem


// Defines the class methods for the link.
#define DlinkMethods(classname,linkname)																		\
	void linkname##Init ( void ) { linkname##pNextItem = nullptr; linkname##pPrevItem = nullptr; }					\
	\
	/* Find the next in the list. */																			\
	classname *linkname##Next ( void )																			\
	{																											\
	VERIFY ( ( linkname##pPrevItem == nullptr ) || ( linkname##pPrevItem->linkname##pNextItem == this ) );		\
	VERIFY ( ( linkname##pNextItem == nullptr ) || ( linkname##pNextItem->linkname##pPrevItem == this ) );		\
	return linkname##pNextItem;																				\
	}																											\
	\
	/* Find the previous in the list. */																		\
	classname *linkname##Prev ( void )																			\
	{																											\
	VERIFY ( ( linkname##pPrevItem == nullptr ) || ( linkname##pPrevItem->linkname##pNextItem == this ) );		\
	VERIFY ( ( linkname##pNextItem == nullptr ) || ( linkname##pNextItem->linkname##pPrevItem == this ) );		\
	return linkname##pPrevItem;																				\
	}																											\
	\
	/* Remove this item from the list. */																		\
	/* Returns the next item. */																				\
	classname *linkname##Del ( void )																			\
	{																											\
	classname *pNext = linkname##pNextItem;																	\
	classname *pPrev = linkname##pPrevItem;																	\
	VERIFY ( ( pPrev == nullptr ) || ( pPrev->linkname##pNextItem == this ) );									\
	VERIFY ( ( pNext == nullptr ) || ( pNext->linkname##pPrevItem == this ) );									\
	if ( pPrev != nullptr )																					\
		{																										\
		pPrev->linkname##pNextItem = pNext;																	\
		}																										\
		if ( pNext != nullptr )																					\
		{																										\
		pNext->linkname##pPrevItem = pPrev;																	\
		}																										\
		linkname##pNextItem = nullptr;																				\
		linkname##pPrevItem = nullptr;																				\
		return pNext;																							\
	}																											\
	\
	/* Remove this item from the list. */																		\
	/* Assumes that this is the first item in the list for speed (asserted). */									\
	/* Returns the next item. */																				\
	classname *linkname##DelScan ( void )																		\
	{																											\
	VERIFY ( linkname##pPrevItem == nullptr );																	\
	classname *pNext = linkname##pNextItem;																	\
	VERIFY ( ( pNext == nullptr ) || ( pNext->linkname##pPrevItem == this ) );									\
	if ( pNext != nullptr )																					\
		{																										\
		pNext->linkname##pPrevItem = nullptr;																	\
		}																										\
		linkname##pNextItem = nullptr;																				\
		linkname##pPrevItem = nullptr;																				\
		return pNext;																							\
	}																											\
	\
	/* Deletes the item, given a pointer to the first item in the list. */										\
	/* Modifies the point as necessary. */																		\
	/* Returns the next item. */																				\
	/* Use like   pThing->DelFromFirst ( &pFirst ); */															\
	classname *linkname##DelFromFirst ( classname **ppFirst )													\
	{																											\
	VERIFY ( ppFirst != nullptr );																				\
	VERIFY ( *ppFirst != nullptr );																			\
	if ( *ppFirst == this )																					\
		{																										\
		VERIFY ( linkname##pPrevItem == nullptr );																\
		/* First in the list. */																			\
		*ppFirst = linkname##Del();																			\
		return *ppFirst;																					\
		}																										\
		else																									\
		{																										\
		/* DLink lists already know their previous item. */													\
		return ( linkname##Del() );																			\
		}																										\
	}																											\
	\
	/* Add the item after the given item. */																	\
	/* Return the new next item. */																				\
	classname *linkname##AddAfter ( classname *pPrev )															\
	{																											\
	VERIFY ( linkname##pNextItem == nullptr );																	\
	VERIFY ( linkname##pPrevItem == nullptr );																	\
	VERIFY ( pPrev != nullptr );																				\
	linkname##pNextItem = pPrev->linkname##pNextItem;														\
	linkname##pPrevItem = pPrev;																			\
	if ( linkname##pNextItem != nullptr )																		\
		{																										\
		linkname##pNextItem->linkname##pPrevItem = this;													\
		}																										\
		pPrev->linkname##pNextItem = this;																		\
		return ( linkname##pNextItem );																			\
	}																											\
	\
	/* Add the item after the given item. */																	\
	/* Assumes this is the last item in the list for speed (asserted) */										\
	void linkname##AddAfterEnd ( classname *pPrev )																\
	{																											\
	VERIFY ( linkname##pNextItem == nullptr );																	\
	VERIFY ( linkname##pPrevItem == nullptr );																	\
	VERIFY ( pPrev != nullptr );																				\
	VERIFY ( pPrev->linkname##pNextItem == nullptr );															\
	linkname##pNextItem = nullptr;																				\
	linkname##pPrevItem = pPrev;																			\
	pPrev->linkname##pNextItem = this;																		\
	}																											\
	\
	/* Add before given item. */																				\
	/* Returns the new previous item. */																		\
	classname *linkname##AddBefore ( classname *pNext )															\
	{																											\
	VERIFY ( linkname##pNextItem == nullptr );																	\
	VERIFY ( linkname##pPrevItem == nullptr );																	\
	VERIFY ( pNext != nullptr );																				\
	linkname##pNextItem = pNext;																			\
	linkname##pPrevItem = pNext->linkname##pPrevItem;														\
	if ( linkname##pPrevItem != nullptr )																		\
		{																										\
		linkname##pPrevItem->linkname##pNextItem = this;													\
		}																										\
		pNext->linkname##pPrevItem = this;																		\
		return ( linkname##pPrevItem );																			\
	}																											\
	\
	/* Add the item before the given item. */																	\
	/* Assumes this is the first item in the list for speed (asserted) */										\
	void linkname##AddBeforeStart ( classname *pNext )															\
	{																											\
	VERIFY ( linkname##pNextItem == nullptr );																	\
	VERIFY ( linkname##pPrevItem == nullptr );																	\
	VERIFY ( pNext != nullptr );																				\
	VERIFY ( pNext->linkname##pPrevItem == nullptr );															\
	linkname##pPrevItem = nullptr;																				\
	linkname##pNextItem = pNext;																			\
	pNext->linkname##pPrevItem = this;																		\
	}																											\
	\
	/* Find the last item in the list. */																		\
	classname *linkname##FindLast ( void )																		\
	{																											\
	classname *pCurr = this;																				\
	VERIFY ( pCurr != nullptr );																				\
	while ( pCurr->linkname##pNextItem != nullptr )															\
		{																										\
		VERIFY ( ( pCurr->linkname##pPrevItem == nullptr ) || ( pCurr->linkname##pPrevItem->linkname##pNextItem == pCurr ) );	\
		VERIFY ( ( pCurr->linkname##pNextItem == nullptr ) || ( pCurr->linkname##pNextItem->linkname##pPrevItem == pCurr ) );	\
		pCurr = pCurr->linkname##pNextItem;																	\
		}																										\
		return pCurr;																							\
	}																											\
	\
	/* Find the first item in the list. */																		\
	classname *linkname##FindFirst ( void )																		\
	{																											\
	classname *pCurr = this;																				\
	VERIFY ( pCurr != nullptr );																				\
	while ( pCurr->linkname##pPrevItem != nullptr )															\
		{																										\
		VERIFY ( ( pCurr->linkname##pPrevItem == nullptr ) || ( pCurr->linkname##pPrevItem->linkname##pNextItem == pCurr ) );	\
		VERIFY ( ( pCurr->linkname##pNextItem == nullptr ) || ( pCurr->linkname##pNextItem->linkname##pPrevItem == pCurr ) );	\
		pCurr = pCurr->linkname##pPrevItem;																	\
		}																										\
		return pCurr;																							\
	}																											\
	\
	/* Consistency check - checks the current list is sound. */													\
	void linkname##ConsistencyCheck ( void )																	\
	{																											\
	classname *pCurr = this;																						\
	/* Scan backwards. */																					\
	VERIFY ( pCurr != nullptr );																				\
	while ( pCurr->linkname##pPrevItem != nullptr )															\
		{																										\
		VERIFY ( ( pCurr->linkname##pPrevItem == nullptr ) || ( pCurr->linkname##pPrevItem->linkname##pNextItem == pCurr ) );				\
		VERIFY ( ( pCurr->linkname##pNextItem == nullptr ) || ( pCurr->linkname##pNextItem->linkname##pPrevItem == pCurr ) );				\
		pCurr = pCurr->linkname##pPrevItem;																	\
		}																										\
		VERIFY ( ( pCurr->linkname##pPrevItem == nullptr ) || ( pCurr->linkname##pPrevItem->linkname##pNextItem == pCurr ) );					\
		VERIFY ( ( pCurr->linkname##pNextItem == nullptr ) || ( pCurr->linkname##pNextItem->linkname##pPrevItem == pCurr ) );					\
		\
		/* Scan forwards. */																					\
		pCurr = this;																							\
		VERIFY ( pCurr != nullptr );																				\
		while ( pCurr->linkname##pNextItem != nullptr )															\
		{																										\
		VERIFY ( ( pCurr->linkname##pPrevItem == nullptr ) || ( pCurr->linkname##pPrevItem->linkname##pNextItem == pCurr ) );				\
		VERIFY ( ( pCurr->linkname##pNextItem == nullptr ) || ( pCurr->linkname##pNextItem->linkname##pPrevItem == pCurr ) );				\
		pCurr = pCurr->linkname##pNextItem;																	\
		}																										\
		VERIFY ( ( pCurr->linkname##pPrevItem == nullptr ) || ( pCurr->linkname##pPrevItem->linkname##pNextItem == pCurr ) );					\
		VERIFY ( ( pCurr->linkname##pNextItem == nullptr ) || ( pCurr->linkname##pNextItem->linkname##pPrevItem == pCurr ) );					\
	}																											\
	\
	/* Adds the item to the end of this list */																	\
	void linkname##AddToEnd ( classname *pItem )																\
	{																											\
	VERIFY ( pItem != nullptr );																				\
	linkname##AddAfterEnd ( pItem->linkname##FindLast() );													\
	}																											\
	\
	/* Adds the item the the start of this list */																\
	void linkname##AddToStart ( classname *pItem )																\
	{																											\
	VERIFY ( pItem != nullptr );																				\
	linkname##AddBeforeStart ( pItem->linkname##FindFirst() );												\
	}																											\
	\
	/* Check that all the pointer are nullptr, ready to be deleted */												\
	bool linkname##CheckNull ( void )																			\
	{																											\
	return ( ( linkname##pNextItem == nullptr ) && ( linkname##pPrevItem == nullptr ) );							\
	}																											\
	\
	/* Calls delete() on all the objects in this list and deletes it */											\
	void linkname##DelWholeList ( void )																		\
	{																											\
	classname *pCur = linkname##FindFirst();																\
	while ( pCur != nullptr )																					\
		{																										\
		classname *pNext = pCur->linkname##DelScan();														\
		delete ( pCur );																					\
		pCur = pNext;																						\
		}																										\
	}																											\
