#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#ifndef FREEBSD
#include <utime.h>
#include <sys/file.h>
#endif

#include "def.h"
#include "newslover.h"

#include "nntpov.h"
const char f_artnum_i [] = "artnum.i";

extern BOOL MakeDirectory(const char *);
extern BOOL MakeOVDirectory(const char *);

//
NewsListOver::NewsListOver (const char * grp,
		const char * dirGroup, ostream * poutStat, int min)
	: NewsListIndex(grp, dirGroup, poutStat, min)
{
	//
	// index file desc
	//
	m_fd = -1;
	m_nIndex = 0;
	m_sizeIdx = 0;

	// ARTNUM list
	m_nArtNum = 0;
	m_ArtNumIdx = NULL;

	//
	m_Thgs = m_pThg = NULL;

	//
	OpenOver();
}

NewsListOver::~NewsListOver ()
{
	delete m_Thgs;

	delete [] m_ArtNumIdx;

	if(m_fd >= 0) close(m_fd);
	m_fileOver.close();
}

BOOL
NewsListOver::OpenOver ()
{
	if(m_fileOver.is_open())
		return TRUE;

	ZString ovf(OVERVIEWDIR);
	ovf += "/";
	ovf += m_dirGroup;
	ovf += "/";
	ovf += ".overview";

	struct stat sb;
	for(int retry = 0; retry < 2; retry++)
	{
		m_fileOver.open(ovf, ios::in|ios::nocreate);
		if(m_fileOver.good()
			&& fstat(m_fileOver.rdbuf()->fd(), &sb) >= 0
			&& sb.st_size > 0)
		{
#ifdef FREEBSD
			m_mtimeOver = sb.st_mtimespec;
			if((time(NULL) - m_mtimeOver.tv_sec) < OV_DELAY)
#else
			m_mtimeOver = sb.st_mtime;
			if((time(NULL) - m_mtimeOver) < OV_DELAY)
#endif
				return TRUE;
		}
		m_fileOver.close();

		if(retry > 0)
			break;

		m_status("Reading news list from server...");

		if(!MakeOVDirectory(m_dirGroup))
			break;
		
		if(chdir(m_dirGroup) != 0)
		{
			DMSG(0, "Cannot chdir to %s", m_dirGroup);
			break;
		}

		int fd = open(f_artnum_i, O_CREAT|O_WRONLY|O_TRUNC,0664);
		chdir(NEWSINDEX);					// for core dump
		if(fd < 0)
		{
			DMSG(0, "Cannot create %s", f_artnum_i);
			break;
		}

		ZString tovf = ovf;
		tovf += ".tmp";
		ofstream of(tovf);

		int n;
		char * p;
		try
		{
			NNTPOverview art(m_pGroup);
			m_ArtNumIdx = new ArtNumIdx [n=art.GetNumArt()];

			streamoff off = 0;
			m_nArtNum = 0;

			int percent = 0;
			int inc = n / 20 + 1;
			while((p=art.GetLine()))
			{
				if(m_nArtNum < n)	// enlarge array !!
				{
					m_ArtNumIdx[m_nArtNum].off = off;
					m_ArtNumIdx[m_nArtNum++].artnum = atol(p);
				}
				of << p << '\n';
				off += strlen(p) + 1;
				if((m_nArtNum % inc) == 0 && percent < 100)
				{
					percent += 5;
					if(percent > 100) percent = 100;
					m_status("Read %d (%d%%) headers", m_nArtNum, percent);
				}
			}
		}
		catch(const char * p)
		{
			DMSG(0, "NNTPOverview: %s", p);
			break;
		}
		catch(int err)
		{
			DMSG(0, "NNTPOverview: &s", strerror(errno));
			break;
		}

		n = m_nArtNum*sizeof(ArtNumIdx);
		if(write(fd, m_ArtNumIdx, n) != n)
		{
			DMSG(0, "NNTPOverview: write index error - &s", strerror(errno));
			break;
		}

		close(fd);
		of.close();
		rename(tovf, ovf);
		m_status.SetDefault();
	}

	m_status.SetDefault();
	return FALSE;
}

int
NewsListOver::SetArtNum ()
{
	return (m_nArticle = m_nIndex ? m_nIndex : m_sizeIdx/sizeof(IndexThg));
}

int
NewsListOver::Fetch ()
{
	if(!m_fileOver.is_open())
		return -1;

	m_stgFileName = NEWSINDEX;
	m_stgFileName += m_dirGroup;
	m_stgFileName += GetFileName();

	DMSG(1, "%s", m_stgFileName.chars());

	struct stat sb;
	for(int retry = 0; retry < 5; retry++)
	{
		if(!OpenOver())
			return -1;

		// Open index for read
#ifdef FREEBSD
		m_fd = open(m_stgFileName, O_RDWR|O_EXLOCK);
#else
		m_fd = open(m_stgFileName, O_RDWR);
#endif
		if(m_fd >= 0)
		{
#ifdef LINUX
			flock(m_fd, LOCK_EX);
#endif
#ifdef SOLARIS
			struct flock lock_it;
			int ret;
    		lock_it.l_whence = SEEK_SET;    /* from current point */
    		lock_it.l_start = 0;        /* -"- */
    		lock_it.l_len = 0;          /* until end of file */
    		lock_it.l_type = F_WRLCK;       /* set exclusive/write lock */
    		lock_it.l_pid = 0;
			while((ret = fcntl(m_fd, F_SETLKW, &lock_it)) < 0 && errno == EINTR)			;
#endif
			if(fstat(m_fd, &sb) >= 0)
			{
				// Index Exist
				m_sizeIdx = sb.st_size;
				if(sb.st_size > 0
#ifdef FREEBSD
					&& (m_mtimeOver.tv_sec <= sb.st_mtimespec.tv_sec
					|| (time(NULL) - sb.st_mtimespec.tv_sec)
#else
					&& (m_mtimeOver <= sb.st_mtime
					|| (time(NULL) - sb.st_mtime)
#endif
							< GetDelayTime(m_pGroup)))
				{
					//
					// Finish some initialization
					//
					return SetArtNum();
				}
				else
				{
					// Index too old
					if(ftruncate(m_fd, 0) >= 0)	// trunicate
					{
						// ready to build index to fd it
						return Make();
			
					}
				}
			}

			// Can't stat/read/truncate - remove file and try again
			Bad();
		}

		// Index Not Exist
		// Try to create it
		
		if(!MakeOVDirectory(m_dirGroup))
			break;

#ifdef FREEBSD
		m_fd = open(m_stgFileName, O_CREAT|O_RDWR|O_EXCL|O_EXLOCK,0664);
#else
		m_fd = open(m_stgFileName, O_CREAT|O_RDWR|O_EXCL,0664);
#endif
		if(m_fd >= 0)
		{
#ifdef LINUX
			flock(m_fd, LOCK_EX);
#endif
#ifdef SOLARIS
			struct flock lock_it;
			int ret;
    		lock_it.l_whence = SEEK_SET;    /* from current point */
    		lock_it.l_start = 0;        /* -"- */
    		lock_it.l_len = 0;          /* until end of file */
    		lock_it.l_type = F_WRLCK;       /* set exclusive/write lock */
    		lock_it.l_pid = 0;
			while((ret = fcntl(m_fd, F_SETLKW, &lock_it)) < 0 && errno == EINTR)			;
#endif
			// file created OK, let's build it
			return Make();
		}
		else
			if(errno != EEXIST)
				break;
	}

	// can't build index
	return -1;
}

BOOL
NewsListOver::WriteIndex ()
{
	if(m_nIndex == 0)				// no index - should not have get here
		return TRUE;				//	nothing to write
		
	lseek(m_fd, (off_t)0, SEEK_SET);		// just to be sure
	ssize_t size = m_nIndex * sizeof(IndexThg);
	ssize_t l = write(m_fd, m_Thgs, size);
	close(m_fd);
	m_fd = -1;
	return (l == size);
}

int
NewsListOver::Set(int nBeg, int nGet, int nDir)
{
	if(nDir < 0)
	{
	    nBeg =  m_nArticle - nBeg - nGet;
		if(nBeg < 0)
		{
			nGet += nBeg;
			nBeg = 0;
		}
	}
	else
		if((nGet+nBeg) > m_nArticle) nGet = m_nArticle-nBeg;

	if(nGet <= 0)
		return nGet;

	//
	// if m_nIndex != 0 => index is already generated in m_Thgs
	//
	if(m_nIndex)
	{
		if(m_nIndex > 2*nGet)
		{
			//
			// Copy the needed and free the big Index
			//
			m_pThg = new IndexThg [nGet];
			for(int i = 0; i < nGet; i++)
				m_pThg[i] = m_Thgs[i+nBeg];
			delete [] m_Thgs;
			m_Thgs = m_pThg;
			m_nIndex = nGet;
			return nGet;
		}

		m_pThg = &m_Thgs[nBeg];
		return nGet;
	}

	m_nIndex = nGet;
	m_Thgs = new IndexThg [m_nIndex];
	m_pThg = m_Thgs;
	if(lseek(m_fd, nBeg * sizeof(IndexThg), SEEK_SET) < 0)
		return FALSE;

	ssize_t size = m_nIndex * sizeof(IndexThg);
	ssize_t l = read(m_fd, m_pThg, size);

	close(m_fd);
	m_fd = -1;
 	return (l == size) ? nGet : -1;
}

ARTNUM
NewsListOver::GetArtNum (int n)
{
	if(n < 0 || n >= m_nIndex)
		return -1;

	if(m_pThg == NULL) m_pThg = m_Thgs;
	return m_pThg ? m_pThg[n].artnum : -1;
}

unsigned
NewsListOver::GetSize (int n)
{
	return m_pThg[n].bytes & BYTE_MASK;
}

unsigned
NewsListOver::GetArtIdx (int n)
{
	return m_pThg[n].idx;
}

unsigned
NewsListOver::GetBytes (int n)
{
	return m_pThg[n].bytes;
}

//
size_t NewsListOver::operator [] (int n)
{
	return (size_t) (m_pThg ? m_pThg[n].off : 0);
}

struct cmpKey
{
	IndexThg * base;
	ARTNUM artnum;
};

static int
CmpArtNumKey (const void * p1, const void * p2)
{
	ARTNUM artnum =
		(((cmpKey*)p1)->base)[((IndexThg *)p2)->idx & IDX_MASK].artnum;
	DMSG(1, "artnum = %ld", artnum);
	return ((cmpKey*)p1)->artnum - artnum;
}

int
NewsListOver::Find (ARTNUM artno)
{
	if(m_nIndex == 0)
	{
		//
		// Get Index
		//
		m_nIndex = m_sizeIdx/sizeof(IndexThg);
		m_Thgs = new IndexThg [m_nIndex];
		ssize_t size = sizeof(IndexThg) * m_nIndex;

		lseek(m_fd, (off_t)0, SEEK_SET);
		ssize_t l = read(m_fd, m_Thgs, size);
		close(m_fd);
		m_fd = -1;

		if(l != size)
		{
			DMSG(0, "index file read error %s", strerror);
			m_nIndex = 0;
			delete m_Thgs;
			m_Thgs = NULL;
			return -1;
		}
	}

	struct cmpKey key;
	key.base = m_Thgs;
	key.artnum = artno;

	IndexThg * ret = (IndexThg *) bsearch(&key, m_Thgs, m_nIndex,
			sizeof(IndexThg), CmpArtNumKey);
	return ret ? (int)(ret->idx&IDX_MASK) : -1;
}

BOOL
NewsListOver::Get (int idx, ZString & stg)
{
	m_fileOver.seekg((*this)[idx]);
	return (readline(m_fileOver, stg) > 0);
}

void
NewsListOver::Bad ()
{
	if(m_fd >= 0)
	{
		close(m_fd);
		m_fd = -1;
	}

	remove(m_stgFileName.chars());
}

//
//
//
int
NewsListOver::Make ()
{
	if(GetHeaders() && Sort() && Save())
		return SetArtNum();

	if(m_fd >= 0)
	{
		close(m_fd);
		m_fd = -1;
	}
	m_fileOver.close();

	return -1;
}

BOOL
NewsListOver::GetHeaders ()
{
  	if(!GetArtNumList())
		return FALSE;

	if(m_nArtNum == 0)
		return FALSE;

	if(!BegOver())
		return FALSE;

	size_t off;
	ZString tmpStg;
	int idx = 0;
	ARTNUM artnum;
	int percent = 0;
	int inc = m_nArtNum / 20 + 1;

	m_fileOver.seekg(0);
	for(m_nIndex = 0; m_nIndex < m_nArtNum; )
	{
		if(!m_fileOver.good())
			break;

		off = m_fileOver.tellg();
		if(readline(m_fileOver, tmpStg) <= 0)
			break;

		artnum = atol(tmpStg);

		//
		// Look for article in the ArtNumIdx array...
		//	since it should be close to last article
		//	we will start from the last idx
		//
		if(artnum > m_ArtNumIdx[idx].artnum)
			while(idx < (m_nArtNum-1) && artnum > m_ArtNumIdx[idx].artnum)
				++idx;
		else
			while(idx > 0 && artnum < m_ArtNumIdx[idx].artnum)
				--idx;

		//
		// Did we find it?
		//
		if(artnum != m_ArtNumIdx[idx].artnum)
			// Nop!
			continue;

		if(!SetOver(idx, off, (char*)tmpStg.chars()))
			break;

		if((m_nIndex % inc) == 0 && percent < 100)
		{
			percent += 5;
			if(percent > 100) percent = 100;
			m_status("Processed %d (%d%%) headers", m_nIndex, percent);
		}
	}

	BOOL ret = EndOver();

	m_fileOver.clear();			// clear eof bit in particular

	delete [] m_ArtNumIdx;
	m_ArtNumIdx = NULL;
	m_nArtNum = 0;

	return ret;
}

BOOL
NewsListOver::Save ()
{
	if(!WriteIndex())
	{
		Bad();
		return FALSE;
	}
		

#ifdef FREEBSD
	struct timeval times[2];
	TIMESPEC_TO_TIMEVAL(&times[0], &m_mtimeOver);
	TIMESPEC_TO_TIMEVAL(&times[1], &m_mtimeOver);
	utimes(m_stgFileName, times); 
#else
	struct utimbuf times;
	times.actime = times.modtime = m_mtimeOver;
	utime(m_stgFileName, &times); 
#endif
	return TRUE;
}

//
// Helpers
//
int
NewsListOver::GetDelayTime (const char * grp)
{
	int nSec = OV_IDX_DELAY;

#ifdef INDEX_UPDATE
	ZString szUpdFileName(NEWSUPDATE);
	szUpdFileName += grp;
	Zifstream updFile(szUpdFileName, ios::nocreate|ios::in);
	if(updFile && updFile.good())
	{
		char buf[12];
		buf[10] = '\0';
		updFile.read(buf, 10);
		nSec = atoi(buf);
	}
#endif
	return nSec;
}

//
#if 0
static int
CmpArtNum (const void * p1, const void * p2)
{
	return ((ArtNumIdx *)p1)->artnum - ((ArtNumIdx *)p2)->artnum;
}
#endif

BOOL
NewsListOver::GetArtNumList ()
{
	if(m_ArtNumIdx)
		return TRUE;

	m_status("Reading ArtNum Index...");

	if(chdir(NEWSINDEX) != 0 || chdir(m_dirGroup) != 0)
	{
		DMSG(0, "cant chdir %s %m", m_dirGroup);
		return FALSE;
	}

	struct stat sb;
	int fd = open(f_artnum_i, O_RDONLY);
	if(fd < 0
		|| fstat(fd, &sb) < 0)
	{
		DMSG(0, "Cannot open %s %s", m_dirGroup, f_artnum_i);
		return FALSE;
	}
	chdir(NEWSINDEX);					// for core dump

	m_nArtNum = sb.st_size / sizeof(ArtNumIdx);
	m_ArtNumIdx = new ArtNumIdx[m_nArtNum];

	int n = m_nArtNum * sizeof(ArtNumIdx);
	int l = read(fd, m_ArtNumIdx, n);
	close(fd);

	m_status.SetDefault();

	if(n != l)
	{
		DMSG(0, "GetArtNumList: Read error %s", strerror(errno));
		return FALSE;
	}

#if 0
	qsort(m_ArtNumIdx, m_nArtNum, sizeof(m_ArtNumIdx[0]), CmpArtNum);
#endif
	return TRUE;
}

//
// If multi-part, display number of articles instead of number of lines
//
int
NewsListOver::GetMultiSum (int n)
{
	return ((m_pThg[n].bytes&PART_MULTI) ? NLINE_MULTI : NLINE_BINARC) 
			| ((m_pThg[n].bytes&PART_COMPLETE) ? NLINE_COMPLETED : 0)
			| ((m_pThg[n].idx>>IDX_BIT) & PART_MASK);
}

//
// If multi-part, display article list instead of article
//
ostream &
NewsListOver::OutSubjectSum (ostream & stm, int n,
	const char * grp, const char * artno, const char * subj,
	int pagelen, int iFile)
{
	int nPart = (m_pThg[n].idx>>IDX_BIT) & PART_MASK;
	if(nPart <= 1)
		return NewsListIndex::OutSubject(stm,n,grp,artno,subj,pagelen, iFile);

	OutURLSum(stm, n, grp, artno, pagelen);
	if(iFile < 0)
		stm	<< HTMLText(subj);
	else
	{
		for(;iFile >= 0; --iFile)
			if(isspace(subj[iFile])) break;

		for(;iFile >= 0; --iFile)
			if(!isspace(subj[iFile])) break;

		if(iFile >= 0)
			stm << HTMLText(ZString(subj, iFile+1));

		stm << " ...";
	}

	stm << "</a>";
	return stm;
}

ostream &
NewsListOver::OutURLSum (ostream & stm, int n,
	const char * grp, const char * artno, int pagelen)
{
	int nPart = (m_pThg[n].idx>>IDX_BIT) & PART_MASK;
	if(nPart <= 1)
		return NewsListIndex::OutURL(stm,n,grp,artno,pagelen);

	stm	<< "<a href=\"" << NEWSBIN << "/wwwnews?"
		<< "-ss,b-" << artno << ",o";
	if(nPart <= pagelen)
		stm << ",n" << nPart;
	else
		stm << "0";
	stm	<< "+" << URLText(grp) << "\">";
	return stm;
}
