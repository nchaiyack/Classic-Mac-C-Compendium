# include <unistd.h>
# include <fcntl.h>

long tell(int fd)
{
return lseek(fd, 0, SEEK_CUR);
}
