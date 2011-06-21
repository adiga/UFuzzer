#include <sys/stat.h>
#include <sys/uio.h>

unsigned int rand32(void);
long long rand64(void);

void rand_string_gen(char *buf, int buf_len);

int rand_fd(void);
char *rand_pathname(void);

void *rand_buf(char rw, unsigned int size);
struct iovec *rand_iovec(int size);

mode_t rand_filemode(void);
mode_t rand_statusmode(void);
int  rand_dirflags(void);

unsigned long rand_bitflip(unsigned long data);
