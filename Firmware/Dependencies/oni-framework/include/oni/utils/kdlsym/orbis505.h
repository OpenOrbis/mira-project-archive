#pragma once
#include <oni/config.h>

#if ONI_PLATFORM==ONI_PLATFORM_ORBIS_BSD_505
/*
These are the required functions in order for the Oni Framework to operate properly
These are all offsets into the base of the kernel. They expect all standard FreeBSD 9 prototypes

The reason we do not hardcode offsets here, is due to the different platforms that are supported, and
for the platforms that do enable kernel ASLR (Address Space Layout Randomization?)
*/

#define kdlsym_addr__mtx_lock_flags                        0x00401CD0
#define kdlsym_addr__mtx_lock_sleep                        0x00401D70
#define kdlsym_addr__mtx_unlock_flags                      0x00401FA0
#define kdlsym_addr__mtx_unlock_sleep                      0x004020A0
#define kdlsym_addr__sceSblAuthMgrGetSelfInfo              0x0063CD40
#define kdlsym_addr__sx_slock                              0x000F5C30
#define kdlsym_addr__sx_sunlock                            0x000F5F10
#define kdlsym_addr__sx_xlock                              0x000F5E10
#define kdlsym_addr__sx_xunlock                            0x000F5FD0
#define kdlsym_addr__vm_map_lock_read                      0x0019F140
#define kdlsym_addr__vm_map_unlock_read                    0x0019F190
#define kdlsym_addr_AesCbcCfb128Decrypt                    0x003A2E00
#define kdlsym_addr_AesCbcCfb128Encrypt                    0x003A2BD0
#define kdlsym_addr_allproc                                0x02382FF8
#define kdlsym_addr_allproc_lock                           0x02382F98
#define kdlsym_addr_copyin                                 0x001EA710
#define kdlsym_addr_copyinstr                              0x001EAB40
#define kdlsym_addr_critical_enter                         0x0028E7A0
#define kdlsym_addr_critical_exit                          0x0028E7B0
#define kdlsym_addr_dmem_start_app_process                 0x002469F0
#define kdlsym_addr_eventhandler_register                  0x001EC400
#define kdlsym_addr_exec_new_vmspace                       0x0038AD10
#define kdlsym_addr_faultin                                0x00006DD0
#define kdlsym_addr_fget_unlocked                          0x000C34B0
#define kdlsym_addr_fpu_ctx                                0x0274C040
#define kdlsym_addr_fpu_kern_enter                         0x001BFF90
#define kdlsym_addr_fpu_kern_leave                         0x001C0090
#define kdlsym_addr_free                                   0x0010E460
#define kdlsym_addr_gpu_va_page_list                       0x0271E208
#define kdlsym_addr_icc_nvs_read                           0x00395830
#define kdlsym_addr_kern_close                             0x000C0EC0
#define kdlsym_addr_kern_mkdirat                           0x00340BD0
#define kdlsym_addr_kern_open                              0x0033B9B0
#define kdlsym_addr_kern_openat                            0x0033BA10
#define kdlsym_addr_kern_readv                             0x00153248
#define kdlsym_addr_kern_reboot                            0x0010D390
#define kdlsym_addr_kern_sysents                           0x0107C610
#define kdlsym_addr_kern_thr_create						   0x001BE1F0   
#define kdlsym_addr_kernel_map                             0x01AC60E0
#define kdlsym_addr_kmem_alloc                             0x000FCC80
#define kdlsym_addr_kmem_free                              0x000FCE50
#define kdlsym_addr_kproc_create                           0x00137DF0
#define kdlsym_addr_kthread_add                            0x00138360
#define kdlsym_addr_kthread_exit                           0x00138640
#define kdlsym_addr_M_MOUNT                                0x019BF300
#define kdlsym_addr_M_TEMP                                 0x014B4110
#define kdlsym_addr_malloc                                 0x0010E250
#define kdlsym_addr_memcmp                                 0x00050AC0
#define kdlsym_addr_memcpy                                 0x001EA530
#define kdlsym_addr_memmove								   0x00073BA0
#define kdlsym_addr_memset                                 0x003205C0
#define kdlsym_addr_mini_syscore_self_binary               0x014C9D48
#define kdlsym_addr_mtx_init                               0x00402780
#define kdlsym_addr_mtx_lock_sleep                         0x00401D70
#define kdlsym_addr_mtx_unlock_sleep                       0x004020A0
#define kdlsym_addr_pfind                                  0x004034E0
#define kdlsym_addr_pmap_activate                          0x002EAFD0
#define kdlsym_addr_printf                                 0x00436040
#define kdlsym_addr_prison0                                0x010986A0
#define kdlsym_addr_proc0                                  0x01AA4600
#define kdlsym_addr_proc_reparent                          0x00035330
#define kdlsym_addr_proc_rwmem                             0x0030D150
#define kdlsym_addr_realloc                                0x0010E590
#define kdlsym_addr_rootvnode                              0x022C1A70
#define kdlsym_addr_RsaesPkcs1v15Dec2048CRT                0x001FD7D0
#define kdlsym_addr_sbl_eap_internal_partition_key         0x02790C90
#define kdlsym_addr_sbl_pfs_sx                             0x0271E5D8
#define kdlsym_addr_sceSblAuthMgrIsLoadable2               0x0063C4F0
#define kdlsym_addr_sceSblAuthMgrSmStart                   0x006418E0
#define kdlsym_addr_sceSblAuthMgrSmVerifyHeader            0x00642B40
#define kdlsym_addr_sceSblAuthMgrVerifyHeader              0x0063C550
#define kdlsym_addr_sceSblDriverSendMsg                    0x0061D7F0
#define kdlsym_addr_sceSblGetEAPInternalPartitionKey       0x006256E0
#define kdlsym_addr_sceSblKeymgrClearKey                   0x0062DB10
#define kdlsym_addr_sceSblKeymgrSetKeyForPfs               0x0062D780
#define kdlsym_addr_sceSblKeymgrSmCallfunc                 0x0062E2A0
#define kdlsym_addr_sceSblPfsSetKeys                       0x0061EFA0
#define kdlsym_addr_sceSblServiceMailbox                   0x00632540
#define kdlsym_addr_self_orbis_sysvec                      0x019BBCD0
#define kdlsym_addr_Sha256Hmac                             0x002D55B0
#define kdlsym_addr_snprintf                               0x00436350
#define kdlsym_addr_sscanf                                 0x00175900
#define kdlsym_addr_strcmp                                 0x001D0FD0
#define kdlsym_addr_strdup                                 0x001C1C30
#define kdlsym_addr_strlen                                 0x003B71A0
#define kdlsym_addr_strncmp                                0x001B8FE0
#define kdlsym_addr_strstr                                 0x0017DFB0
#define kdlsym_addr_sys_accept                             0x0031A170
#define kdlsym_addr_sys_bind                               0x00319820
#define kdlsym_addr_sys_close                              0x000C0EB0
#define kdlsym_addr_sys_dup2                               0x000BF050
#define kdlsym_addr_sys_fstat                              0x000C1430
#define kdlsym_addr_sys_getdents                           0x00341390
#define kdlsym_addr_sys_kill                               0x000D19D0
#define kdlsym_addr_sys_listen                             0x00319A60
#define kdlsym_addr_sys_lseek                              0x0033D9F0
#define kdlsym_addr_sys_mkdir                              0x00340B50
#define kdlsym_addr_sys_mlock                              0x0013E250
#define kdlsym_addr_sys_mlockall                           0x0013E300
#define kdlsym_addr_sys_mmap                               0x0013D230
#define kdlsym_addr_sys_munmap                             0x0013D9A0
#define kdlsym_addr_sys_nmount                             0x001DE2E0
#define kdlsym_addr_sys_open                               0x0033B990
#define kdlsym_addr_sys_ptrace                             0x0030D5E0
#define kdlsym_addr_sys_read                               0x00152AB0
#define kdlsym_addr_sys_recvfrom                           0x0031B460
#define kdlsym_addr_sys_rmdir                              0x00340ED0
#define kdlsym_addr_sys_sendto                             0x0031AD10
#define kdlsym_addr_sys_setuid                             0x00054950
#define kdlsym_addr_sys_shutdown                           0x0031B6A0
#define kdlsym_addr_sys_socket                             0x00318EE0
#define kdlsym_addr_sys_stat                               0x0033DFE0
#define kdlsym_addr_sys_unlink                             0x0033D3D0
#define kdlsym_addr_sys_unmount                            0x001DFC70
#define kdlsym_addr_sys_wait4                              0x00035470
#define kdlsym_addr_sys_write                              0x00152FC0
#define kdlsym_addr_trap_fatal                             0x00171580
#define kdlsym_addr_utilUSleep                             0x00658C30
#define kdlsym_addr_vm_map_lookup_entry                    0x0019F760
#define kdlsym_addr_vmspace_acquire_ref                    0x0019EF90
#define kdlsym_addr_vmspace_alloc                          0x0019EB20
#define kdlsym_addr_vmspace_free                           0x0019EDC0
#define kdlsym_addr_vsnprintf                              0x004363F0
#define kdlsym_addr_Xfast_syscall                          0x000001C0
#define kdlsym_addr_wakeup                                 0x003FB940

#endif
