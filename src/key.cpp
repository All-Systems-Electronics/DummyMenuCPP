// Credit: https://rosettacode.org/wiki/Keyboard_input/Keypress_check#C

#include "key.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

static void set_mode(int want_key)
{
	static struct termios oldData, newData;
	if (!want_key) {
		tcsetattr(STDIN_FILENO, TCSANOW, &oldData);
		return;
	}
 
	tcgetattr(STDIN_FILENO, &oldData);
	newData = oldData;
	newData.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newData);
}
 
static int get_key()
{
	int c = 0;
	struct timeval tv;
	fd_set fs;
	tv.tv_usec = tv.tv_sec = 0;
 
	FD_ZERO(&fs);
	FD_SET(STDIN_FILENO, &fs);
	select(STDIN_FILENO + 1, &fs, 0, 0, &tv);
 
	if (FD_ISSET(STDIN_FILENO, &fs)) {
		c = getchar();
	}
	return c;
}

void KeyStartScanMode()
{
	set_mode(1);
}

void KeyStopScanMode()
{
	set_mode(0);
}

int KeyGetChar()
{
	return get_key();
}
