#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>
#include <string.h>
#include <string>

#include "def.h"

#include "tmplerr.h"

void TemplateError::OutContentType (ostream& stm)
{
	stm << "Content-Type: text/html" << endl << endl;
}

void TemplateError::OutError (ostream& stm, const char * pError)
{
	ZString Filename = DRNTMPLDIR;
	Filename += "/error.htm";
	Zifstream errFile(Filename, ios::nocreate|ios::in);
	if(!errFile)
	{
		stm
			<< DEF_BODY_TAG << endl
			<< "<h3>Template file " << Filename << " is not found</h3>" << endl
			<< "<p>" << DRN_NOTICE << "</p>" << endl;
		return;
	}

	if(!errFile.good())
	{
		stm
			<< DEF_BODY_TAG << endl
			<< "<h3>Template file " << Filename << " is empty</h3>" << endl
			<< "<p>" << DRN_NOTICE << "</p>" << endl;
		return;
	}

	// Scan for <!--pathlink drn=error -->
	char * p;
	do
	{
		p = errFile.GetLine();

		static const char token [] = "<!--pathlink drn=error -->";
		if(strncmp(p, token, sizeof(token)-1) == 0)
			break;

		stm << p;
	} while(errFile.NextLine());

	stm << pError << endl
		<< "<p>" << DRN_NOTICE << "</p>" << endl;

	while(errFile.NextLine())
	{
		p = errFile.GetLine();
		stm << p;
	}
}
