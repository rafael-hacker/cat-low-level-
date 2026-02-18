#define exit(num)({\
	asm volatile (\
		"syscall"\
		:\
		:"a" (60), "D" (num)\
		:"rcx", "r11", "memory"\
	);\
})
#define write(fd, buf, size) ({\
    asm volatile (\
      "syscall"\
      :\
      :"a" (1), "D" (fd), "S" (buf), "d" (size)\
      :"rcx", "r11", "memory"\
    );\
})
#define len(buf) ({ \
	long ret; \
	unsigned long c = 0; \
	while(buf[c] != '\0') ++c; \
	ret = (long)c;\
})

long read(int fd, char *buf, unsigned long size){
  int ret;

  asm volatile (
    "syscall"
    : "=a" (ret)
    : "a" (0), "D" (fd), "S" (buf), "d" (size)
    : "rcx", "r11", "memory"
  );
  return ret;
}
#define cat_menu "-h for help menu \n cat -n [archive] for cont lines of archive\n cat [archive] from the basic cat \n"

void cat(const char *filename){
	char buffer[(1024 * 8)];
	long wb;
	int fd;

	asm volatile (
		"syscall"
		: "=a" (fd)
		: "a" (2), "D" (filename), "S" (0)
		: "rcx", "r11", "memory"
	);
	
	if (fd < 0){
		exit(1);
	}

	asm volatile(
		"syscall"
		: "=a" (wb)
		: "a" (0), "D" (fd), "S" (buffer),"d" (sizeof(buffer))
		: "rcx", "r11", "memory"
	);

	asm volatile(
		"syscall"
		:
		:"a" (1), "D" (1), "S" (buffer), "d" (wb)
		: "rcx", "memory", "r11"
	);
	exit(0);
	
	
}
void cat_list(const char *path){                                // flag -l for list all archives 
  int fd;
  asm volatile(
    "syscall"
    : "=a" (fd)
    : "a" (2), "D" (path), "S" (0), "d" (0x10000)
    : "rcx", "r11", "memory"
  );
  if (fd < 0){
    exit(1);
  }

  long nread;
  char buf[(32 * 1024)];
  asm volatile(
    "syscall"
    : "=a" (nread)
    : "a" (217), "D" (fd), "S" (buf), "d" (sizeof(buf))
    : "rcx", "r11", "memory"
  );
  int bpos = 0;
  while(bpos < nread){
    char *name = (bpos + buf + 19);        // this 19 is the posiction of d_name 
    write(1, "-> ", 3);
    write(1, name, len(name));
    write(1, "\n", 1);
    unsigned short d_reclen = *(unsigned short *)(buf + bpos + 16);
    bpos += d_reclen;
  }
}

int strcmp(const char *s1, const char *s2){
  int i = 0;
  while(s1[i] == s2[i] && s1[i] != '\0'){
    i++;
  }
  return (unsigned long)s1[i] - (unsigned long)s2[i]; // if s1 = s2 return 0 
}

void cat_lines(const char *filename){ // need the flag -n for use this function 
  int fd; 
  asm volatile (
      "syscall"
      : "=a" (fd)
      : "a" (2), "D" (filename), "S" (0)
      : "rcx", "r11", "memory"
      );

  if (fd < 0){
    exit(1);
  }

  char buf[1899];

  long i = 0;
  while (read(fd, buf, sizeof(fd)) > 0){ // this part counting lines 
    if (buf[0] == '\n'){
      i++;
    }
  }
  if (i == 0){
    write(1, "0", 1);
    exit(0);
  }
  char buffer[20];
  int n = 0;

  while (i > 0){
    buffer[n++] = (i % 10) + 48;               // this part transform int in ansii
    i /= 10;
  }
  buf[n] = '\n';
  while(n--){
    write(1, &buffer[n], 1);
  }
  exit(0);
}


void c_start(long *sp){                              // pointer of the type long resposability for being argc 
	int argc = (int)*sp;                              // here is argc 
	char **argv = (char **)(sp + 1);                 // here is jumping 8 bytes for go to argv
	
	if (argc == 2){
    if (strcmp(argv[1], "-h") == 0){
      write(1, cat_menu, len(cat_menu));
      exit(0);
    }
  }
  
	if (argc == 3){
    if (strcmp(argv[1], "-l") == 0){
      cat_list(argv[2]);
      exit(0);
    }
  }	

    for (int i=1; i < argc; i++){               
		  if (strcmp(argv[1], "-n") == 0){
        cat_lines(argv[i]);
      }
    for (int i = 1; i < argc; i++){
      cat(argv[i]);
    }
}
}

    
void __attribute__((naked)) _start(){
	
	asm volatile(
		
		"mov %rsp, %rdi\n"
		"andq $-16, %rsp\n"
		"call c_start"
	);
}
