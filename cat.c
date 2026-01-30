#define exit(num)({\
	asm volatile (\
		"syscall"\
		:\
		:"a" (60), "D" (num)\
		:"rcx", "r11", "memory"\
	);\
})

#define len(buf) ({ \
	long ret; \
	unsigned long c = 0; \
	while(buf[c] != '\0') ++c; \
	ret = (long)c;\
})


void cat(const char *filename){
	int fd;
	char buffer[(1024 * 8)];
	long wb;
	asm volatile (
		"syscall"
		:"=a" (fd)
		: "a" (2), "D" (filename), "S" (0)
		: "r11", "memory", "rcx"
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

void cat_stdin(){
	int fd;
	char buffer[(1024 * 8)];
	long wb;
	asm volatile(
		"syscall"
		: "=a" (fd)
		: "a" (2), "D" (0), "S" (0)
		: "r11", "rcx", "memory"
	);
	asm volatile (
		"syscall"
		: "=a" (wb)
		: "a" (0), "D" (0), "S" (buffer), "d" (sizeof(buffer))
		: "r11", "memory", "rcx"
	);
	asm volatile (
		"syscall"
		:
		: "a" (1), "D" (1), "S" (buffer), "d" (wb)
		: "rcx", "r11", "memory"
	);
}
void c_start(long *sp){                    // ponteiro long responsavel por ser o argc
	int argc = (int)*sp;              // aqui eo argc 
	char **argv = (char **)(sp + 1); // aqui eu ando 8 bytes para chegar em argv[0]
	
	if (argc == 1){
		cat_stdin();	
	}
	for (int i = 1; i < argc; i++){ // para aceitar mais arquivos 
		cat(argv[i]);
	}
	
}

void __attribute__((naked)) _start(){
	
	asm volatile(
		
		"mov %rsp, %rdi\n"
		"andq $-16, %rsp\n"
		"call c_start"
	);
}
