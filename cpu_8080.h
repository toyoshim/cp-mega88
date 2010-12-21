#if !defined(__cpu_8080_h__)
# define __cpu_8080_h__

# if defined(__cplusplus)
extern "C" {
# endif // defined(__cplusplus)

typedef unsigned char (*cpu_8080_load_8)(unsigned short addr);
typedef void (*cpu_8080_store_8)(unsigned short addr, unsigned char val);
typedef unsigned char (*cpu_8080_in)(unsigned char port);
typedef void (*cpu_8080_out)(unsigned char port, unsigned char val);

typedef struct _cpu_8080_work {
  unsigned char a;
  unsigned char f;
  unsigned char b;
  unsigned char c;
  unsigned char d;
  unsigned char e;
  unsigned char h;
  unsigned char l;
  unsigned short pc;
  unsigned short sp;
  cpu_8080_load_8 load_8;
  cpu_8080_store_8 store_8;
  cpu_8080_in in;
  cpu_8080_out out;
  unsigned char op;
} cpu_8080_work;

void cpu_8080_reset(cpu_8080_work *);
int cpu_8080_step(cpu_8080_work *);

# if defined(__cplusplus)
};
# endif // defined(__cplusplus)

#endif // !defined(__cpu_8080_h__)
