#if !defined __i8080_h__
# define __i8080_h__

struct i8080_work {
  unsigned char reg_a;
  unsigned char reg_f;
  unsigned char reg_b;
  unsigned char reg_c;
  unsigned char reg_d;
  unsigned char reg_e;
  unsigned char reg_h;
  unsigned char reg_l;
  unsigned char reg_pc_h;
  unsigned char reg_pc_l;
  unsigned char reg_sp_h;
  unsigned char reg_sp_l;
};

extern struct i8080_work i8080_work;

void i8080_reset(void);
char i8080_run(void);

#endif // !defined(__i8080_h__)
