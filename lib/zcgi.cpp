#include <stdio.h>
#include <unistd.h>
#include <string>

#include "dmsg.h"
#include "zcgi.h"

const char form_data [] = "multipart/form-data; boundary=";
const char Content_Disp [] = "Content-Disposition: form-data; name=";
const char Filename_eq [] = "filename=";

extern const char * TMPDIR;

ZCGI::ZCGI ()
{
}

ZCGI::~ZCGI ()
{
	if(debug == 0)
	{
		ParamMap::iterator iter;
		for(iter = m_fparam.begin(); iter != m_fparam.end(); iter++)
			unlink((*iter).second.c_str());
	}
}

const char * 
ZCGI::GetParam (const char * p)
{
	const ParamMap::iterator iter = m_param.find(p);
	if(iter == m_param.end())
		return NULL;

	return (*iter).second.c_str();
}

const char * 
ZCGI::GetFParam (const char * p)
{
	const ParamMap::iterator iter = m_fparam.find(p);
	if(iter == m_fparam.end())
		return NULL;

	return (*iter).second.c_str();
}

int
ZCGI::GetNParam (const char * p)
{
	const map<string, int>::iterator iter = m_nparam.find(p);
	if(iter == m_nparam.end())
		return 0;

	return (*iter).second;
}

bool
ZCGI::ReadInput ()
{
	char * p = getenv("REQUEST_METHOD");
	if(p == NULL)
	{
		DMSG(0, "Cannot get REQUEST_METHOD");
		return false;
	}

	if(strcasecmp(p, "POST") != 0)
	{
		DMSG(0, "REQUEST_METHOD not POST");
		return false;
	}

	p = getenv("CONTENT_LENGTH");
	if(p == NULL)
	{
		DMSG(0, "Cannot get CONTENT_LENGTH");
		return false;
	}
	int nContent = atoi(p);

	p = getenv("CONTENT_TYPE");
	return (p && strncmp(p, form_data, sizeof(form_data)-1) == 0)
		? ReadFormData(nContent, p + sizeof(form_data)-1)
		: ReadURLEncoded(nContent);
}

extern int x2c(const char * q);

bool
ZCGI::ReadURLEncoded (int nContent)
{
	char buf[nContent+1];
	if(nContent != read(0, buf, nContent))
	{
		DMSG(0, "Read Error");
		return false;
	}

	buf[nContent] = 0;

	char *p, *q, *r, *s;

	for(p = buf; *p; p = s)
	{
		q = strchr(p, '=');
		if(q == NULL)
			return false;

		*q++ = 0;
		for(s = r = p; *s; s++)
		{
			*r++ = (*s == '%') ? x2c((++s)++) : ((*s=='+')?' ':*s);
		}
		*r = 0;

		for(s = r = q; *s && *s != '&'; s++)
		{
			*r++ = (*s == '%') ? x2c((++s)++) : ((*s=='+')?' ':*s);
		}
		int c = *s;
		*r = 0;

		DMSG(1, "%s <- %s", p, q);
		m_param[p] = q;
		if(c == '&') s++;
	}

	return true;
}

bool
ZCGI::ReadFormData (int nContent, char * pBoundary)
{
	enum { FORM_BEGIN, FORM_HEADER, FORM_STRING, FORM_FILE }
		tSect = FORM_STRING, nSect = FORM_BEGIN;

	const int MAX_LINE = 1024;
	char buf[MAX_LINE+2+1];
	int nBoundary = strlen(pBoundary);
	char * line = buf+2;
	const char *p = line;
	const char *q;

	buf[0] = '\r';
	buf[1] = '\n';

	DMSG(1, "CONTENT_LENGTH = %d, BOUNDARY = (%d)%s",
		nContent, nBoundary, pBoundary);
	string * pName = NULL;
	int nf = 0;
	FILE * of = NULL;
	size_t fsize = 0;
	char filename[MAX_LINE];
	int nl;
	for(int nGet = 0;
		(nGet < nContent) && ((nl=GetLine(line, MAX_LINE)) > 0); )
	{
		if(nGet+nl > nContent)
			nl = nContent - nGet;
		nGet += nl;

		if(nSect == FORM_HEADER)
		{
			//
			// Header Section
			//
			if(line[0] == '\r' || line[0] == '\n')
			{
				//
				// End of Header
				//
				nSect = tSect;
				p = line;
				continue;
			}

			if(strncmp(line, Content_Disp, sizeof(Content_Disp)-1) == 0)
			{
				p = line + sizeof(Content_Disp) - 1;
				if(*p == '"')
				{
					q = strchr(++p, '"');
					pName = q ? new string(p, q-p) : new string(p);
					DMSG(1, "Name = %s", pName->c_str());
					q = strchr(q, ';');
					if(q)
					{
						while(*++q) if(!isspace(*q)) break;
						if(strncmp(q, Filename_eq, sizeof(Filename_eq)-1) == 0)
						{
							p = q + sizeof(Filename_eq) - 1;
							if(*p == '"')
							{
								q = strchr(++p, '"');
								m_param[pName->c_str()] = q ? string(p, q-p) : string(p);
								DMSG(1, "Filename = %s", m_param[pName->c_str()].c_str());
							}
							tSect = FORM_FILE;
						}
					}
				}
			}
		}

		//
		// Other Sections
		//
		// Check if boundary
		//
		if(nl > nBoundary)
		{
			char c = line[2+nBoundary];
			if(line[0] == '-' && line[1] == '-'
				&& (c == '-' || c == '\r' || c == '\n')
				&& strncmp(line+2, pBoundary, nBoundary) == 0)
			{
				if(strncmp(line+2+nBoundary, "--", 2) == 0)
				{
					DMSG(1, "End of multipart");
					return true;
				}

				//
				// Found boundary
				//
				if(of)
				{
					m_nparam[pName->c_str()] = fsize;
					fclose(of);
					of = NULL;
					fsize = 0;
				}
				delete pName;
				pName = NULL;

				//
				// Headers
				//
				nSect = FORM_HEADER;
				tSect = FORM_STRING;
				continue;
			}
		}

		if(nSect == FORM_BEGIN)
			continue;

		q = line;
		if(line[nl-1] == '\n') --q, --nl;
		if(line[nl-1] == '\r') --q, --nl;
		line[nl] = 0;

		switch(nSect)
		{
		case FORM_STRING:
			if(*p) m_param[pName->c_str()] += p;
			break;

		case FORM_FILE:
			nl += line-p;
			if(nl > 0)
			{
				if(of == NULL)
				{
					if(MAX_LINE <
						snprintf(filename, MAX_LINE,
							"%s/drnp%d.%d", TMPDIR, getpid(), nf++))
					{
						DMSG(0, "Tmp file name too long");
						return false;
					}

					of = fopen(filename, "w");
					if(of == NULL)
					{
						DMSG(0, "Cannot create file %s", filename);
						return false;
					}

					m_fparam[pName->c_str()] = filename;
				}

				if(fwrite(p, 1, nl, of) != (size_t) nl)
				{
					DMSG(0, "Write error - %s", filename);
					return false;
				}
				fsize += nl;
			}

			break;

		default:
			break;
		}

		p = q;
	}

	return true;
}

int
ZCGI::GetLine (char * line, int nMax)
{
	int nl;
	for(nl = 0; (nl < nMax) && (fread(line+nl, 1, 1, stdin) == 1); nl++)
	{
		if(line[nl] == '\n')
		{
			++nl;
			break;
		}
	}

	line[nl] = 0;
	return nl;
}

void
ZCGI::DumpParam ()
{
	ParamMap::iterator iter;
	DMSG(0, "\nDump m_param:");
	for(iter = m_param.begin(); iter != m_param.end(); iter++)
	{
		DMSG(0, "%s = %s", (*iter).first.c_str(), (*iter).second.c_str());
	}
	DMSG(0, "\nDump m_faram:");
	for(iter = m_fparam.begin(); iter != m_fparam.end(); iter++)
	{
		DMSG(0, "%s = %s", (*iter).first.c_str(), (*iter).second.c_str());
	}
	DMSG(0, ".");
}
