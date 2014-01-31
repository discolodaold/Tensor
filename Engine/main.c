#include "nos.h"

#include "user.h"
#include "shell.h"
#include "fs.h"
#include "resource.h"
#include "proc.h"

int time;

proc(fps) {
	static int time_start;
	
	proc_begin();
	
	time_start = nos_time();
	
	while(1) {
		time = nos_time() - time_start;
		proc_yield();
	}
	
	proc_end();
}

int argc = 0;
char *argv[] = {};

int nos_init(void) {
	shell_init(argc, argv);
	user_init();
	fs_init();
	resource_init();
	
	proc_spawn(fps, NULL);
	
	return 0;
}

int nos_frame(void) {
	return !proc_loop_step();
}

void nos_uninit(void) {
}