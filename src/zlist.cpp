#include "def.h"

#include "zlist.h"

/*
	Function: ZList::~ZList
	Description:
		Destructor for ZList class.  The function iterates over all
		the linked list nodes and deletes them.  If the data
		corresponding to each node needs to be deleted, ZList
		should be inherited and the interited destructor should
		delete the data before ZList::~ZList is called.
*/
ZList::~ZList()
{
	// Delete all the nodes in the list.
	ListNode * plnNext = m_lnListHead.m_plnNext;
	while (plnNext != NULL) {
		ListNode * plnCurrent = plnNext;
		plnNext = plnNext->m_plnNext;
		delete plnCurrent;
	};
}

/*
	Function: ZList::Add
	Description:
		Adds a node to the end of the list.
*/
void ZList::Add(void * pvThing)
{
	ASSERT(pvThing != NULL);
	// Create a new node.
	ListNode * plnNew = new ListNode(NULL, pvThing);

	// Add new nodes to the end of the list.
	ListNode *plnTail = &m_lnListHead;
	while (plnTail->m_plnNext != NULL) {
		plnTail = plnTail->m_plnNext;
	}
	plnTail->m_plnNext = plnNew;
}

/*
	Function: ZList::Insert
	Description:
		Insert *pvThing into the list.  Uses the function Compare
		to decide where in the list to put the item.  By default,
		the list is ordered from "smaller" to "larger," with
		duplicate items being inserted ahead of previously inserted
		duplicates.  To change the behaviour, alter the return value
		of the Compare function to get what you want.
*/
void ZList::Insert(void * pvThing)
{
	ASSERT(pvThing != NULL);

	// Create a new node.
	ListNode * plnNew = new ListNode(NULL, pvThing);

	// Insert the new node.
	ListNode *plnBehind = &m_lnListHead;
	ListNode *plnAhead = m_lnListHead.m_plnNext;
	while (plnAhead != NULL &&
		(Compare(pvThing, plnAhead->m_pvThing) == 1)) {
		plnBehind = plnAhead;
		plnAhead = plnAhead->m_plnNext;
	}
	plnBehind->m_plnNext = plnNew;
	plnNew->m_plnNext = plnAhead;

}

/*
	Function: ZList::Delete
	Description:
		Deletes a node from the list.
*/
void ZList::Delete(void * pvThing)
{
	ASSERT(pvThing != NULL);

	ListNode *plnBehind = &m_lnListHead;
	ListNode *plnCurrent = m_lnListHead.m_plnNext;
	while (plnCurrent != NULL && plnCurrent->m_pvThing != pvThing) {
		plnBehind = plnCurrent;
		plnCurrent = plnCurrent->m_plnNext;
	}
	if (plnCurrent != NULL) {
		DeleteData(plnCurrent->m_pvThing);
		plnBehind->m_plnNext = plnCurrent->m_plnNext;
        delete plnCurrent;
	}
}

/*
	Function: ZList::First
	Description:
		Returns a pointer to the data in the first node of the
		list.  If there are no nodes in the list then it returns
		NULL.  ZList::First should be called before ZList::Next
		to iterate over the list.
*/
void * ZList::First(void)
{
	if (m_lnListHead.m_plnNext == NULL) {
		m_plnListNext = NULL;
		return NULL;
	}
	else {
		m_plnListNext = m_lnListHead.m_plnNext->m_plnNext;
		return m_lnListHead.m_plnNext->m_pvThing;
	}
}

/*
	Function: ZList::Next
	Description:
		Returns a pointer to the next data object in the list following
		a call to the object returned by First or a previous call to
		Next.  Returns NULL if there are no more items in the list.
		ZList::First and ZList::Next can be used to iterate
		over the list.  Note that iteration order is undefined if
		list items are added while an iteration is progressing.  In
		particular, if ZList::Next has just returned NULL, calling
		it again after adding an element will not return that element.
*/
void * ZList::Next(void)
{
	if (m_plnListNext == NULL) {
		return NULL;
	}
	else {
		void * pvRetval = m_plnListNext->m_pvThing;
		m_plnListNext = m_plnListNext->m_plnNext;
		return pvRetval;
	}
}

