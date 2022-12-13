#include "def.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

#include "artspool.h"
#include "hexbin.h"

static char fname_trans_table[] =
"@ABCDEFGHIJKLMNOPQRSTUVWXYZexxxx_xxxxxxxxxxxxx.x0123456789xxxxxxxABCDEFGHIJKLMNOPQRSTUVWXYZxxxx_xabcdefghijklmnopqrstuvwxyzxxxxxxABCDEFGHIJKLMNOPQRSTUVWXYZexxxxxxxxxxxxxxxxxxxx0123456789xxxxxxxABCDEFGHIJKLMNOPQRSTUVWXYZxxxx_xabcdefghijklmnopqrstuvwxyzxxxxxx";

#ifdef MAXNAMLEN
#define FNAMELEN MAXNAMLEN
#else
#define FNAMELEN DIRSIZ
#endif

#include <sys/time.h>
#include <sys/timeb.h>
#define search_last rindex

static int eof;
static char obuf[3];
static char *op, *oend;

/*
 * xbin -- unpack BinHex format file into suitable
 * format for downloading with macput
 * Dave Johnson, Brown University Computer Science
 *
 * checksum code by Darin Adler, TMQ Software
 *
 * modification to support MacBinary output file by Earle Horton
 *  uses code from Jim Budler's "macbin.c"
 * 
 * modification to this modification to create correct MacBinary files
 *  by Johannes Endres (endres@uni-muenster.de)
 *
 * (c) 1984 Brown University 
 * may be used but not sold without permission
 *
 * created ddj 12/16/84 
 * revised ddj 03/10/85 -- version 4.0 compatibility, other minor mods
 * revised ddj 03/11/85 -- strip LOCKED bit from m_flags
 * revised ahm 03/12/85 -- System V compatibility
 * revised dba 03/16/85 -- 4.0 EOF fixed, 4.0 checksum added
 * revised ddj 03/17/85 -- extend new features to older formats: -l, stdin
 * revised ddj 03/24/85 -- check for filename truncation, allow multiple files
 * revised ddj 03/26/85 -- fixed USG botches, many problems w/multiple files
 * revised jcb 03/30/85 -- revised for compatibility with 16-bit int
 * revised erh 07/16/88 -- revised to support MacBinary output with "-b"
 * revised jne 09/20/95 -- fixed MacBinary output
 */
char usage[] = "usage: \"hexbin filename\"\n";

/*
Zifstream * m_pDecodeFile;
int main(int ac, char **av)
{
	if(ac != 2)
	{
		fprintf(stderr, usage);
		exit(1);
	}
	char * filename = av[1];
	if (filename[0] == '\0')
	{
		fprintf(stderr, usage);
		exit(1);
	}
	m_pDecodeFile = new Zifstream(filename, ios::nocreate|ios::in);
	if(!m_pDecodeFile || !m_pDecodeFile->good())
	{
		delete m_pDecodeFile;
		perror("input file");
		exit(-1);
	}
	ZHexBin HexBin;
	int n = FindHeader();
	if(n == -1)
	{
		m_pDecodeFile->close();
		delete m_pDecodeFile;
		fprintf(stderr, "unexpected EOF\n");
		exit(2);
	}
	HexBin.SetQFormat(n);
	if(!HexBin.SetupFiles(filename))
	{
		m_pDecodeFile->close();
		delete m_pDecodeFile;
		fprintf(stderr, "unexpected EOF\n");
		exit(2);
	}
	if(!HexBin.Open(HexBin.m_FileInfo.m_info))
	{
		m_pDecodeFile->close();
		delete m_pDecodeFile;
		perror("output file");
		exit(-1);
	}

	switch(HexBin.ProcessForks())
	{
	case HB_EOF:
		m_pDecodeFile->close();
		delete m_pDecodeFile;
		fprintf(stderr, "unexpected EOF\n");
		exit(2);

	case HB_ERR_OPEN:
		m_pDecodeFile->close();
		delete m_pDecodeFile;
		perror(fname);
		exit(-1);

	case HB_ERR_CRC:
		m_pDecodeFile->close();
		delete m_pDecodeFile;
		fprintf(stderr, "missing CRC\n");
		exit(3);
	}

	// now that we know the size of the forks
	if(HexBin.ForgeInfo() == HB_ERR_OPEN)
		perror("output file");

	m_pDecodeFile->close();
	delete m_pDecodeFile;
} */

BOOL ZHexBin::Init (ArticleSpool * partspool, ZString & DecodeName,
	int nTotalPart, ZString * pArtNums, const char * pNewsName)
{
	m_nTotalPart = nTotalPart;
	m_pArtNums = pArtNums;
	m_pNewsName = pNewsName;
	m_nCurPart = 1;
	m_pArtSpool = partspool;
	if((m_nQFormat = FindHeader()) == -1)
		return FALSE;
// Note: doesn't support old format yet, need a real filename when support
	if(!SetupFiles(DecodeName.chars()))
		return FALSE;

	DecodeName = m_MacHeader.m_name;
	BOOL bFoundExt = FALSE;
	for(int n = DecodeName.length() - 1; n >= 0 ; n--)
	{
		// replace all the '.' with '_' if it is not used to seperate the
		//	file extension
		if(DecodeName[n] == '.')
		{
			if(bFoundExt)
				DecodeName[n] = '_';
			else
				bFoundExt = TRUE;
		}
		else if(isspace(DecodeName[n]))
			DecodeName[n] = '_';
	}
	return TRUE;
}

//
//	eat characters until header detected, return which format
//
int ZHexBin::FindHeader ()
{
/* Not need because it is already found in the calling place
	// look for "(This file ...)" line
	char * p;
	while((p = m_pArtSpool->GetLine()))
	{
		if (strncmp(p, "(This file", 10) == 0)
			break;
	}
*/

	char c;
	int at_bol = 1;
	while((c = m_pArtSpool->Getc()) != EOF)
	{
		switch (c)
		{
		case '\n':
		case '\r':
			at_bol = 1;
			break;
		case ':':
			if (at_bol)				// q format
				return 1;
			break;
/* Doesn't support the old format yet
		case '#':
			if (at_bol)
			{
				// old format
				m_pArtSpool->putback(c);
				return 0;
			}
			break;
*/
		default:
			at_bol = 0;
			break;
		}
	}

	return -1;
}

BOOL ZHexBin::SetupFiles (const char * filename)
{
	char	namebuf[256], *np;
	int		n;
	long	curtime;

	curtime = time(0);
	m_MacHeader.m_createtime = curtime;
	m_MacHeader.m_modifytime = curtime;

	/* eat mailer header &cetera, intuit format */

	if (m_nQFormat == 1)
	{
		if(!DoQHeader())
			return FALSE;
	}
	else
	{
		if(!DoOHeader(filename))
			return FALSE;
	}

	/* make sure host file name doesn't get truncated beyond recognition */
	n = strlen(m_MacHeader.m_name);
	if (n > FNAMELEN - 2)
		n = FNAMELEN - 2;
	strncpy(namebuf, m_MacHeader.m_name, n);
	namebuf[n] = '\0';

	/* get rid of troublesome characters */
	for (np = namebuf; *np; np++)
		/* Negative chars on some systems make lousy subscripts. */
		*np = fname_trans_table[(unsigned) *np];

	sprintf(m_FileInfo.m_info, "%s", namebuf);
	return TRUE;
}

//
// read header of .hqx file
//
BOOL ZHexBin::DoQHeader ()
{
	char namebuf[256];		/* big enough for both att & bsd */
	int n;
	int calc_crc, file_crc;

	m_uCRC = 0;			/* compute a crc for the header */
	QInit();			/* reset static variables */

	n = GetQByte();			/* namelength */
	if(n < 0)
		return FALSE;
	n++;				/* must read trailing null also */
	if(GetQBuf(namebuf, n) < 0)	/* read name */
		return FALSE;

	n = strlen(namebuf);
	if (n > NAMEBYTES)
		n = NAMEBYTES;

	strncpy(m_MacHeader.m_name, namebuf, n);
	m_MacHeader.m_name[n] = '\0';

	if(GetQBuf(m_MacHeader.m_type, 4) < 0)
		return FALSE;
	if(GetQBuf(m_MacHeader.m_author, 4) < 0)
		return FALSE;
	if((m_MacHeader.m_flags = Get2Q()) < 0)
		return FALSE;
	if((m_MacHeader.m_datalen = Get4Q()) < 0)
		return FALSE;
	if((m_MacHeader.m_rsrclen = Get4Q()) < 0)
		return FALSE;

	CompQCRC(0);
	CompQCRC(0);
	calc_crc = m_uCRC;
	if((file_crc = Get2Q()) == 0)
		return FALSE;
	if(VerifyCRC(calc_crc, file_crc) == HB_ERR_CRC)
		return FALSE;
	return TRUE;
}

//
// old format -- process .hex and .hcx files */
//
BOOL ZHexBin::DoOHeader (const char * filename)
{
	char namebuf[256];		/* big enough for both att & bsd */

	/* set up name for output files */
	strcpy(namebuf, filename);

	/* strip directories */
	char * p = search_last(namebuf, '/');
	if (p == NULL)
		p = namebuf;
	else
		p++;

	/* strip extension */
	int n = strlen(p);
	if (n > 4)
	{
		n -= 4;
		if (p[n] == '.' && p[n + 1] == 'h'
			&& p[n + 3] == 'x')
			p[n] = '\0';
	}
	n = strlen(p);
	if (n > NAMEBYTES)
		n = NAMEBYTES;
	strncpy(m_MacHeader.m_name, p, n);
	m_MacHeader.m_name[n] = '\0';

	/* read "#TYPEAUTH$flag" line */
	p = m_pArtSpool->GetLine();
	if(p == NULL)
		return FALSE;
	n = strlen(p);
	if (n >= 7 && p[0] == '#' && p[n - 6] == '$')
	{
		if (n >= 11)
			strncpy(m_MacHeader.m_type, &p[1], 4);
		if (n >= 15)
			strncpy(m_MacHeader.m_author, &p[5], 4);
		sscanf(&p[n - 5], "%4hx", &m_MacHeader.m_flags);
	}
	return TRUE;
}

BOOL ZHexBin::Open (const char * pName)
{
	m_FileName = DECODEDIR;
	m_FileName += pName;
	return OpenOutFile();
}

BOOL ZHexBin::OpenOutFile ()
{
	m_pOutFile = fopen(m_FileName, "w");
	if (m_pOutFile == NULL)
		return FALSE;
	chmod(m_FileName, 0644);
	for(int i = 128 ; i-- > 0;)
	{
		putc(0, m_pOutFile);		// Make space for info.
	}
	return TRUE;
}

void ZHexBin::CloseOutFile ()
{
	if(m_pOutFile)
	{
		fclose(m_pOutFile);
		m_pOutFile = NULL;
	}
}

int ZHexBin::Decode (ArticleSpool * partspool)
{
	if(partspool != m_pArtSpool)
	{
//??	delete m_pArtSpool;
		m_pArtSpool = partspool;
	}
	int n = ProcessForks();
	if(n < 0)
		return n;

	return ForgeInfo();
}

int ZHexBin::ProcessForks ()
{
	int n;
	if (m_nQFormat == 1)
	{
		/* read data and resource forks of .hqx file */
/*
		if(!m_bRsrc)
		{
			if(!m_bDataBegin)
			{
				m_bDataBegin = TRUE;
				if (!m_bMacBinary)
				{
					CloseOutFile();
					m_pOutFile = fopen(fname, "w");
					if (m_pOutFile == NULL)
					{
						return HB_ERR_OPEN;
					}
				}
				m_uCRC = 0;					// compute a crc for a fork
			}
*/
			n = DoQFork(m_FileInfo.m_data, m_MacHeader.m_datalen);
			if(n < 0)
				return n;
//		}

/*
		if (!m_bMacBinary)
		{
			CloseOutFile();
			m_pOutFile = fopen(fname, "w");
			if (m_pOutFile == NULL)
			{
				return HB_ERR_OPEN;
			}
		}
		m_uCRC = 0;			// compute a crc for a fork
*/
		n = DoQFork(m_FileInfo.m_rsrc, m_MacHeader.m_rsrclen);
	}
	else
	{
		n = DoOForks();
	}
	return n;
}

//
// return:
//		>0 if OK
//		-1 if EOF
//		-2 if Open file error
//
int ZHexBin::DoQFork (char * fname, long len)
{
	register int c, i;
	int calc_crc, file_crc;

	if (!m_bMacBinary)
	{
		CloseOutFile();
		m_pOutFile = fopen(fname, "w");
		if (m_pOutFile == NULL)
		{
			return HB_ERR_OPEN;
		}
	}
	m_uCRC = 0;			/* compute a crc for a fork */

	if (len)
		for (i = 0; i < len; i++)
		{
			if ((c = GetQByte()) < 0)
			{
				return c;
			}
			putc(c, m_pOutFile);
		}

	CompQCRC(0);
	CompQCRC(0);
	calc_crc = m_uCRC;
	if((file_crc = Get2Q()) == EOF)
		return file_crc;
	if(VerifyCRC(calc_crc, file_crc) == HB_ERR_CRC)
		return HB_ERR_CRC;
	if (!m_bMacBinary)
	{
		CloseOutFile();
	}
	else
	{
		/* Q&D hack: both forks have to be padded to next multiple of 128 */
		if((calc_crc=len%128))
			for(;calc_crc<128;calc_crc++)
				putc(0,m_pOutFile);
	}
	return 0;
}

int ZHexBin::DoOForks ()
{
	int forks = 0, found_crc = 0;
	int calc_crc = 0, file_crc = 0;

	m_uCRC = 0;				/* calculate a crc for both forks */

	/* create empty files ahead of time */
	if (!m_bMacBinary)
	{
		close(creat(m_FileInfo.m_data, 0666));
		close(creat(m_FileInfo.m_rsrc, 0666));
	}
	char * p;
	while (!found_crc && (p = m_pArtSpool->GetLine()))
	{
		if (forks == 0 && strncmp(p, "***COMPRESSED", 13) == 0)
		{
			m_nCompressed++;
			continue;
		}
		if (strncmp(p, "***DATA", 7) == 0)
		{
			m_MacHeader.m_datalen = MakeFile(m_FileInfo.m_data);
			forks++;
			continue;
		}
		if (strncmp(p, "***RESOURCE", 11) == 0)
		{
			m_MacHeader.m_rsrclen = MakeFile(m_FileInfo.m_rsrc);
			forks++;
			continue;
		}
		if (m_nCompressed && strncmp(p, "***CRC:", 7) == 0)
		{
			found_crc++;
			calc_crc = m_uCRC;
			sscanf(&p[7], "%x", &file_crc);
			break;
		}
		if (!m_nCompressed && strncmp(p, "***CHECKSUM:", 12) == 0)
		{
			found_crc++;
			calc_crc = m_uCRC & BYTEMASK;
			sscanf(&p[12], "%x", &file_crc);
			file_crc &= BYTEMASK;
			break;
		}
	}

	if (found_crc)
	{
		if(VerifyCRC(calc_crc, file_crc) == HB_ERR_CRC)
			return HB_ERR_CRC;
	}
	else
	{
		return HB_ERR_CRC;
	}
	return 0;
}

/* write out .info file from information in the mh structure */
int ZHexBin::ForgeInfo ()
{
	static char	buf[DATABYTES];
	char	*np;
	int		n;
	long	tdiff;
#if 0
	struct tm	*tp;
#ifdef BSD
	struct timeb	tbuf;
#else
	long	bs;
#endif
#endif

	if (m_bMacBinary)
		rewind(m_pOutFile);

	for (np = m_MacHeader.m_name; *np; np++)
		if (*np == '_')
			*np = ' ';

	buf[H_NLENOFF] = n = np - m_MacHeader.m_name;
	strncpy(buf + H_NAMEOFF, m_MacHeader.m_name, n);
	strncpy(buf + H_TYPEOFF, m_MacHeader.m_type, 4);
	strncpy(buf + H_AUTHOFF, m_MacHeader.m_author, 4);
	Put2(buf + H_FLAGOFF, m_MacHeader.m_flags & ~F_LOCKED);
	if (m_bPreBeta)
	{
		Put4(buf + H_OLD_DLENOFF, m_MacHeader.m_datalen);
		Put4(buf + H_OLD_RLENOFF, m_MacHeader.m_rsrclen);
	}
	else
	{
		Put4(buf + H_DLENOFF, m_MacHeader.m_datalen);
		Put4(buf + H_RLENOFF, m_MacHeader.m_rsrclen);

		/* convert unix file time to mac time format */
#if 0
#ifdef BSD
		ftime(&tbuf);
		tp = localtime(&tbuf.time);
		tdiff = TIMEDIFF - tbuf.timezone * 60;
		if (tp->tm_isdst)
			tdiff += 60 * 60;
		tdiff = TIMEDIFF;
#else
		/* I hope this is right! -andy */
		time(&bs);
		tp = localtime(&bs);
		tdiff = TIMEDIFF - timezone;
		if (tp->tm_isdst)
			tdiff += 60 * 60;
#endif
#endif
		tdiff = TIMEDIFF;

		Put4(buf + H_CTIMOFF, m_MacHeader.m_createtime + tdiff);
		Put4(buf + H_MTIMOFF, m_MacHeader.m_modifytime + tdiff);
	}
	if (m_pOutFile == NULL)
	{
		if(!OpenOutFile())
			return HB_ERR_OPEN;
	}
	if (m_bMacBinary)
	{
		if (buf[74] & 0x40)
			buf[81] = '\1';		/* protected */
		buf[74] = '\0';			/* MacBinary identifiers */
		buf[82] = '\0';
	}
	fwrite(buf, 1, DATABYTES, m_pOutFile);
	CloseOutFile();
	return 0;
}

/* VerifyCRC(); -- check if crc's check out */
int ZHexBin::VerifyCRC(unsigned int calc_crc, unsigned int file_crc)
{
	calc_crc &= WORDMASK;
	file_crc &= WORDMASK;

	if (calc_crc != file_crc)
	{
		return HB_ERR_CRC;
	}
	return 0;
}

/* initialize static variables for q format input */
void ZHexBin::QInit ()
{
	eof = 0;
	op = obuf;
	oend = obuf + sizeof obuf;
}

/* Get2Q(); q format -- read 2 bytes from input, return short */
short ZHexBin::Get2Q ()
{
	register int c;
	short value = 0;

	c = GetQByte();
	if(c < 0)
		return c;
	value = (c & BYTEMASK) << 8;
	c = GetQByte();
	if(c < 0)
		return c;
	value |= (c & BYTEMASK);

	return value;
}

/* Get4Q(); q format -- read 4 bytes from input, return long */
long ZHexBin::Get4Q()
{
	register int c, i;
	long value = 0L;

	for (i = 0; i < 4; i++)
	{
		if((c = GetQByte()) < 0)
			return c;
		value <<= 8;
		value |= (c & BYTEMASK);
	}
	return value;
}

/* GetQBuf(); q format -- read n characters from input into buf */
/*	All or nothing -- no partial buffer allowed */
int ZHexBin::GetQBuf (char * buf, int n)
{
	register int c, i;

	for (i = 0; i < n; i++)
	{
		if ((c = GetQByte()) < 0)
			return c;
		*buf++ = c;
	}
	return 0;
}

#define RUNCHAR 0x90

/* q format -- return one byte per call, keeping track of run codes */
int ZHexBin::GetQByte ()
{
	register int c;

	if ((c = GetQNoCRC()) < 0)
		return c;
	CompQCRC(c);
	return c;
}

int ZHexBin::GetQNoCRC ()
{
	static int rep, lastc;
	int c;

	if (rep)
	{
		rep--;
		return lastc;
	}
	if ((c = GetQRaw()) == EOF)
		return HB_EOF;
	else if(c == HB_ERR_PART)
		return c;
	if (c == RUNCHAR)
	{
		if ((rep = GetQRaw()) == EOF)
			return HB_EOF;
		else if(rep == HB_ERR_PART)
			return c;
		if (rep == 0)
		{
			return lastc = RUNCHAR;
		}
		else
		{
			/* already returned one, about to return another */
			rep -= 2;
			return lastc;
		}
	}
	else
	{
		lastc = c;
		return c;
	}
}

/* q format -- return next 8 bits from file without interpreting run codes */
int ZHexBin::GetQRaw ()
{
	char ibuf[4];
	register char *ip = ibuf, *iend = ibuf + sizeof ibuf;
	int c;

	if (op == obuf)
	{
		for (ip = ibuf; ip < iend; ip++)
		{
			if ((c = Get6Bits()) == EOF)
			{
				if (ip <= &ibuf[1])
					return EOF;
				else if (ip == &ibuf[2])
					eof = 1;
				else
					eof = 2;
			}
			else if(c < 0)
				return c;
			*ip = c;
		}
		obuf[0] = (ibuf[0] << 2 | ibuf[1] >> 4);
		obuf[1] = (ibuf[1] << 4 | ibuf[2] >> 2);
		obuf[2] = (ibuf[2] << 6 | ibuf[3]);
	}
	if ((eof) & (op >= &obuf[eof]))
		return EOF;
	c = *op++;
	if (op >= oend)
		op = obuf;
	return (c & BYTEMASK);
}

char tr[] = "!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";

/* q format -- decode one byte into 6 bit binary */
int ZHexBin::Get6Bits ()
{
	register int c;
	register char *where;
	int n;

	if(m_pBuffer == NULL)
	{
		m_pBuffer = m_pArtSpool->GetLine();
		if(strncmp(m_pBuffer, "--- end of part", 15) == 0)
		{
			m_pBuffer = NULL;
			return EOF;
		}
	}

	while (1)
	{
		if(*m_pBuffer == '\0')
		{
			if((m_pBuffer = m_pArtSpool->GetLine()) == NULL)
			{
				// EOF
				if(m_nCurPart < m_nTotalPart)
				{
					m_nCurPart++;
					n = OpenInFile();
					if(n < 0)
						return n;
					m_pBuffer = "";
					continue;
				}
				return EOF;
			}
			if(*m_pBuffer == '\0')
				continue;
			if(strncmp(m_pBuffer, "--- end of part", 15) == 0)
			{
				if(m_nCurPart < m_nTotalPart)
				{
					m_nCurPart++;
					n = OpenInFile();
					if(n < 0)
						return n;
					m_pBuffer = "";
					continue;
				}
				return EOF;
			}
		}
		c = *m_pBuffer++;
		switch(c)
		{
		case '\n':
		case '\r':
			continue;

		case ':':
			return EOF;
		}

		where = tr;
		while (*where != '\0' && *where != c)
			where++;
		if (*where == c)
			return (where - tr);
		else
		{
			m_pBuffer = "";
			return EOF;
		}
	}
}

/*
int ZHexBin::Get6Bits ()
{
	register int c;
	register char *where;
	int n;

	streampos pos;
	while (1)
	{
		c = m_pDecodeFile->get();
		switch (c)
		{
		case '\n':
		case '\r':
			continue;
		case EOF:
			if(m_nCurPart < m_nTotalPart)
			{
				m_nCurPart++;
				n = OpenInFile();
				if(n < 0)
					return n;
				continue;
			}
		case ':':
			return EOF;
		default:
			where = tr;
			while (*where != '\0' && *where != c)
				where++;
			if (*where == c)
				return (where - tr);
			else
			{
				fprintf(stderr, "bad char\n");
				return EOF;
			}
		}
	}
}
*/

int ZHexBin::OpenInFile ()
{
	// Close the old file
	if(m_nCurPart > 2)
	{
		delete m_pArtSpool;
		m_pArtSpool = NULL;
	}
	ZString FileName;
	while(m_nCurPart <= m_nTotalPart)
	{
		if(!m_pArtNums[m_nCurPart].length())
		{
			m_nCurPart++;
			continue;
		}

		m_pArtSpool = new ArticleSpool(m_pNewsName,
				m_pArtNums[m_nCurPart].chars());
		if(!m_pArtSpool && !m_pArtSpool->good())
			return HB_ERR_PART;
		m_pArtSpool->NoFile();

		char * p;
		while((p = m_pArtSpool->GetLine()))
		{
			if (strcmp(p, "---") == 0)
				break;
		}
		return 0;
	}
	return EOF;
}

#define CRCCONSTANT 0x1021

void ZHexBin::CompQCRC(register unsigned int c)
{
	register int i;
	register unsigned long temp = m_uCRC;

	for (i = 0; i < 8; i++)
	{
		c <<= 1;
		if ((temp <<= 1) & WORDBIT)
			temp = (temp & WORDMASK) ^ CRCCONSTANT;
		temp ^= (c >> 8);
		c &= BYTEMASK;
	}
	m_uCRC = temp;
}

// later... need to fix return when support old format
long ZHexBin::MakeFile (char * fname)
{
	register long nbytes = 0;

	if (!m_bMacBinary)
	{
		CloseOutFile();
		m_pOutFile = fopen(fname, "w");
		if (m_pOutFile == NULL)
		{
			perror(fname);
			exit(-1);
		}
	}
	char * p;
	while((p = m_pArtSpool->GetLine()))
	{
		if (strncmp(p, "***END", 6) == 0)
			break;
		if (m_nCompressed)
			nbytes += Comp2Bin(p, m_pOutFile);
		else
			nbytes += Hex2Bin(p, m_pOutFile);
	}
	if (!m_bMacBinary)
	{
		CloseOutFile();
	}
	return nbytes;
}

#define SIXB(c) (((c)-0x20) & 0x3f)

int ZHexBin::Comp2Bin (char * ibuf, FILE * outf)
{
	char obuf[BUFSIZ];
	register char *ip = ibuf;
	register char *op = obuf;
	register int n, outcount;
	int numread, incount;

	numread = strlen(ibuf);
	ip[numread - 1] = ' ';					/* zap out the newline */
	outcount = (SIXB(ip[0]) << 2) | (SIXB(ip[1]) >> 4);
	incount = ((outcount / 3) + 1) * 4;
	for (n = numread; n < incount; n++)		/* restore lost spaces */
		ibuf[n] = ' ';

	n = 0;
	while (n <= outcount)
	{
		*op++ = SIXB(ip[0]) << 2 | SIXB(ip[1]) >> 4;
		*op++ = SIXB(ip[1]) << 4 | SIXB(ip[2]) >> 2;
		*op++ = SIXB(ip[2]) << 6 | SIXB(ip[3]);
		ip += 4;
		n += 3;
	}

	for (n = 1; n <= outcount; n++)
		CompCCRC(obuf[n]);

	fwrite(obuf + 1, 1, outcount, outf);
	return outcount;
}

int ZHexBin::Hex2Bin (char * ibuf, FILE * outf)
{
	register char *ip = ibuf;
	register int n, outcount;
	int c;

	n = strlen(ibuf) - 1;
	outcount = n / 2;
	for (n = 0; n < outcount; n++)
	{
		c = HexIt(*ip++);
		CompECRC(c = (c << 4) | HexIt(*ip++));
		fputc(c, outf);
	}
	return outcount;
}

int ZHexBin::HexIt (char c)
{
	if ('0' <= c && c <= '9')
		return c - '0';
	if ('A' <= c && c <= 'F')
		return c - 'A' + 10;

	fprintf(stderr, "illegal hex digit: %c", c);
	exit(4);
	/* NOTREACHED */
}

void ZHexBin::CompCCRC (unsigned char c)
{
	m_uCRC = (m_uCRC + c) & WORDMASK;
	m_uCRC = ((m_uCRC << 3) & WORDMASK) | (m_uCRC >> 13);
}

void ZHexBin::CompECRC(unsigned char c)
{
	m_uCRC += c;
}

void ZHexBin::Put2 (char * bp, short value)
{
	*bp++ = (value >> 8) & BYTEMASK;
	*bp++ = value & BYTEMASK;
}

void ZHexBin::Put4 (char * bp, long value)
{
	register int i, c;

	for (i = 0; i < 4; i++)
	{
		c = (value >> 24) & BYTEMASK;
		value <<= 8;
		*bp++ = c;
	}
}
