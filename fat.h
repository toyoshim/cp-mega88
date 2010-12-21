#if !defined __fat_h__
# define __fat_h__

int fat_init(void);
void fat_rewind(void);
int fat_next(void);
void fat_name(char *namebuf);
char fat_attr(void);
int fat_chdir(void);
unsigned long fat_size(void);
int fat_open(void);
int fat_seek(unsigned long pos);
int fat_read(void);

#endif // !defined(__fat_h__)
