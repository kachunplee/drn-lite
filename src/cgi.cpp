/************************************************************************
Copyright Notice

Copyright (c) MCNC, Clearinghouse for Networked Information Discovery and
Retrieval, 1994. 

Permission to use, copy, modify, distribute, and sell this software and
its documentation, in whole or in part, for any purpose is hereby granted
without fee, provided that

1. The above copyright notice and this permission notice appear in all
copies of the software and related documentation. Notices of copyright
and/or attribution which appear at the beginning of any file included in
this distribution must remain intact. 

2. Users of this software agree to make their best efforts (a) to return
to MCNC any improvements or extensions that they make, so that these may
be included in future releases; and (b) to inform MCNC/CNIDR of noteworthy
uses of this software. 

3. The names of MCNC and Clearinghouse for Networked Information Discovery
and Retrieval may not be used in any advertising or publicity relating to
the software without the specific, prior written permission of MCNC/CNIDR. 

THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY WARRANTY
OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. 

IN NO EVENT SHALL MCNC/CNIDR BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF THE
POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgi.h"
#include "httpbox.h"

#define NEWSTRUCT(a) (a*)calloc(1,sizeof(a))
#define FREE(a) free(a)

PCGI cgi_Create()
{
        PCGI cgi;
 
        if((cgi=NEWSTRUCT(CGI))==NULL)
                return NULL;
 
        return cgi;
}
 
void cgi_Destroy(PCGI cgi)
{
        if(cgi)
                FREE(cgi);
}
 
int cgi_GetInput(PCGI cgi)
{
	int ContentLen,x;
	PCHR P;
	PENTRY Entries;
	PCHR Method;

	if(!cgi)
		return(0);
	
	if((Method = (char *)getenv("REQUEST_METHOD"))==NULL) {
		printf("Unable to get request_method\n");
		return(0);
	}
	if(strcasecmp((char *)getenv("REQUEST_METHOD"), "POST")) {
		printf("This script to be referenced with a METHOD of POST.\n");
		printf ("If you don't understand this, see this ");
		printf ("<A HREF=\"http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/Docs/fill-out-forms/overview.html\">forms overview</A>.%c", 10);
		return(0);
        }

	Entries = cgi->Entries;

	if((P=(char *)getenv("CONTENT_LENGTH"))) 
		ContentLen = atoi(P);
 
	/* Build the list of cgi entries */
	for (x = 0; ContentLen && (!feof (stdin)); x++) {
        	cgi->EntryCount = x+1;
        	Entries[x].val = fmakeword(stdin,'&',&ContentLen);
        	plustospace (Entries[x].val);
        	unescape_url (Entries[x].val);
        	Entries[x].name = makeword (Entries[x].val, '=');
	}
	return(1);
}

PCHR cgi_GetName(PCGI cgi, INT4 i)
{
	return cgi->Entries[i].name;
}

PCHR cgi_GetValue(PCGI cgi, INT4 i)
{
	return cgi->Entries[i].val;
}

PCHR cgi_GetValueByName(PCGI cgi, PCHR Name)
{
	INT i;

	if((cgi==NULL)||(Name==NULL)||(Name[0]=='\0'))
		return NULL;
	for(i=0;i<=cgi->EntryCount;i++) {
		if((cgi->Entries[i].name==NULL)||
			(cgi->Entries[i].name[0]=='\0'))
			return NULL;
		if(!strcasecmp(cgi->Entries[i].name, Name)) {
			if(cgi->Entries[i].val!='\0')
				return cgi->Entries[i].val;
			return NULL;
		}
	}
	return NULL;
}
