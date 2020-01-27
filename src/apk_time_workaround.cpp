#include <time.h>

extern "C" {

// copy-pasted from apk-tools src/apk.c
// without this I get a link error: undefined reference

time_t apk_time(void)
{
#ifdef TEST_MODE
	return 1559567666;
#else
	return time(nullptr);
#endif
}

}
