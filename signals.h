void ignore_stop_signals(void);
void dfl_stop_signals(void);
void handle_sigchld(void(*)(int));
void block_sigchld(void);
void unblock_sigchld(void);
