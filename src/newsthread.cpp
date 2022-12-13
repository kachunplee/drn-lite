#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>

#include "def.h"
#include "szptr.h"
#include "status.h"

#include "newsthread.h"

const char * NewsThread::GetFileName () { return "/thread.i"; }

NewsThread::~NewsThread ()
{
	DeletePThreads();
}

ostream &
NewsThread::OutSubject (ostream & stm, int n,
	const char * grp, const char * artno, const char * subj,
		int pagelen, int iFile)
{
	int nDepth = m_pThg[n].idx >> IDX_BIT;
	if(nDepth)
	{
		for(int n = 0; n < nDepth; n++)
			stm << "  ";

//		static const char re [] = {0x7c, 0};
//		subj = re;
	}

	return NewsListIndex::OutSubject(stm, n, grp, artno, subj, pagelen, iFile);
}

//
// Read headers from overview file
//
BOOL
NewsThread::BegOver ()
{
	m_pThreads = new ThreadStruct* [m_nArtNum];
	if(m_pThreads == NULL)
		return FALSE;

	for(int i = 0; i < m_nArtNum; i++ )
		m_pThreads[i] = NULL;

	return TRUE;
}

BOOL
NewsThread::SetOver (int n, ssize_t off, char * over)
{
	if(m_pThreads[n] == NULL)
	{
		m_pThreads[n] = new ThreadStruct;

		m_pThreads[n]->thg.off = (streamoff) off;
		m_pThreads[n]->thg.artnum = m_ArtNumIdx[n].artnum;
		m_pThreads[n]->out = 0;
		m_pThreads[n]->link = NULL;

		m_nIndex++;
	}

	char * p = over;
	char *pMsgId = NULL, *pRef = NULL;
	for( int nField=1; *p; p++ )
	{
		switch(*p)
		{
		case '\t':
			nField++;
			if (nField == FIELD_MSGID)
				pMsgId = p+1;
			else if (nField == FIELD_REF)
				pRef = p+1;
			else if (nField == FIELD_BYTES)
				m_pThreads[n]->thg.bytes = strtoul(p+1, NULL, 10);
		case '\r':
		case '\n':
			*p = 0;
		}
	}

	m_pThreads[n]->msgid = pMsgId;
	m_pThreads[n]->ref = pRef;

	return TRUE;
}

BOOL
NewsThread::EndOver ()
{
	//
	// compact array
	//
	int j = 0;
	for(int i = 0; i < m_nArtNum; i++)
	{
		if(m_pThreads[i])
		{
			m_pThreads[j] = m_pThreads[i];
			m_pThreads[j]->thg.idx = j;
			++j;
		}
	}

	ASSERT(j == m_nIndex);
	return TRUE;
} 

//
// Thread Sort
//

class ThreadLink
{
public:
	ThreadLink ()
	{ m_me = NULL; m_parent = m_children = NULL; m_peer = NULL;
		m_loop = 0; }

	friend class NewsThread;
protected:
	ThreadStruct *	m_me;
	ThreadLink *	m_parent;
	ThreadLink *	m_children;
	ThreadLink *	m_peer;
	short			m_loop;
};

typedef map<szptr, ThreadLink> mapthread;

BOOL
NewsThread::Sort ()
{
	mapthread tdmap;
	int i;

	//
	// m_pThreads should be already sort article number
	//
	// Find parent
	//
	m_status("Finding thread parent...");

	ThreadLink *link, *parent;
	for(i = 0; i < m_nIndex; i++)
	{
		link = &tdmap[m_pThreads[i]->msgid.chars()];
		link->m_me = m_pThreads[i];
		m_pThreads[i]->link = link;
		if(link->m_parent == NULL)
		{
			// try to find parent
			char * p = (char *)m_pThreads[i]->ref.chars();
			if(p[0])
			{
				// has ref header
				char * q;
				for(q = p + strlen(p); q > p; )
				{
					while(q >= p && *q != ' ') --q;
					if(q[1] == '<')		// sanity check
					{
						parent = &tdmap[q+1];
						link->m_parent = parent;		
						if(parent->m_parent   // parent already set
						   || parent->m_me)
							// this link has an article
							// and that article should know its parents better
							break;

						//
						// well, go up the ref header and chain the others
						//
						link = parent;
					}
					if(q[0] == ' ')
						*q-- = 0;
				}
			}
		}
	}

	m_status("Linking threads to parent...");
	for(i = 0; i < m_nIndex; ++i)
	{
		for(link = m_pThreads[i]->link,
				ASSERT(link->m_peer == NULL);
			(parent = link->m_parent)
			; link = parent)
		{

			ThreadLink * chain = parent->m_children;
			if(chain)
			{
				link->m_peer = chain->m_peer;
				chain->m_peer = link;
			}
			else
				link->m_peer = link;

			parent->m_children = link;

			if(parent->m_peer || parent->m_me)
				break;
		}
	}

#ifdef DEBUG
  if(debug > 1)
  {
	mapthread::iterator end = tdmap.end();
	mapthread::iterator iter;
            
	for( iter = tdmap.begin(); iter != end; ++iter)
	{
		if((*iter).second.m_me)
		{
			cerr << (*iter).second.m_me->thg.artnum;
		}
		cerr << "\t"
			<< (*iter).first << ' ' << (long) &(*iter)
			<< ' ' << (long) (*iter).second.m_parent
			<< ' ' << (long) (*iter).second.m_peer
			<< ' ' << (long) (*iter).second.m_children
			<< endl;
	}
  }
#endif

	//
	// Now generate the out map
	// latest thread first
	//
	m_Thgs = new IndexThg [m_nIndex];
	int nThg = 0;
	for(i = m_nIndex-1; i >= 0; --i)
	{
		if(m_pThreads[i]->out < 0)
			continue;						// already marked out

		// move to top of thread
		link = m_pThreads[i]->link;
		ASSERT(link != NULL);
		
		for(;;)
		{
			link->m_loop = 1;
			if((parent = link->m_parent) == NULL
				|| (parent->m_loop > 0))
				break;
			link = parent;
		}

		// output thread
		nThg = PutThread(nThg, link, 0);
	}

	DMSG(1, "nThg=%d, m_nIndex=%d\n", nThg, m_nIndex);
	ASSERT(nThg == m_nIndex);

#ifdef DEBUG
  if(debug > 1)
  {
	//
	for(i = 0; i < m_nIndex; i++)
	{
		ThreadStruct * pth = m_pThreads[m_Thgs[i].idx & IDX_MASK];
		cerr << m_Thgs[i].artnum 
			<< ' ' << pth->msgid;
		if((parent = pth->link->m_parent) && parent->m_me)
			cerr << ' ' << parent->m_me->thg.artnum
				<< ' ' << parent->m_me->msgid;
		cerr << endl;
	}
  }
#endif

	//
	// m_pThreads should not be needed any more...
	// borrow it to generate the artnum idx,
	// which needs for bsearch later.
	//
	// Copy the idx over
	//
	for(int i = 0; i < m_nIndex; i++)
		m_pThreads[i]->thg.idx = m_Thgs[i].idx & IDX_MASK;

	for(int i = 0; i < m_nIndex; i++)
	{
		m_Thgs[m_pThreads[i]->thg.idx].idx &= ~IDX_MASK;
		m_Thgs[m_pThreads[i]->thg.idx].idx |= i;
	}

	//
	// All done with m_pThreads
	//
	DeletePThreads();
	m_status.SetDefault();
	return TRUE;
}

//
// Recursive put thread into index
//
int
NewsThread::PutThread (int nThg, ThreadLink * link, unsigned nDepth)
{
		if(link->m_loop == 2)
			return nThg;

		if(link->m_me)
		{
			if(link->m_me->out == -1)
				return nThg;

			ASSERT(nThg < m_nIndex);
			m_Thgs [nThg] = link->m_me->thg;
			m_Thgs [nThg++].idx |=
				((nDepth>=IDX_SPC)?(IDX_SPC-1):nDepth) << IDX_BIT;
			++nDepth;

			link->m_me->out = -1;
		}

		link->m_loop = 2;

		ThreadLink * child = link->m_children;
		if(child == NULL)
			return nThg;

		do
		{
			child = child->m_peer;
			ASSERT(child != NULL);
			nThg = PutThread(nThg, child, nDepth);
		}
		while(child != link->m_children);
		return nThg;
}

void
NewsThread::DeletePThreads ()
{
	if(m_pThreads)
	{
		for(int i = 0; i < m_nIndex; i++)
			delete m_pThreads[i];

		delete [] m_pThreads;
		m_pThreads = NULL;
	}
}
