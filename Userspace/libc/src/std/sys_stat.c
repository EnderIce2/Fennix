#include <sys/stat.h>

#include <sys/types.h> // For PUBLIC

PUBLIC int mkdir(const char *path, mode_t mode)
{
	return 0;
}

PUBLIC int remove(const char *pathname)
{
	return 0;
}

PUBLIC int rename(const char *oldpath, const char *newpath)
{
	return 0;
}
