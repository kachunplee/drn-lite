#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "def.h"
#include "szptr.h"
#include "advert.h"
#include "userinfo.h"
#include "tmplerr.h"
#include "zstg.h"
#include "zcgi.h"
#include "status.h"
#include "cnntp.h"

extern const char * NNTPservers;
extern const char * NNTPauths;
extern const char szVersion[];
extern const char szBuild[];

UserInfo gUserInfo;

static void SendBody(ZString&, CNNTP&, FILE*);
static void SendAttach(FILE * fpAttach, int nAttSplitSize, int nAttLineSize,
		CNNTP & nntp, FILE * fp);
static void SendPkt(const char *, int, CNNTP&, FILE*);

const char stgCRLF [] = "\r\n";
const int MAXLINE = 1024;
static char buf[MAXLINE+100];

const int UUE_LINE_BYTE = 45;

main(int argc, char *argv[])
{
	NewsOption options;

	Status status(&cout);

	TemplateError tmplerr;
	ZString sErr;

	cout << "Content-Type: text/html" << endl << endl;

	if(argc > 1)
	{
		if(strcmp(argv[1], "-v") == 0)
		{
			cout << DEF_BODY_TAG << endl
				 << "<h3>DRN " << szVersion << szBuild
				 << " (c) 1995-1999 Pathlink Technology</h3>" << endl;
			return(1);
		}

		while(argc > 1 && strcmp(argv[1], "-D") == 0)
		{
			++debug;
			--argc;
			++argv;
		}
	}

	if (strcasecmp(getenv("REQUEST_METHOD"), "POST"))
	{
		tmplerr.OutError(cout, "<h2>Post news internal error</h2>");
		return(1);
	}

	if(!gUserInfo.Init(cout, getenv("REMOTE_USER"),
		getenv("REMOTE_HOST"), getenv("REMOTE_ADDR")))
	{
		sErr = "<h3>Please contact your ISP for DRN access information from this host: ";
		sErr += gUserInfo.GetHostName();
		sErr += "</h3>";
		tmplerr.OutError(cout, sErr.chars());
		return(1);
	}

	options.ReadPreference();

	const char * p;
	p = getenv("CONTENT_LENGTH");
	if(p)
		status("Receiving post (%s bytes)...", p);

	ZCGI cgi;
	if(!cgi.ReadInput())
	{
		tmplerr.OutError(cout, "<h2>Missing posting input</h2>");
		return(1);
	}

	sErr = "";
	ZString Subject = cgi.GetParam("Subject");
	if(Subject.length() == 0)
		sErr += "<h2>[Subject] field is empty</h2>\n";

	ZString Newsgrps = cgi.GetParam("Newsgrps");
	if(Newsgrps.length() == 0)
		sErr += "<h2>[Newsgroups] field is empty</h2>\n";

	ZString From = cgi.GetParam("From");
	if(From.length() == 0)
		sErr = sErr + "<h2>[From] field is empty</h2>\n";

	ZString Body = cgi.GetParam("Body");
	ZString Filename = cgi.GetParam("Filename");
	if(Body.length() == 0 && Filename.length() == 0)
		sErr += "<h2>[Body] field is empty</h2>\n";

	if(From.find('@') == string::npos)
	{
		// cannot find the '@' char, append mail address to first word of From
		strncpy(buf, From.chars(), MAXLINE);
		buf[MAXLINE] = 0;
		char * p = strchr(buf, ' ');
		if(p && p > buf)
			*p = 0;
		From += " <";
		From += buf;
		From += "_member@";
		From += options.GetMailDomain();
		From += ">";
	}

	BOOL bCcMail = FALSE;
	ZString ReplyTo = cgi.GetParam("ReplyTo");
	ZString ReplyFrom = cgi.GetParam("ReplyFrom");
	if(ReplyTo.length() > 0)
	{
		bCcMail = TRUE;
		if(ReplyFrom.length() == 0)
			ReplyFrom = ReplyTo;
	}

	ZString Signature = cgi.GetParam("Signature");
	{
		// Make sure signature doesn't contain more than five 80 chars lines
		BOOL bLong = FALSE;
		int nSig = 0;
		unsigned int m, n;
		m = Signature.rfind(stgCRLF);
		if(m == (Signature.length() - 2))
			Signature.del(m, 2);		// delete the ending newline
		n = 0;
		while((m = Signature.find(stgCRLF, n)) != string::npos)
		{
			nSig++;
			if((m-n) > 80)
			{
				bLong = TRUE;
			}
			n = m + 2;
		}
		if(n < Signature.length())
			nSig++;

		if(nSig > 5)
			sErr += "<h2>[Signature] field contains more than 5 lines</h2>\n";
		if(bLong)
			sErr += "<h2>[Signature] field contains line(s) with more than 80 characters</h2>\n";
		if(Signature.rfind(stgCRLF, 0) != (Signature.length() - 2))
		{
			Signature += stgCRLF;
		}
	}

	if(sErr.length() > 0)
	{
		sErr += "<p>Please use the <b>BACK</b> button of your browser to go back to make the changes";
		if(Filename.length() > 0)
			sErr += " and verify the file attachment still appears in the Attachment Box";
		sErr += ".<br>\n";
		tmplerr.OutError(cout, sErr.chars());
		return(1);
	}

	ZString TmplName = DRNTMPLDIR;
	TmplName += "/post.htm";
	Zifstream tmplFile(TmplName, ios::nocreate|ios::in);
	if(!tmplFile)
	{
		ZString err = "<h3>Template file " + TmplName;
		err += " is not found</h3>";
		tmplerr.OutError(cout, err.chars());
		return(1);
	}

	if(!tmplFile.good())
	{
		ZString err = "<h3>Template file " + TmplName;
		err += " is empty</h3>";
		tmplerr.OutError(cout, err.chars());
		return(1);
	}

	// Scan for <!--pathlink drn=post -->
	BOOL bDspPost = FALSE;
	do
	{
		p = tmplFile.GetLine();

		if(strncmp(p, "<!--pathlink drn=post -->", 25) == 0)
		{
			bDspPost = TRUE;
			break;
		}

		cout << p << endl;
	} while(tmplFile.NextLine());

	if(bDspPost)
	{
		FILE * fpAttach = NULL;
		int nAttLineSize = 0;
		int nAttSplitSize = 0;
		int nTotal = 1;
		int nDigit = 1;

		if(Filename.length())
		{
			int nAttach = cgi.GetNParam("Filename");
			ZString AttachTmp = cgi.GetFParam("Filename");
			if(nAttach > 0 && AttachTmp.length())
			{
				fpAttach = fopen(AttachTmp, "r");
				if(fpAttach)
				{
					p = cgi.GetParam("AttSplitSize");
					nAttSplitSize = (p) ? atoi(p) : options.GetAttSplitSize();
					if(nAttSplitSize < options.GetMinSplitSize())
						nAttSplitSize = options.GetMinSplitSize();
					if(nAttSplitSize > options.GetMaxSplitSize())
						nAttSplitSize = options.GetMaxSplitSize();

					nAttLineSize = (nAttach+UUE_LINE_BYTE-1)/UUE_LINE_BYTE;
					nTotal = (nAttLineSize+nAttSplitSize-1)/nAttSplitSize;
					for(int i = nTotal; i > 9; i /= 10) nDigit++;

					unsigned idx = Filename.rfind('\\');
					if(idx != string::npos)
						Filename.del(0, idx+1);
					else
					{
						idx = Filename.rfind('/');
						if(idx != string::npos)
							Filename.del(0, idx+1);
					}
				}
			}
		}

		cout << "<h3>Posting - Please Wait...</h3><p>" << endl;

		pid_t pid = getpid();

		try
		{
			int nl;
			CNNTP nntp(NNTPservers, NNTPauths);

			for(int nPart = 1; nPart <= nTotal; ++nPart)
			{
				FILE * fp = NULL;
				BOOL bPipe = FALSE;

				status("Posting %d/%d...", nPart, nTotal);

				if(bCcMail)
				{
					sprintf(buf, "/usr/sbin/sendmail -t -f'%s'",
						ReplyFrom.chars());
					if((fp = popen(buf, "w")) == NULL)
					{
						cout << "<h3>Your reply email could not be sent. "
							<< "Please contact the tech support to report "
							<< "the problem [" << getpid() << "]</h3>"
							<< endl;

						// try to open a temp file to save the email
						sprintf(buf, "/tmp/drn%d", pid);
						fp = fopen(buf, "w");
					}
					else
						bPipe = TRUE;
				}

				p = nntp.SendCmd("POST\r\n", "340 ");
				DMSG(1, "%s", p);

				//
				// Newsgroup header
				//
				nl = snprintf(buf, MAXLINE, "Newsgroups: %s\r\n",
						Newsgrps.chars());
				DMSG(1, "%s", buf);
				nntp.Send(buf, nl);

				//
				// From header
				//
				nl = snprintf(buf, MAXLINE, "From: %s\r\n", From.chars());
				DMSG(1, "%s", buf);
				nntp.Send(buf, nl);

				//
				// Content-Type: text/html
				//
				if((p = cgi.GetParam("TypeHTML")) != NULL && *p)
				{
					const char * pType = "MIME-Version: 1.0\r\nContent-Type: text/html\r\n";
					DMSG(1, "%s", pType);
					nntp.Send(pType, strlen(pType));
				}

				//
				// Path header
				//
				p = cgi.GetParam("Path");
				nl = snprintf(buf, MAXLINE, "Path: %s\r\n",
					(p == NULL || *p == 0) ? DEF_PATH : p);
				DMSG(1, "%s", buf);
				nntp.Send(buf, nl);

				//
				// Sender header
				//
				const char * pServer = DRN_SERVER;
				if(strncmp(pServer, "http://", 7) == 0)
					pServer += 7;
				nl = snprintf(buf, MAXLINE, "Sender: usenet@%s\r\n", pServer);
				DMSG(1, "%s", buf);
				nntp.Send(buf, nl);

				//
				// Newsreader header
				//
				nl = snprintf(buf, MAXLINE,
					"X-Newsreader: Direct Read News %s\r\n",
					szVersion);
				DMSG(1, "%s", buf);
				nntp.Send(buf, nl);

				//
				// X-No-Archive header
				//
				if((p = cgi.GetParam("Archive")) != NULL && *p)
				{
					const char stgArc[] = "X-No-Archive: Yes\r\n";
					DMSG(1, "%s", stgArc);
					nntp.Send(stgArc, sizeof(stgArc)-1);
				}

				//
				// Organization header
				//
				if((p = cgi.GetParam("Org")) != NULL && *p)
				{
					nl = snprintf(buf, MAXLINE, "Organization: %s\r\n", p);
					DMSG(1, "%s", buf);
					nntp.Send(buf, nl);
				}

				//
				// Reference header
				//
				if((p = cgi.GetParam("Refer")) != NULL && *p)
				{
					nl = snprintf(buf, MAXLINE, "References: %s\r\n", p);
					DMSG(1, "%s", buf);
					nntp.Send(buf, nl);
				}

				//
				// Send mail headers
				//
				if(fp)
				{
					fprintf(fp, "To: %s\r\n", ReplyTo.chars());
					fprintf(fp, "From: %s\r\n", From.chars());
				}

				//
				// Subject header
				//
				nl = snprintf(buf, MAXLINE, "Subject: %s", Subject.chars());
				if(fpAttach)
				{
					//
					// See if subject has file name
					//
					ZString lsubj = Subject;
					lsubj.downcase();
					ZString lfile = Filename;
					lfile.downcase();
					if(lsubj.find(lfile) == string::npos)
					{
						//
						// No - add one
						//
						nl += snprintf(buf+nl, MAXLINE-nl, " - %s",
							Filename.c_str());
					}
					if(nTotal > 1)
						nl += snprintf(buf+nl, MAXLINE-nl,
							 " [%0*d/%0d]", nDigit, nPart, nTotal);
				}
				nl += snprintf(buf+nl, MAXLINE-nl, "\r\n");
				SendPkt(buf, nl, nntp, fp);

				//
				// Finished Header - set blank line
				//
				SendPkt(stgCRLF, sizeof(stgCRLF)-1, nntp, fp);

				//
				// If Part 1 - send Text Body and Attachment
				//
				if(nPart == 1)
				{
					//
					// Body
					//
					SendBody(Body, nntp, fp);

					//
					// Signature?
					//
					if(Signature.length() > 0)
					{
						SendPkt(stgCRLF, sizeof(stgCRLF)-1, nntp, fp);
						SendPkt(Signature.chars(), Signature.length(), nntp, fp);
					}

					//
					// Attachment?
					//
					if(fpAttach)
					{
						//
						// Form begin header
						//
						nl = snprintf(buf, MAXLINE, "begin 0644 %s\r\n",
							Filename.c_str());
						//
						// Send begin header
						//
						SendPkt(buf, nl, nntp, fp);
					}
				}

				//
				// Send attachment up to nAttSplitSize
				//
				if(fpAttach)
					SendAttach(fpAttach, nAttSplitSize, nAttLineSize, nntp, fp);

				p = nntp.SendCmd(".\r\n", "240 ");
				DMSG(1, "%s", p);

				if(fp)
				{
					if(bPipe)
						pclose(fp);
					else
						fclose(fp);
				}

				//
				// Next piece
				//
				if((nAttLineSize -= nAttSplitSize) <= 0)
					break; // just in case 'npart' is all screw up
			}
		}
		catch(const char * p)
		{
			// 502 Authentication error
			//
			// 441 Article has no body -- just headers
			//
			cout << "<h2>" << p << "</h2>" << endl;
		}


		if(fpAttach) fclose(fpAttach);
		cout << "<h2>Article Posted</h2>" << endl
		 	<< "<p>" << DRN_NOTICE << "</p>" << endl;
	}

	while(tmplFile.NextLine())
	{
		p = tmplFile.GetLine();
		cout << p << endl;
	}
	return(0);
}

static void
SendBody (ZString &Body, CNNTP & nntp, FILE * fp)
{
	unsigned int n;
	ZString stgLine;
	const char * q;
	const char * r;
	while(Body.length() > 0)
	{
		if((n = Body.find(stgCRLF)) == string::npos)
		{
			stgLine = Body;
			Body = "";
		}
		else
		{
			if(n == 0)
			{
				// got a blank line, 
				Body.del(0, 2);
				SendPkt(stgCRLF, sizeof(stgCRLF)-1, nntp, fp);
				continue;
			}
			stgLine = Body.substr(0, n);
			Body.del(0, n+2);
		}

		n = 0;
		q = stgLine.chars();
		while(q && *q)
		{
			r = q;
			while(isspace(*r)) r++;		// skip all the blanks

			if(n == 0)
				q = r;		// beginning of line, throw away blanks

			if(*q == '>' || (n + strlen(q)) <= 80)
			{
				// a reply line or total chars left + chars
				//   in buffer <= 80 chars, so send it and 
				//   done with current line
				strcpy(buf+n, q);
				n += strlen(q);
				strcpy(buf+n, stgCRLF);
				SendPkt(buf, strlen(buf), nntp, fp);
				break;
			}

			r = strchr(r, ' ');
			if(r)
			{
				// find the next blank
				if((n + (r - q)) > 80)
				{
					// adding next word will pass 80 chars, so
					// 	send the line first and start a new line
					strcpy(buf+n, stgCRLF);
					SendPkt(buf, strlen(buf), nntp, fp);
					n = 0;
					continue;
				}

				// add next word to current line
				strncpy(buf+n, q, r-q); 
				n += (r - q);
				q = r;
				continue;
			}

			// last word
			if(n)
			{
				// a previous line exists, send it
				strcpy(buf+n, stgCRLF);
				SendPkt(buf, strlen(buf), nntp, fp);
			}
			while(*q && isspace(*q)) q++;
			strncpy(buf, q, MAXLINE);
			buf[MAXLINE] = 0;
			n = strlen(buf);
			strcpy(buf+n, stgCRLF);
			SendPkt(buf, n+sizeof(stgCRLF)-1, nntp, fp);
			break;
		}
	}
}

extern int uue_byte(int);
extern void uuencode(const char *, char *);

void
SendAttach (FILE * fpAttach, int nAttSplitSize, int nAttLineSize,
		CNNTP & nntp, FILE * fp)
{
	int nl;
	char line[UUE_LINE_BYTE];
	for(int i = (nAttSplitSize<nAttLineSize)?
					nAttSplitSize:nAttLineSize; i > 0; i--)
	{
		if((nl = fread(line, 1, UUE_LINE_BYTE, fpAttach))>0)
		{
			char * out = buf;
			*out++ = uue_byte(nl);
			for(int n = 0; n < nl; n += 3)
			{
				uuencode(line+n, out);
				out += 4;
			}
			strcpy(out, stgCRLF);
			SendPkt(buf, out-buf+sizeof(stgCRLF)-1, nntp, fp);
		}
	}

	if(nAttLineSize <= nAttSplitSize)
	{
		nl = sprintf(buf, "%c\nend\n", uue_byte(0));
		SendPkt(buf, nl, nntp, fp);
	}
}

void
SendPkt (const char * p, int n, CNNTP& nntp, FILE* fp)
{
	DMSG(1, "%s", p);
	nntp.Send(p, n);
	if(fp) fwrite(p, 1, n, fp);
}
