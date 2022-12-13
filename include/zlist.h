#ifndef __ZLIST_H__
#define __ZLIST_H__

/*
	Class: ZList
	Description:
		A simple ordered collection class.  Provides for adding
		a set of things to an ordered list and iterating over them.
		No provision for finding a particular entry.
*/
class ZList {
private:
	// List structure is a simple linked list with each node composed
	// of a pointer to the next node and a pointer to the data.
	// The data pointer is void because we don't know what it will
    // store (and, frankly, don't care).
	struct ListNode {
		ListNode *	m_plnNext;
		void *		m_pvThing;

		// Constructors
		ListNode(ListNode *pln, void * pv) : m_plnNext(pln),
			m_pvThing(pv) {};
		ListNode() {};
	};

	ListNode	m_lnListHead;
	ListNode *	m_plnListNext;

	// Compare returns -1, 0, 1 for <, =, >.
	virtual int Compare(void * pvThing1, void * pvThing2) = 0;
	// Deletes that data pointed to by pvThing before the list node
    // itself is deleted in ZList::Delete.
    virtual void DeleteData(void * pvThing) = 0;
public:
	// Constructor and destructor.
	ZList() : m_lnListHead(NULL, NULL), m_plnListNext(NULL) {};
	virtual ~ZList();

	// Manipulation functions.
	void Add(void * pvThing);
	void Insert(void * pvThing);
    void Delete(void * pvThing);
	void * First(void);
	void * Next(void);
};

#endif // __ZLIST_H__

