#include <Windows.h>

bool XOR(const bool p,const bool q)
{
	return ((p || q) && !(p && q));
}

int soundOf7_11Door()
{
	Beep(860,350); // This parameter must be in the range 37 through 32,767.
	Beep(690,700);
	return 0;
}