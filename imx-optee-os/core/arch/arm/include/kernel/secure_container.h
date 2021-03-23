#ifndef NAMESPACES_H
#define NAMESPACES_H

#include <compiler.h>
#include <stdint.h>
#include <types_ext.h>
#include <kernel/thread.h>
#include <sm/optee_smc.h>

#define OPTEE_SMC_FUNCID_UPDATE_CONTAINER_REGION   15
#define OPTEE_SMC_UPDATE_CONTAINER_REGION \
	OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_UPDATE_CONTAINER_REGION)

#define UID_GID_MAP_MAX_EXTENTS 5
#define UCOUNT_COUNTS 7
#define __NEW_UTS_LEN 64

typedef struct {
	int counter;
} atomic_t;

typedef struct {
	uint32_t val;
} kuid_t;

typedef struct {
	uint32_t val;
} kgid_t;

struct ns_common {
	atomic_t stashed;
	const struct proc_ns_operations *ops;
	unsigned int inum;
};

struct uid_gid_map {	/* 64 bytes -- 1 cache line */
	uint32_t nr_extents;
	struct uid_gid_extent {
		uint32_t first;
		uint32_t lower_first;
		uint32_t count;
	} extent[UID_GID_MAP_MAX_EXTENTS];
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};



struct nsproxy {
	int count;
	struct uts_namespace *uts_ns;
	struct ipc_namespace *ipc_ns;
	struct mnt_namespace *mnt_ns;
	struct pid_namespace *pid_ns_for_children;
	struct net 	     *net_ns;
	struct cgroup_namespace *cgroup_ns;
};

struct proc_ns_operations {
	const char *name;
	const char *real_ns_name;
	int type;
	struct ns_common *(*get)(int *task);
	void (*put)(struct ns_common *ns);
	int (*install)(struct nsproxy *nsproxy, struct ns_common *ns);
	struct user_namespace *(*owner)(struct ns_common *ns);
	struct ns_common *(*get_parent)(struct ns_common *ns);
} ;

struct ucounts {
	struct hlist_node node;
	struct user_namespace *ns;
	kuid_t uid;
	int count;
	atomic_t ucount[UCOUNT_COUNTS];
};

struct user_namespace {
	struct uid_gid_map	uid_map;
	struct uid_gid_map	gid_map;
	struct uid_gid_map	projid_map;
	atomic_t		count;
	struct user_namespace	*parent;
	int			level;
	kuid_t			owner;
	kgid_t			group;
	struct ns_common	ns;
	unsigned long		flags;

} ;


struct new_utsname {
	char sysname[__NEW_UTS_LEN + 1];
	char nodename[__NEW_UTS_LEN + 1];
	char release[__NEW_UTS_LEN + 1];
	char version[__NEW_UTS_LEN + 1];
	char machine[__NEW_UTS_LEN + 1];
	char domainname[__NEW_UTS_LEN + 1];
};

typedef struct refcount_struct {
	atomic_t refs;
} refcount_t;

struct kref {
	refcount_t refcount;
};

struct uts_namespace {
	struct kref kref;
	struct new_utsname name;
	struct user_namespace *user_ns;
	struct ucounts *ucounts;
	struct ns_common ns;
} ;


// struct ipc_namespace {
// 	refcount_t	count;
// 	struct ipc_ids	ids[3];

// 	int		sem_ctls[4];
// 	int		used_sems;

// 	unsigned int	msg_ctlmax;
// 	unsigned int	msg_ctlmnb;
// 	unsigned int	msg_ctlmni;
// 	atomic_t	msg_bytes;
// 	atomic_t	msg_hdrs;

// 	size_t		shm_ctlmax;
// 	size_t		shm_ctlall;
// 	unsigned long	shm_tot;
// 	int		shm_ctlmni;
// 	/*
// 	 * Defines whether IPC_RMID is forced for _all_ shm segments regardless
// 	 * of shmctl()
// 	 */
// 	int		shm_rmid_forced;

// 	struct notifier_block ipcns_nb;

// 	/* The kern_mount of the mqueuefs sb.  We take a ref on it */
// 	struct vfsmount	*mq_mnt;

// 	/* # queues in this ns, protected by mq_lock */
// 	unsigned int    mq_queues_count;

// 	/* next fields are set through sysctl */
// 	unsigned int    mq_queues_max;   /* initialized to DFLT_QUEUESMAX */
// 	unsigned int    mq_msg_max;      /* initialized to DFLT_MSGMAX */
// 	unsigned int    mq_msgsize_max;  /* initialized to DFLT_MSGSIZEMAX */
// 	unsigned int    mq_msg_default;
// 	unsigned int    mq_msgsize_default;

// 	/* user_ns which owns the ipc ns */
// 	struct user_namespace *user_ns;
// 	struct ucounts *ucounts;

// 	struct ns_common ns;
// } ;

// struct mnt_namespace {
// 	atomic_t		count;
// 	struct ns_common	ns;
// 	struct mount *	root;
// 	struct list_head	list;
// 	struct user_namespace	*user_ns;
// 	struct ucounts		*ucounts;
// 	u64			seq;	/* Sequence number to prevent loops */
// 	wait_queue_head_t poll;
// 	u64 event;
// 	unsigned int		mounts; /* # of mounts in the namespace */
// 	unsigned int		pending_mounts;
// } ;

// struct pid_namespace {
// 	struct kref kref;
// 	struct pidmap pidmap[PIDMAP_ENTRIES];
// 	struct rcu_head rcu;
// 	int last_pid;
// 	unsigned int nr_hashed;
// 	struct task_struct *child_reaper;
// 	struct kmem_cache *pid_cachep;
// 	unsigned int level;
// 	struct pid_namespace *parent;
// #ifdef CONFIG_PROC_FS
// 	struct vfsmount *proc_mnt;
// 	struct dentry *proc_self;
// 	struct dentry *proc_thread_self;
// #endif
// #ifdef CONFIG_BSD_PROCESS_ACCT
// 	struct fs_pin *bacct;
// #endif
// 	struct user_namespace *user_ns;
// 	struct ucounts *ucounts;
// 	struct work_struct proc_work;
// 	kgid_t pid_gid;
// 	int hide_pid;
// 	int reboot;	/* group exit code if this pidns was rebooted */
// 	struct ns_common ns;
// } ;

// struct net {
// 	refcount_t		passive;	/* To decided when the network
// 						 * namespace should be freed.
// 						 */
// 	atomic_t		count;		/* To decided when the network
// 						 *  namespace should be shut down.
// 						 */
// 	spinlock_t		rules_mod_lock;

// 	atomic64_t		cookie_gen;

// 	struct list_head	list;		/* list of network namespaces */
// 	struct list_head	cleanup_list;	/* namespaces on death row */
// 	struct list_head	exit_list;	/* Use only net_mutex */

// 	struct user_namespace   *user_ns;	/* Owning user namespace */
// 	struct ucounts		*ucounts;
// 	spinlock_t		nsid_lock;
// 	struct idr		netns_ids;

// 	struct ns_common	ns;

// 	struct proc_dir_entry 	*proc_net;
// 	struct proc_dir_entry 	*proc_net_stat;

// #ifdef CONFIG_SYSCTL
// 	struct ctl_table_set	sysctls;
// #endif

// 	struct sock 		*rtnl;			/* rtnetlink socket */
// 	struct sock		*genl_sock;

// 	struct list_head 	dev_base_head;
// 	struct hlist_head 	*dev_name_head;
// 	struct hlist_head	*dev_index_head;
// 	unsigned int		dev_base_seq;	/* protected by rtnl_mutex */
// 	int			ifindex;
// 	unsigned int		dev_unreg_count;

// 	/* core fib_rules */
// 	struct list_head	rules_ops;

// 	struct list_head	fib_notifier_ops;  /* protected by net_mutex */

// 	struct net_device       *loopback_dev;          /* The loopback */
// 	struct netns_core	core;
// 	struct netns_mib	mib;
// 	struct netns_packet	packet;
// 	struct netns_unix	unx;
// 	struct netns_ipv4	ipv4;
// #if IS_ENABLED(CONFIG_IPV6)
// 	struct netns_ipv6	ipv6;
// #endif
// #if IS_ENABLED(CONFIG_IEEE802154_6LOWPAN)
// 	struct netns_ieee802154_lowpan	ieee802154_lowpan;
// #endif
// #if defined(CONFIG_IP_SCTP) || defined(CONFIG_IP_SCTP_MODULE)
// 	struct netns_sctp	sctp;
// #endif
// #if defined(CONFIG_IP_DCCP) || defined(CONFIG_IP_DCCP_MODULE)
// 	struct netns_dccp	dccp;
// #endif
// #ifdef CONFIG_NETFILTER
// 	struct netns_nf		nf;
// 	struct netns_xt		xt;
// #if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
// 	struct netns_ct		ct;
// #endif
// #if defined(CONFIG_NF_TABLES) || defined(CONFIG_NF_TABLES_MODULE)
// 	struct netns_nftables	nft;
// #endif
// #if IS_ENABLED(CONFIG_NF_DEFRAG_IPV6)
// 	struct netns_nf_frag	nf_frag;
// 	struct ctl_table_header *nf_frag_frags_hdr;
// #endif
// 	struct sock		*nfnl;
// 	struct sock		*nfnl_stash;
// #if IS_ENABLED(CONFIG_NETFILTER_NETLINK_ACCT)
// 	struct list_head        nfnl_acct_list;
// #endif
// #if IS_ENABLED(CONFIG_NF_CT_NETLINK_TIMEOUT)
// 	struct list_head	nfct_timeout_list;
// #endif
// #endif
// #ifdef CONFIG_WEXT_CORE
// 	struct sk_buff_head	wext_nlevents;
// #endif
// 	struct net_generic __rcu	*gen;

// 	/* Note : following structs are cache line aligned */
// #ifdef CONFIG_XFRM
// 	struct netns_xfrm	xfrm;
// #endif
// #if IS_ENABLED(CONFIG_IP_VS)
// 	struct netns_ipvs	*ipvs;
// #endif
// #if IS_ENABLED(CONFIG_MPLS)
// 	struct netns_mpls	mpls;
// #endif
// #if IS_ENABLED(CONFIG_CAN)
// 	struct netns_can	can;
// #endif
// 	struct sock		*diag_nlsk;
// 	atomic_t		fnhe_genid;
// } ;

struct cgroup_namespace {
	refcount_t		count;
	struct ns_common	ns;
	struct user_namespace	*user_ns;
	struct ucounts		*ucounts;
	char css_set[256];
};

void tee_entry_get_container_config(struct thread_smc_args *args);
void tee_entry_update_container_region(struct thread_smc_args *args);
void tee_entry_get_nsproxy(struct thread_smc_args *args);

#endif /*NAMESPACES_H*/