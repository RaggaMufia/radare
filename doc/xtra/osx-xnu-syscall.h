http://uninformed.org/index.cgi?v=4&a=3&p=12

///
PTRACE
======

#define PT_TRACE_ME     0       /* child declares it's being traced */
#define PT_READ_I       1       /* read word in child's I space */
#define PT_READ_D       2       /* read word in child's D space */
#define PT_READ_U       3       /* read word in child's user structure */
#define PT_WRITE_I      4       /* write word in child's I space */
#define PT_WRITE_D      5       /* write word in child's D space */
#define PT_WRITE_U      6       /* write word in child's user structure */
#define PT_CONTINUE     7       /* continue the child */
#define PT_KILL         8       /* kill the child process */
#define PT_STEP         9       /* single step the child */
#define PT_ATTACH       10      /* trace some running process */
#define PT_DETACH       11      /* stop tracing a process */
#define PT_SIGEXC       12      /* signals as exceptions for current_proc */
#define PT_THUPDATE     13      /* signal for thread# */
#define PT_ATTACHEXC    14      /* attach to running process with signal exception */

#define PT_FORCEQUOTA   30      /* Enforce quota for root */
#define PT_DENY_ATTACH  31
#define PT_FIRSTMACH    32      /* for machine-specific requests */


Interrupts:
 int 0x80 : used to communicate with the BSD
 int 0x81 : used for MACH traps

////

MACH TRAPS:
===========

Next: MIG Up: Mach Traps (system calls) Previous: Mach Traps (system calls)   Contents

List of mach traps in xnu-792.6.22

/* 26 */	mach_reply_port
/* 27 */	thread_self_trap
/* 28 */	task_self_trap
/* 29 */	host_self_trap
/* 31 */	mach_msg_trap
/* 32 */	mach_msg_overwrite_trap
/* 33 */	semaphore_signal_trap
/* 34 */	semaphore_signal_all_trap
/* 35 */	semaphore_signal_thread_trap
/* 36 */	semaphore_wait_trap
/* 37 */	semaphore_wait_signal_trap
/* 38 */	semaphore_timedwait_trap
/* 39 */	semaphore_timedwait_signal_trap
/* 41 */	init_process
/* 43 */	map_fd
/* 45 */ 	task_for_pid
/* 46 */	pid_for_task
/* 48 */	macx_swapon
/* 49 */	macx_swapoff
/* 51 */	macx_triggers
/* 52 */	macx_backing_store_suspend
/* 53 */	macx_backing_store_recovery
/* 59 */ 	swtch_pri
/* 60 */	swtch
/* 61 */	thread_switch
/* 62 */	clock_sleep_trap
/* 89 */	mach_timebase_info_trap
/* 90 */	mach_wait_until_trap
/* 91 */	mk_timer_create_trap
/* 92 */	mk_timer_destroy_trap
/* 93 */	mk_timer_arm_trap
/* 94 */	mk_timer_cancel_trap
/* 95 */	mk_timebase_info_trap
/* 100 */	iokit_user_client_trap

When executing one of these traps the number on the left hand side (multiplied by -1) must be placed into the eax register. (intel) Each of the arguments must be pushed to the stack in reverse order. Although I could go into a low level description of how to send a mach msg here, the paper [11] in the references has already done this and the author is a lot better at it than me. I strongly suggest reading this paper if you are at all interested in the subject matter. 

/**************************************/

BSD TRAPS:
==========

#ifndef _SYS_SYSCALL_H_
#define	_SYS_SYSCALL_H_

#include <sys/appleapiopts.h>
#ifdef __APPLE_API_PRIVATE
#define	SYS_syscall        0
#define	SYS_exit           1
#define	SYS_fork           2
#define	SYS_read           3
#define	SYS_write          4
#define	SYS_open           5
#define	SYS_close          6
#define	SYS_wait4          7
			/* 8  old creat */
#define	SYS_link           9
#define	SYS_unlink         10
			/* 11  old execv */
#define	SYS_chdir          12
#define	SYS_fchdir         13
#define	SYS_mknod          14
#define	SYS_chmod          15
#define	SYS_chown          16
#define	SYS_obreak         17
#define	SYS_ogetfsstat     18
#define	SYS_getfsstat      18
			/* 19  old lseek */
#define	SYS_getpid         20
			/* 21  old mount */
			/* 22  old umount */
#define	SYS_setuid         23
#define	SYS_getuid         24
#define	SYS_geteuid        25
#define	SYS_ptrace         26
#define	SYS_recvmsg        27
#define	SYS_sendmsg        28
#define	SYS_recvfrom       29
#define	SYS_accept         30
#define	SYS_getpeername    31
#define	SYS_getsockname    32
#define	SYS_access         33
#define	SYS_chflags        34
#define	SYS_fchflags       35
#define	SYS_sync           36
#define	SYS_kill           37
			/* 38  old stat */
#define	SYS_getppid        39
			/* 40  old lstat */
#define	SYS_dup            41
#define	SYS_pipe           42
#define	SYS_getegid        43
#define	SYS_profil         44
			/* 45  old ktrace */
#define	SYS_sigaction      46
#define	SYS_getgid         47
#define	SYS_sigprocmask    48
#define	SYS_getlogin       49
#define	SYS_setlogin       50
#define	SYS_acct           51
#define	SYS_sigpending     52
#define	SYS_sigaltstack    53
#define	SYS_ioctl          54
#define	SYS_reboot         55
#define	SYS_revoke         56
#define	SYS_symlink        57
#define	SYS_readlink       58
#define	SYS_execve         59
#define	SYS_umask          60
#define	SYS_chroot         61
			/* 62  old fstat */
			/* 63  used internally , reserved */
			/* 64  old getpagesize */
#define	SYS_msync          65
#define	SYS_vfork          66
			/* 67  old vread */
			/* 68  old vwrite */
#define	SYS_sbrk           69
#define	SYS_sstk           70
			/* 71  old mmap */
#define	SYS_ovadvise       72
#define	SYS_munmap         73
#define	SYS_mprotect       74
#define	SYS_madvise        75
			/* 76  old vhangup */
			/* 77  old vlimit */
#define	SYS_mincore        78
#define	SYS_getgroups      79
#define	SYS_setgroups      80
#define	SYS_getpgrp        81
#define	SYS_setpgid        82
#define	SYS_setitimer      83
			/* 84  old wait */
#define	SYS_swapon         85
#define	SYS_getitimer      86
			/* 87  old gethostname */
			/* 88  old sethostname */
#define	SYS_getdtablesize  89
#define	SYS_dup2           90
			/* 91  old getdopt */
#define	SYS_fcntl          92
#define	SYS_select         93
			/* 94  old setdopt */
#define	SYS_fsync          95
#define	SYS_setpriority    96
#define	SYS_socket         97
#define	SYS_connect        98
			/* 99  old accept */
#define	SYS_getpriority    100
			/* 101  old send */
			/* 102  old recv */
			/* 103  old sigreturn */
#define	SYS_bind           104
#define	SYS_setsockopt     105
#define	SYS_listen         106
			/* 107  old vtimes */
			/* 108  old sigvec */
			/* 109  old sigblock */
			/* 110  old sigsetmask */
#define	SYS_sigsuspend     111
			/* 112  old sigstack */
			/* 113  old recvmsg */
			/* 114  old sendmsg */
			/* 115  old vtrace */
#define	SYS_gettimeofday   116
#define	SYS_getrusage      117
#define	SYS_getsockopt     118
			/* 119  old resuba */
#define	SYS_readv          120
#define	SYS_writev         121
#define	SYS_settimeofday   122
#define	SYS_fchown         123
#define	SYS_fchmod         124
			/* 125  old recvfrom */
#define	SYS_setreuid       126
#define	SYS_setregid       127
#define	SYS_rename         128
			/* 129  old truncate */
			/* 130  old ftruncate */
#define	SYS_flock          131
#define	SYS_mkfifo         132
#define	SYS_sendto         133
#define	SYS_shutdown       134
#define	SYS_socketpair     135
#define	SYS_mkdir          136
#define	SYS_rmdir          137
#define	SYS_utimes         138
#define	SYS_futimes        139
#define	SYS_adjtime        140
			/* 141  old getpeername */
#define	SYS_gethostuuid    142
			/* 143  old sethostid */
			/* 144  old getrlimit */
			/* 145  old setrlimit */
			/* 146  old killpg */
#define	SYS_setsid         147
			/* 148  old setquota */
			/* 149  old qquota */
			/* 150  old getsockname */
#define	SYS_getpgid        151
#define	SYS_setprivexec    152
#define	SYS_pread          153
#define	SYS_pwrite         154
#define	SYS_nfssvc         155
			/* 156  old getdirentries */
#define	SYS_statfs         157
#define	SYS_fstatfs        158
#define	SYS_unmount        159
			/* 160  old async_daemon */
#define	SYS_getfh          161
			/* 162  old getdomainname */
			/* 163  old setdomainname */
			/* 164  */
#define	SYS_quotactl       165
			/* 166  old exportfs */
#define	SYS_mount          167
			/* 168  old ustat */
#define	SYS_csops          169
#define	SYS_table          170
			/* 171  old wait3 */
			/* 172  old rpause */
#define	SYS_waitid         173
			/* 174  old getdents */
			/* 175  old gc_control */
#define	SYS_add_profil     176
			/* 177  */
			/* 178  */
			/* 179  */
#define	SYS_kdebug_trace   180
#define	SYS_setgid         181
#define	SYS_setegid        182
#define	SYS_seteuid        183
#define	SYS_sigreturn      184
#define	SYS_chud           185
			/* 186  */
			/* 187  */
#define	SYS_stat           188
#define	SYS_fstat          189
#define	SYS_lstat          190
#define	SYS_pathconf       191
#define	SYS_fpathconf      192
			/* 193  */
#define	SYS_getrlimit      194
#define	SYS_setrlimit      195
#define	SYS_getdirentries  196
#define	SYS_mmap           197
			/* 198  __syscall */
#define	SYS_lseek          199
#define	SYS_truncate       200
#define	SYS_ftruncate      201
#define	SYS___sysctl       202
#define	SYS_mlock          203
#define	SYS_munlock        204
#define	SYS_undelete       205
#define	SYS_ATsocket       206
#define	SYS_ATgetmsg       207
#define	SYS_ATputmsg       208
#define	SYS_ATPsndreq      209
#define	SYS_ATPsndrsp      210
#define	SYS_ATPgetreq      211
#define	SYS_ATPgetrsp      212
			/* 213  Reserved for AppleTalk */
#define	SYS_kqueue_from_portset_np 214
#define	SYS_kqueue_portset_np 215
#define	SYS_mkcomplex      216
#define	SYS_statv          217
#define	SYS_lstatv         218
#define	SYS_fstatv         219
#define	SYS_getattrlist    220
#define	SYS_setattrlist    221
#define	SYS_getdirentriesattr 222
#define	SYS_exchangedata   223
			/* 224  was checkuseraccess */
#define	SYS_searchfs       225
#define	SYS_delete         226
#define	SYS_copyfile       227
			/* 228  */
			/* 229  */
#define	SYS_poll           230
#define	SYS_watchevent     231
#define	SYS_waitevent      232
#define	SYS_modwatch       233
#define	SYS_getxattr       234
#define	SYS_fgetxattr      235
#define	SYS_setxattr       236
#define	SYS_fsetxattr      237
#define	SYS_removexattr    238
#define	SYS_fremovexattr   239
#define	SYS_listxattr      240
#define	SYS_flistxattr     241
#define	SYS_fsctl          242
#define	SYS_initgroups     243
#define	SYS_posix_spawn    244
			/* 245  */
			/* 246  */
#define	SYS_nfsclnt        247
#define	SYS_fhopen         248
			/* 249  */
#define	SYS_minherit       250
#define	SYS_semsys         251
#define	SYS_msgsys         252
#define	SYS_shmsys         253
#define	SYS_semctl         254
#define	SYS_semget         255
#define	SYS_semop          256
			/* 257  */
#define	SYS_msgctl         258
#define	SYS_msgget         259
#define	SYS_msgsnd         260
#define	SYS_msgrcv         261
#define	SYS_shmat          262
#define	SYS_shmctl         263
#define	SYS_shmdt          264
#define	SYS_shmget         265
#define	SYS_shm_open       266
#define	SYS_shm_unlink     267
#define	SYS_sem_open       268
#define	SYS_sem_close      269
#define	SYS_sem_unlink     270
#define	SYS_sem_wait       271
#define	SYS_sem_trywait    272
#define	SYS_sem_post       273
#define	SYS_sem_getvalue   274
#define	SYS_sem_init       275
#define	SYS_sem_destroy    276
#define	SYS_open_extended  277
#define	SYS_umask_extended 278
#define	SYS_stat_extended  279
#define	SYS_lstat_extended 280
#define	SYS_fstat_extended 281
#define	SYS_chmod_extended 282
#define	SYS_fchmod_extended 283
#define	SYS_access_extended 284
#define	SYS_settid         285
#define	SYS_gettid         286
#define	SYS_setsgroups     287
#define	SYS_getsgroups     288
#define	SYS_setwgroups     289
#define	SYS_getwgroups     290
#define	SYS_mkfifo_extended 291
#define	SYS_mkdir_extended 292
#define	SYS_identitysvc    293
#define	SYS_shared_region_check_np 294
#define	SYS_shared_region_map_np 295
			/* 296  old load_shared_file */
			/* 297  old reset_shared_file */
			/* 298  old new_system_shared_regions */
			/* 299  old shared_region_map_file_np */
			/* 300  old shared_region_make_private_np */
#define	SYS___pthread_mutex_destroy 301
#define	SYS___pthread_mutex_init 302
#define	SYS___pthread_mutex_lock 303
#define	SYS___pthread_mutex_trylock 304
#define	SYS___pthread_mutex_unlock 305
#define	SYS___pthread_cond_init 306
#define	SYS___pthread_cond_destroy 307
#define	SYS___pthread_cond_broadcast 308
#define	SYS___pthread_cond_signal 309
#define	SYS_getsid         310
#define	SYS_settid_with_pid 311
#define	SYS___pthread_cond_timedwait 312
#define	SYS_aio_fsync      313
#define	SYS_aio_return     314
#define	SYS_aio_suspend    315
#define	SYS_aio_cancel     316
#define	SYS_aio_error      317
#define	SYS_aio_read       318
#define	SYS_aio_write      319
#define	SYS_lio_listio     320
#define	SYS___pthread_cond_wait 321
#define	SYS_iopolicysys    322
			/* 323  */
#define	SYS_mlockall       324
#define	SYS_munlockall     325
			/* 326  */
#define	SYS_issetugid      327
#define	SYS___pthread_kill 328
#define	SYS___pthread_sigmask 329
#define	SYS___sigwait      330
#define	SYS___disable_threadsignal 331
#define	SYS___pthread_markcancel 332
#define	SYS___pthread_canceled 333
#define	SYS___semwait_signal 334
			/* 335  old utrace */
#define	SYS_proc_info      336
#define	SYS_sendfile       337
#define	SYS_stat64         338
#define	SYS_fstat64        339
#define	SYS_lstat64        340
#define	SYS_stat64_extended 341
#define	SYS_lstat64_extended 342
#define	SYS_fstat64_extended 343
#define	SYS_getdirentries64 344
#define	SYS_statfs64       345
#define	SYS_fstatfs64      346
#define	SYS_getfsstat64    347
#define	SYS___pthread_chdir 348
#define	SYS___pthread_fchdir 349
#define	SYS_audit          350
#define	SYS_auditon        351
			/* 352  */
#define	SYS_getauid        353
#define	SYS_setauid        354
#define	SYS_getaudit       355
#define	SYS_setaudit       356
#define	SYS_getaudit_addr  357
#define	SYS_setaudit_addr  358
#define	SYS_auditctl       359
#define	SYS_bsdthread_create 360
#define	SYS_bsdthread_terminate 361
#define	SYS_kqueue         362
#define	SYS_kevent         363
#define	SYS_lchown         364
#define	SYS_stack_snapshot 365
#define	SYS_bsdthread_register 366
#define	SYS_workq_open     367
#define	SYS_workq_ops      368
			/* 369  */
			/* 370  */
			/* 371  */
			/* 372  */
			/* 373  */
			/* 374  */
			/* 375  */
			/* 376  */
			/* 377  */
			/* 378  */
			/* 379  */
#define	SYS___mac_execve   380
#define	SYS___mac_syscall  381
#define	SYS___mac_get_file 382
#define	SYS___mac_set_file 383
#define	SYS___mac_get_link 384
#define	SYS___mac_set_link 385
#define	SYS___mac_get_proc 386
#define	SYS___mac_set_proc 387
#define	SYS___mac_get_fd   388
#define	SYS___mac_set_fd   389
#define	SYS___mac_get_pid  390
#define	SYS___mac_get_lcid 391
#define	SYS___mac_get_lctx 392
#define	SYS___mac_set_lctx 393
#define	SYS_setlcid        394
#define	SYS_getlcid        395
#define	SYS_read_nocancel  396
#define	SYS_write_nocancel 397
#define	SYS_open_nocancel  398
#define	SYS_close_nocancel 399
#define	SYS_wait4_nocancel 400
#define	SYS_recvmsg_nocancel 401
#define	SYS_sendmsg_nocancel 402
#define	SYS_recvfrom_nocancel 403
#define	SYS_accept_nocancel 404
#define	SYS_msync_nocancel 405
#define	SYS_fcntl_nocancel 406
#define	SYS_select_nocancel 407
#define	SYS_fsync_nocancel 408
#define	SYS_connect_nocancel 409
#define	SYS_sigsuspend_nocancel 410
#define	SYS_readv_nocancel 411
#define	SYS_writev_nocancel 412
#define	SYS_sendto_nocancel 413
#define	SYS_pread_nocancel 414
#define	SYS_pwrite_nocancel 415
#define	SYS_waitid_nocancel 416
#define	SYS_poll_nocancel  417
#define	SYS_msgsnd_nocancel 418
#define	SYS_msgrcv_nocancel 419
#define	SYS_sem_wait_nocancel 420
#define	SYS_aio_suspend_nocancel 421
#define	SYS___sigwait_nocancel 422
#define	SYS___semwait_signal_nocancel 423
#define	SYS___mac_mount    424
#define	SYS___mac_get_mount 425
#define	SYS___mac_getfsstat 426
#define	SYS_MAXSYSCALL	427

#endif /* __APPLE_API_PRIVATE */
#endif /* !_SYS_SYSCALL_H_ */
