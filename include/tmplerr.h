#ifndef	__TMPLERR_H__
#define	__TMPLERR_H__

class TemplateError
{
public:
	TemplateError ()								{}

	void OutContentType(ostream&);
	void OutError(ostream&, const char *);
};

#endif //__TMPLERR_H__
