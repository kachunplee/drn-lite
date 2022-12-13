int
x1c (int c)
{
	if(c >= 'a' && c <= 'z') return c-'a'+10;
	if(c >= 'A' && c <= 'Z') return c-'A'+10;
	return c-'0';
}

int
x2c (const char * q)
{
	return (x1c(q[0])<<4) + x1c(q[1]);
}

int
uue_byte (int b)
{
	b &= 0x3f;
	return (b==0)?'`':b+' ';
}

//
// From[3] -> to[4]
//
void
uuencode (const char * from, char * to)
{
	to[0] = uue_byte(from[0] >> 2);
	to[1] = uue_byte(((from[0] << 4)&0x30) | ((from[1] >> 4)&0xf));
 	to[2] = uue_byte(((from[1] << 2)&0x3c) | ((from[2] >> 6)&0x3));
	to[3] = uue_byte(from[2]);
}
