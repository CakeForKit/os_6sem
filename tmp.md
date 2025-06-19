# fs
```C
/ include / linux / fs_struct.h
struct fs_struct {
	int users;
	spinlock_t lock;
	seqcount_spinlock_t seq;
	int umask;
	int in_exec;
	struct path root, pwd;
} __randomize_layout;

/ include / linux / path.h
struct path {
	struct vfsmount *mnt;
	struct dentry *dentry;
} __randomize_layout;

/ include / linux / mount.h
struct vfsmount {
	struct dentry *mnt_root;	/* root of the mounted tree */
	struct super_block *mnt_sb;	/* pointer to superblock */
	int mnt_flags;
	struct user_namespace *mnt_userns;
} __randomize_layout;

/ include / linux / fdtable.h
/* Open file table structure */
struct files_struct {
  /* read mostly part */
	atomic_t count;
	bool resize_in_progress;
	wait_queue_head_t resize_wait;

	struct fdtable __rcu *fdt;
	struct fdtable fdtab;
  /* written part on a separate cache line in SMP */
	spinlock_t file_lock ____cacheline_aligned_in_smp;
	unsigned int next_fd;
	unsigned long close_on_exec_init[1];
	unsigned long open_fds_init[1];
	unsigned long full_fds_bits_init[1];
	struct file __rcu * fd_array[NR_OPEN_DEFAULT];
};

struct fdtable {
	unsigned int max_fds;
	struct file __rcu **fd;      /* current fd array */
	unsigned long *close_on_exec;
	unsigned long *open_fds;
	unsigned long *full_fds_bits;
	struct rcu_head rcu;
};

<stdio.h>
FILE *fopen( const char *filename, const char *mode );

/* /usr/include/x86_64-linux-gnu/bits/types/FILE.h */
struct _IO_FILE;
typedef struct _IO_FILE FILE;

/* /usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h */
struct _IO_FILE {
    int _flags;		/* High-order word is _IO_MAGIC; rest is flags. */
    /* The following pointers correspond to the C++ streambuf protocol. */
    char *_IO_read_ptr;	/* Current read pointer */
    char *_IO_read_end;	/* End of get area. */
    char *_IO_read_base;	/* Start of putback+get area. */
    char *_IO_write_base;	/* Start of put area. */
    char *_IO_write_ptr;	/* Current put pointer. */
    char *_IO_write_end;	/* End of put area. */
    char *_IO_buf_base;	/* Start of reserve area. */
    char *_IO_buf_end;	/* End of reserve area. */
    /* The following fields are used to support backing up and undo. */
    char *_IO_save_base; /* Pointer to start of non-current get area. */
    char *_IO_backup_base;  /* Pointer to first valid character of backup area */
    char *_IO_save_end; /* Pointer to end of non-current get area. */
    struct _IO_marker *_markers;
    struct _IO_FILE *_chain;

    int _fileno;    // 
    int _flags2;
    __off_t _old_offset; /* This used to be _offset but it's too small.  */
    /* 1+column number of pbase(); 0 is unknown. */
    unsigned short _cur_column;
    signed char _vtable_offset;
    char _shortbuf[1];
    _IO_lock_t *_lock;
};
```


# struct file 
```C
struct file {
	union {
		struct llist_node	fu_llist;
		struct rcu_head 	fu_rcuhead;
	} f_u;
	struct path		f_path;
	struct inode		*f_inode;	/* cached value */
	const struct file_operations	*f_op;

	/*
	 * Protects f_ep, f_flags.
	 * Must not be taken from IRQ context.
	 */
	spinlock_t		f_lock;
	enum rw_hint		f_write_hint;
	atomic_long_t		f_count;
	unsigned int 		f_flags;
	fmode_t			f_mode;
	struct mutex		f_pos_lock;
	loff_t			f_pos;
	struct fown_struct	f_owner;
	const struct cred	*f_cred;
	struct file_ra_state	f_ra;

	u64			f_version;
#ifdef CONFIG_SECURITY
	void			*f_security;
#endif
	/* needed for tty driver, and maybe others */
	void			*private_data;

#ifdef CONFIG_EPOLL
	/* Used by fs/eventpoll.c to link all the hooks to this file */
	struct hlist_head	*f_ep;
#endif /* #ifdef CONFIG_EPOLL */
	struct address_space	*f_mapping;
	errseq_t		f_wb_err;
	errseq_t		f_sb_err; /* for syncfs */
} __randomize_layout

struct file_operations {
	struct module *owner;
	loff_t (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
	ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
	int (*iopoll)(struct kiocb *kiocb, struct io_comp_batch *,
			unsigned int flags);
	int (*iterate) (struct file *, struct dir_context *);
	int (*iterate_shared) (struct file *, struct dir_context *);
	__poll_t (*poll) (struct file *, struct poll_table_struct *);
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
	int (*mmap) (struct file *, struct vm_area_struct *);
	unsigned long mmap_supported_flags;
	int (*open) (struct inode *, struct file *);
	int (*flush) (struct file *, fl_owner_t id);
	int (*release) (struct inode *, struct file *);
	int (*fsync) (struct file *, loff_t, loff_t, int datasync);
	int (*fasync) (int, struct file *, int);
	int (*lock) (struct file *, int, struct file_lock *);
	ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
	unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
	int (*check_flags)(int);
	int (*flock) (struct file *, int, struct file_lock *);
	ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
	ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
	int (*setlease)(struct file *, long, struct file_lock **, void **);
	long (*fallocate)(struct file *file, int mode, loff_t offset,
			  loff_t len);
	void (*show_fdinfo)(struct seq_file *m, struct file *f);
#ifndef CONFIG_MMU
	unsigned (*mmap_capabilities)(struct file *);
#endif
	ssize_t (*copy_file_range)(struct file *, loff_t, struct file *,
			loff_t, size_t, unsigned int);
	loff_t (*remap_file_range)(struct file *file_in, loff_t pos_in,
				   struct file *file_out, loff_t pos_out,
				   loff_t len, unsigned int remap_flags);
	int (*fadvise)(struct file *, loff_t, loff_t, int);
} __randomize_layout;
```

# socket
```C
asmlinkage long sys_socketcall(int call, unsigned long *args) {
    unsigned long a[AUDITSC_ARGS];
	unsigned long a0, a1; ...
    if (copy_from_user(a, args, len))
		return -EFAULT;
    ...
	switch (call) {
	case SYS_SOCKET:
		err = __sys_socket(a0, a1, a[2]); break;
	case SYS_BIND:
		err = __sys_bind(a0, (struct sockaddr __user *)a1, a[2]); break;
...}

int __sys_socket(int family, int type, int protocol) {
	int retval;
	struct socket *sock;
	...
	retval = sock_create(family, type, protocol, &sock);
	if (retval < 0) return retval;
	return sock_map_fd(sock, flags & (O_CLOEXEC | O_NONBLOCK));
}

SYSCALL_DEFINE2(socketcall, int, call, unsigned long __user *, args) {
	unsigned long a[AUDITSC_ARGS];
	unsigned long a0, a1;
	int err;
	unsigned int len;
	if (call < 1 || call > SYS_SENDMMSG) return -EINVAL;
	call = array_index_nospec(call, SYS_SENDMMSG + 1);
	len = nargs[call];
	if (len > sizeof(a)) return -EINVAL;
	/* copy_from_user should be SMP safe. */
	if (copy_from_user(a, args, len)) return -EFAULT;
	err = audit_socketcall(nargs[call] / sizeof(unsigned long), a);
	if (err) return err;
	a0 = a[0];
	a1 = a[1];
	switch (call) {
	case SYS_SOCKET:
		err = __sys_socket(a0, a1, a[2]); break;
	case SYS_BIND:
		err = __sys_bind(a0, (struct sockaddr __user *)a1, a[2]); break;
	case SYS_CONNECT:
		err = __sys_connect(a0, (struct sockaddr __user *)a1, a[2]); break;
	...
	default:
		err = -EINVAL; break;
	}
	return err;
}

/*  struct socket - general BSD socket
 *  @state: socket state (%SS_CONNECTED, etc)
 *  @type: socket type (%SOCK_STREAM, etc)
 *  @flags: socket flags (%SOCK_NOSPACE, etc)
 *  @ops: protocol specific socket operations
 *  @file: File back pointer for gc
 *  @sk: internal networking protocol agnostic socket representation
 *  @wq: wait queue for several uses */
struct socket {
	socket_state		state;
	short			type;
	unsigned long		flags;
	struct file		*file;
	struct sock		*sk;
	const struct proto_ops	*ops;
	struct socket_wq	wq;
};
typedef enum {
	SS_FREE = 0,			/* not allocated		*/
	SS_UNCONNECTED,			/* unconnected to any socket	*/
	SS_CONNECTING,			/* in process of connecting	*/
	SS_CONNECTED,			/* connected to socket		*/
	SS_DISCONNECTING		/* in process of disconnecting	*/
} socket_state;

struct sockaddr {
	sa_family_t	sa_family;	/* address family, AF_xxx	*/
	char		sa_data[14];	/* 14 bytes of protocol address	*/
};

/ include / uapi / linux / in.h
/* Structure describing an Internet (IP) socket address. */
struct sockaddr_in {
  __kernel_sa_family_t	sin_family;	/* Address family		*/
  __be16		sin_port;	/* Port number			*/
  struct in_addr	sin_addr;	/* Internet address		*/
  /* Pad to size of `struct sockaddr'. */
  unsigned char		__pad[__SOCK_SIZE__ - sizeof(short int) -
			sizeof(unsigned short int) - sizeof(struct in_addr)];
};
```

```bash
touch image # образ диска
mkdir dir	# точка монтирования
sudo mount -o loop -t myfs ./image ./dir
sudo umount ./dir
```

# super_block
```C
/ fs / super.c
/**
 *	alloc_super	-	create new superblock
 *	@type:	filesystem type superblock should belong to
 *	@flags: the mount flags
 *	@user_ns: User namespace for the super_block
 *
 *	Allocates and initializes a new &struct super_block.  alloc_super()
 *	returns a pointer new superblock or %NULL if allocation had failed.
 */
static struct super_block *alloc_super(struct file_system_type *type, int flags,
				       struct user_namespace *user_ns) {
	struct super_block *s = kzalloc(sizeof(struct super_block), GFP_KERNEL);
	static const struct super_operations default_op;
	...
	INIT_LIST_HEAD(&s->s_mounts);
    ...
    INIT_LIST_HEAD(&s->s_inodes);
    ...
}

/ include / linux / fs.h
struct super_block {
	struct list_head	s_list;		/* Keep this first */
	dev_t			s_dev;		/* search index; _not_ kdev_t */
	unsigned char		s_blocksize_bits;
	unsigned long		s_blocksize;
	loff_t			s_maxbytes;	/* Max file size */
	struct file_system_type	*s_type;
	const struct super_operations	*s_op;
	const struct dquot_operations	*dq_op;
	const struct quotactl_ops	*s_qcop;
	const struct export_operations *s_export_op;
	unsigned long		s_flags;
	unsigned long		s_iflags;	/* internal SB_I_* flags */
	unsigned long		s_magic;
	struct dentry		*s_root;
	struct rw_semaphore	s_umount;
	int			s_count;
	atomic_t		s_active;
#ifdef CONFIG_SECURITY
	void                    *s_security;
#endif
	const struct xattr_handler **s_xattr;
#ifdef CONFIG_FS_ENCRYPTION
	const struct fscrypt_operations	*s_cop;
	struct key		*s_master_keys; /* master crypto keys in use */
#endif
#ifdef CONFIG_FS_VERITY
	const struct fsverity_operations *s_vop;
#endif
#ifdef CONFIG_UNICODE
	struct unicode_map *s_encoding;
	__u16 s_encoding_flags;
#endif
	struct hlist_bl_head	s_roots;	/* alternate root dentries for NFS */
	struct list_head	s_mounts;	/* list of mounts; _not_ for fs use */
	struct block_device	*s_bdev;
	struct backing_dev_info *s_bdi;
	struct mtd_info		*s_mtd;
	struct hlist_node	s_instances;
	unsigned int		s_quota_types;	/* Bitmask of supported quota types */
	struct quota_info	s_dquot;	/* Diskquota specific options */

	struct sb_writers	s_writers;

	/* Keep s_fs_info, s_time_gran, s_fsnotify_mask, and
	 * s_fsnotify_marks together for cache efficiency. They are frequently
	 * accessed and rarely modified.  */
	void			*s_fs_info;	/* Filesystem private info */

	/* Granularity of c/m/atime in ns (cannot be worse than a second) */
	u32			s_time_gran;
	/* Time limits for c/m/atime in seconds */
	time64_t		   s_time_min;
	time64_t		   s_time_max;
#ifdef CONFIG_FSNOTIFY
	__u32			s_fsnotify_mask;
	struct fsnotify_mark_connector __rcu	*s_fsnotify_marks;
#endif

	char			s_id[32];	/* Informational name */
	uuid_t			s_uuid;		/* UUID */

	unsigned int		s_max_links;
	fmode_t			s_mode;

	/* The next field is for VFS *only*. No filesystems have any business
	 * even looking at it. You had been warned. */
	struct mutex s_vfs_rename_mutex;	/* Kludge */

	/* Filesystem subtype.  If non-empty the filesystem type field
	 * in /proc/mounts will be "type.subtype" */
	const char *s_subtype;

	const struct dentry_operations *s_d_op; /* default d_op for dentries */

	/* Saved pool identifier for cleancache (-1 means none) */
	int cleancache_poolid;

	struct shrinker s_shrink;	/* per-sb shrinker handle */

	/* Number of inodes with nlink == 0 but still referenced */
	atomic_long_t s_remove_count;

	/* Number of inode/mount/sb objects that are being watched, note that
	 * inodes objects are currently double-accounted. */
	atomic_long_t s_fsnotify_connectors;

	/* Being remounted read-only */
	int s_readonly_remount;

	/* per-sb errseq_t for reporting writeback errors via syncfs */
	errseq_t s_wb_err;

	/* AIO completions deferred from interrupt context */
	struct workqueue_struct *s_dio_done_wq;
	struct hlist_head s_pins;

	/* Owning user namespace and default context in which to
	 * interpret filesystem uids, gids, quotas, device nodes,
	 * xattrs and security labels. */
	struct user_namespace *s_user_ns;

	/* The list_lru structure is essentially just a pointer to a table
	 * of per-node lru lists, each of which has its own spinlock.
	 * There is no need to put them into separate cachelines. */
	struct list_lru		s_dentry_lru;
	struct list_lru		s_inode_lru;
	struct rcu_head		rcu;
	struct work_struct	destroy_work;
	struct mutex		s_sync_lock;	/* sync serialisation lock */

	/* Indicates how deep in a filesystem stack this SB is */
	int s_stack_depth;

	/* s_inode_list_lock protects s_inodes */
	spinlock_t		s_inode_list_lock ____cacheline_aligned_in_smp;
	struct list_head	s_inodes;	/* all inodes */

	spinlock_t		s_inode_wblist_lock;
	struct list_head	s_inodes_wb;	/* writeback inodes */
} __randomize_layout;

struct super_operations {
   	struct inode *(*alloc_inode)(struct super_block *sb);
	void (*destroy_inode)(struct inode *);
	void (*free_inode)(struct inode *);

   	void (*dirty_inode) (struct inode *, int flags);
	int (*write_inode) (struct inode *, struct writeback_control *wbc);
	int (*drop_inode) (struct inode *);
	void (*evict_inode) (struct inode *);
	void (*put_super) (struct super_block *);
	int (*sync_fs)(struct super_block *sb, int wait);
	int (*freeze_super) (struct super_block *);
	int (*freeze_fs) (struct super_block *);
	int (*thaw_super) (struct super_block *);
	int (*unfreeze_fs) (struct super_block *);
	int (*statfs) (struct dentry *, struct kstatfs *);
	int (*remount_fs) (struct super_block *, int *, char *);
	void (*umount_begin) (struct super_block *);

	int (*show_options)(struct seq_file *, struct dentry *);
	int (*show_devname)(struct seq_file *, struct dentry *);
	int (*show_path)(struct seq_file *, struct dentry *);
	int (*show_stats)(struct seq_file *, struct dentry *);
#ifdef CONFIG_QUOTA
	ssize_t (*quota_read)(struct super_block *, int, char *, size_t, loff_t);
	ssize_t (*quota_write)(struct super_block *, int, const char *, size_t, loff_t);
	struct dquot **(*get_dquots)(struct inode *);
#endif
	long (*nr_cached_objects)(struct super_block *,
				  struct shrink_control *);
	long (*free_cached_objects)(struct super_block *,
				    struct shrink_control *);
};
```

# struct inode
```C
struct inode {
	umode_t			i_mode;
	unsigned short		i_opflags;
	kuid_t			i_uid;
	kgid_t			i_gid;
	unsigned int		i_flags;
#ifdef CONFIG_FS_POSIX_ACL
	struct posix_acl	*i_acl;
	struct posix_acl	*i_default_acl;
#endif
	const struct inode_operations	*i_op;
	struct super_block	*i_sb;
	struct address_space	*i_mapping;
#ifdef CONFIG_SECURITY
	void			*i_security;
#endif
	/* Stat data, not accessed from path walking */
	unsigned long		i_ino;
	/*
	 * Filesystems may only read i_nlink directly.  They shall use the
	 * following functions for modification:
	 *
	 *    (set|clear|inc|drop)_nlink
	 *    inode_(inc|dec)_link_count
	 */
	union {
		const unsigned int i_nlink;
		unsigned int __i_nlink;
	};
	dev_t			i_rdev;
	loff_t			i_size;
	struct timespec64	i_atime;
	struct timespec64	i_mtime;
	struct timespec64	i_ctime;
	spinlock_t		i_lock;	/* i_blocks, i_bytes, maybe i_size */
	unsigned short          i_bytes;
	u8			i_blkbits;
	u8			i_write_hint;
	blkcnt_t		i_blocks;
#ifdef __NEED_I_SIZE_ORDERED
	seqcount_t		i_size_seqcount;
#endif
	/* Misc */
	unsigned long		i_state;
	struct rw_semaphore	i_rwsem;

	unsigned long		dirtied_when;	/* jiffies of first dirtying */
	unsigned long		dirtied_time_when;

	struct hlist_node	i_hash;
	struct list_head	i_io_list;	/* backing dev IO list */
#ifdef CONFIG_CGROUP_WRITEBACK
	struct bdi_writeback	*i_wb;		/* the associated cgroup wb */

	/* foreign inode detection, see wbc_detach_inode() */
	int			i_wb_frn_winner;
	u16			i_wb_frn_avg_time;
	u16			i_wb_frn_history;
#endif
	struct list_head	i_lru;		/* inode LRU list */
	struct list_head	i_sb_list;
	struct list_head	i_wb_list;	/* backing dev writeback list */
	union {
		struct hlist_head	i_dentry;
		struct rcu_head		i_rcu;
	};
	atomic64_t		i_version;
	atomic64_t		i_sequence; /* see futex */
	atomic_t		i_count;
	atomic_t		i_dio_count;
	atomic_t		i_writecount;
#if defined(CONFIG_IMA) || defined(CONFIG_FILE_LOCKING)
	atomic_t		i_readcount; /* struct files open RO */
#endif
	union {
		const struct file_operations	*i_fop;	/* former ->i_op->default_file_ops */
		void (*free_inode)(struct inode *);
	};
	struct file_lock_context	*i_flctx;
	struct address_space	i_data;
	struct list_head	i_devices;
	union {
		struct pipe_inode_info	*i_pipe;
		struct cdev		*i_cdev;
		char			*i_link;
		unsigned		i_dir_seq;
	};
	__u32			i_generation;
#ifdef CONFIG_FSNOTIFY
	__u32			i_fsnotify_mask; /* all events this inode cares about */
	struct fsnotify_mark_connector __rcu	*i_fsnotify_marks;
#endif
#ifdef CONFIG_FS_ENCRYPTION
	struct fscrypt_info	*i_crypt_info;
#endif
#ifdef CONFIG_FS_VERITY
	struct fsverity_info	*i_verity_info;
#endif
	void			*i_private; /* fs or device private pointer */
} __randomize_layout;


struct inode_operations {
	struct dentry * (*lookup) (struct inode *,struct dentry *, unsigned int);
	int (*create) (struct user_namespace *, struct inode *,struct dentry *, umode_t, bool);
	int (*link) (struct dentry *,struct inode *,struct dentry *);
	int (*symlink) (struct user_namespace *, struct inode *,struct dentry *, const char *);
	int (*mkdir) (struct user_namespace *, struct inode *,struct dentry *, umode_t);
	...
} ____cacheline_aligned;
```

# struct dentry 
```C
struct dentry {
    ...
    struct hlist_bl_node d_hash;	/* lookup hash list */
	struct dentry *d_parent;	/* parent directory */
    struct qstr d_name;
	struct inode *d_inode;		
    unsigned char d_iname[DNAME_INLINE_LEN];	/* small names */
	struct lockref d_lockref;	/* per-dentry lock and refcount */
	const struct dentry_operations *d_op; 
	struct super_block *d_sb;	
	union {
		struct list_head d_lru;		/* LRU list */
		wait_queue_head_t *d_wait;	/* in-lookup ones only */
	};
	struct list_head d_child;	/* child of parent list */
	struct list_head d_subdirs;	/* our children */
	/*
	 * d_alias and d_rcu can share memory
	 */
	union {
		struct hlist_node d_alias;	/* inode alias list */
		struct hlist_bl_node d_in_lookup_hash;	/* only for in-lookup ones */
	 	struct rcu_head d_rcu;
	} d_u;
} __randomize_layout;

struct dentry_operations {
	int (*d_revalidate)(struct dentry *, unsigned int);
	int (*d_hash)(const struct dentry *, struct qstr *);
	int (*d_compare)(const struct dentry *, unsigned int, const char *, const struct qstr *);
	int (*d_delete)(const struct dentry *);
	void (*d_release)(struct dentry *);
	void (*d_iput)(struct dentry *, struct inode *);
	...
} ____cacheline_aligned;
```



# struct file_system_type
```C
struct file_system_type {
	const char *name;
	int fs_flags;
#define FS_REQUIRES_DEV		1 
#define FS_BINARY_MOUNTDATA	2
#define FS_HAS_SUBTYPE		4
#define FS_USERNS_MOUNT		8	/* Can be mounted by userns root */
#define FS_DISALLOW_NOTIFY_PERM	16	/* Disable fanotify permission events */
#define FS_ALLOW_IDMAP         32      /* FS has been updated to handle vfs idmappings. */
#define FS_RENAME_DOES_D_MOVE	32768	/* FS will handle d_move() during rename() internally. */
	int (*init_fs_context)(struct fs_context *);
	const struct fs_parameter_spec *parameters;
	struct dentry *(*mount) (struct file_system_type *, int, const char *, void *); 
	void (*kill_sb) (struct super_block *); 
	struct module *owner;	 
	struct file_system_type * next; 
	struct hlist_head fs_supers;	 
	struct lock_class_key s_lock_key;
	struct lock_class_key s_umount_key;
	struct lock_class_key s_vfs_rename_key;
	struct lock_class_key s_writers_key[SB_FREEZE_LEVELS];
	struct lock_class_key i_lock_key;
	struct lock_class_key i_mutex_key;
	struct lock_class_key invalidate_lock_key;
	struct lock_class_key i_mutex_dir_key;
};

extern struct dentry *mount_bdev(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data,
	int (*fill_super)(struct super_block *, void *, int));

extern struct dentry *mount_single(struct file_system_type *fs_type,
	int flags, void *data,
	int (*fill_super)(struct super_block *, void *, int));

extern struct dentry *mount_nodev(struct file_system_type *fs_type,
	int flags, void *data,
	int (*fill_super)(struct super_block *, void *, int));

extern struct dentry *mount_subtree(struct vfsmount *mnt, const char *path);
```


# struct proc_dir_entry
```C
/ fs / proc / internal.h
struct proc_dir_entry {
	/*
	 * number of callers into module in progress;
	 * negative -> it's going away RSN
	 */
	atomic_t in_use;
	refcount_t refcnt;
	struct list_head pde_openers;	/* who did ->open, but not ->release */
	/* protects ->pde_openers and all struct pde_opener instances */
	spinlock_t pde_unload_lock;
	struct completion *pde_unload_completion;
	const struct inode_operations *proc_iops;
	union {
		const struct proc_ops *proc_ops;
		const struct file_operations *proc_dir_ops;
	};
	const struct dentry_operations *proc_dops;
	union {
		const struct seq_operations *seq_ops;
		int (*single_show)(struct seq_file *, void *);
	};
	proc_write_t write;
	void *data;
	unsigned int state_size;
	unsigned int low_ino;
	nlink_t nlink;
	kuid_t uid;
	kgid_t gid;
	loff_t size;
	struct proc_dir_entry *parent;
	struct rb_root subdir;
	struct rb_node subdir_node;
	char *name;
	umode_t mode;
	u8 flags;
	u8 namelen;
	char inline_name[];
} __randomize_layout;

/ include / linux / proc_fs.h
struct proc_ops {
	unsigned int proc_flags;
	int	(*proc_open)(struct inode *, struct file *);
	ssize_t	(*proc_read)(struct file *, char __user *, size_t, loff_t *);
	ssize_t (*proc_read_iter)(struct kiocb *, struct iov_iter *);
	ssize_t	(*proc_write)(struct file *, const char __user *, size_t, loff_t *);
	/* mandatory unless nonseekable_open() or equivalent is used */
	loff_t	(*proc_lseek)(struct file *, loff_t, int);
	int	(*proc_release)(struct inode *, struct file *);
	__poll_t (*proc_poll)(struct file *, struct poll_table_struct *);
	long	(*proc_ioctl)(struct file *, unsigned int, unsigned long);
#ifdef CONFIG_COMPAT
	long	(*proc_compat_ioctl)(struct file *, unsigned int, unsigned long);
#endif
	int	(*proc_mmap)(struct file *, struct vm_area_struct *);
	unsigned long (*proc_get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
} __randomize_layout;

extern struct proc_dir_entry *proc_create_data(const char *, umode_t,
					       struct proc_dir_entry *,
					       const struct proc_ops *,
					       void *);
extern void remove_proc_entry(const char *, struct proc_dir_entry *);
static inline struct proc_dir_entry *proc_symlink(const char *name,
		struct proc_dir_entry *parent,const char *dest) { return NULL;}
static inline struct proc_dir_entry *proc_mkdir(const char *name,
	struct proc_dir_entry *parent) {return NULL;}

fs/proc/generic.c
struct proc_dir_entry *proc_create(const char *name, umode_t mode,
				   struct proc_dir_entry *parent,
				   const struct proc_ops *proc_ops) {
	return proc_create_data(name, mode, parent, proc_ops, NULL);
}
EXPORT_SYMBOL(proc_create);
```

# __copy_to_user
```C
include/linux/uaccess.h
static __always_inline __must_check unsigned long
    __copy_to_user(void __user *to, const void *from, unsigned long n);
static __always_inline __must_check unsigned long
    __copy_from_user(void *to, const void __user *from, unsigned long n);
```

# struct seq_file
```C
 / include / linux / seq_file.h
struct seq_file {
	char *buf;
	size_t size;
	size_t from;
	size_t count;
	size_t pad_until;
	loff_t index;
	loff_t read_pos;
	struct mutex lock;
	const struct seq_operations *op;
	int poll_event;
	const struct file *file;
	void *private;
};

struct seq_operations {
	void * (*start) (struct seq_file *m, loff_t *pos);
	void (*stop) (struct seq_file *m, void *v);
	void * (*next) (struct seq_file *m, void *v, loff_t *pos);
	int (*show) (struct seq_file *m, void *v);
};

int seq_open(struct file *, const struct seq_operations *);
ssize_t seq_read(struct file *, char __user *, size_t, loff_t *);
ssize_t seq_read_iter(struct kiocb *iocb, struct iov_iter *iter);
loff_t seq_lseek(struct file *, loff_t, int);
int seq_release(struct inode *, struct file *);
int seq_write(struct seq_file *seq, const void *data, size_t len);
void seq_printf(struct seq_file *m, const char *fmt, ...);

/ fs / seq_file.c
/*  seq_read -	->read() method for sequential files.
 *	@file: the file to read from
 *	@buf: the buffer to read to
 *	@size: the maximum number of bytes to read
 *	@ppos: the current position in the file
 *	Ready-made ->f_op->read() */
ssize_t seq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos) {
	struct iovec iov = { .iov_base = buf, .iov_len = size};
	struct kiocb kiocb;
	struct iov_iter iter;
	ssize_t ret;
	init_sync_kiocb(&kiocb, file);
	iov_iter_init(&iter, READ, &iov, 1, size);
	kiocb.ki_pos = *ppos;
	ret = seq_read_iter(&kiocb, &iter); //
	*ppos = kiocb.ki_pos;
	return ret;
}
EXPORT_SYMBOL(seq_read);

/* Ready-made ->f_op->read_iter() */
ssize_t seq_read_iter(struct kiocb *iocb, struct iov_iter *iter) {
	struct seq_file *m = iocb->ki_filp->private_data;
	size_t copied = 0;
	size_t n;
	void *p;
	int err = 0;
	if (!iov_iter_count(iter))
		return 0;

	mutex_lock(&m->lock);
	/* if request is to read from zero offset, reset iterator to first
	 * record as it might have been already advanced by previous requests */
	if (iocb->ki_pos == 0) {
		m->index = 0;
		m->count = 0;
	}
	/* Don't assume ki_pos is where we left it */
	if (unlikely(iocb->ki_pos != m->read_pos)) {
		while ((err = traverse(m, iocb->ki_pos)) == -EAGAIN);
		if (err) {
			/* With prejudice... */
			m->read_pos = 0;
			m->index = 0;
			m->count = 0;
			goto Done;
		} else {
			m->read_pos = iocb->ki_pos;
		}
	}
	/* grab buffer if we didn't have one */
	if (!m->buf) {
		m->buf = seq_buf_alloc(m->size = PAGE_SIZE);
		if (!m->buf)
			goto Enomem;
	}
	// something left in the buffer - copy it out first
	if (m->count) {
		n = copy_to_iter(m->buf + m->from, m->count, iter);
		m->count -= n;
		m->from += n;
		copied += n;
		if (m->count)	// hadn't managed to copy everything
			goto Done;
	}
	// get a non-empty record in the buffer
	m->from = 0;
	p = m->op->start(m, &m->index);
	while (1) {
		err = PTR_ERR(p);
		if (!p || IS_ERR(p))	// EOF or an error
			break;
		err = m->op->show(m, p);
		if (err < 0)		// hard error
			break;
		if (unlikely(err))	// ->show() says "skip it"
			m->count = 0;
		if (unlikely(!m->count)) { // empty record
			p = m->op->next(m, p, &m->index);
			continue;
		}
		if (!seq_has_overflowed(m)) // got it
			goto Fill;
		// need a bigger buffer
		m->op->stop(m, p);
		kvfree(m->buf);
		m->count = 0;
		m->buf = seq_buf_alloc(m->size <<= 1);
		if (!m->buf)
			goto Enomem;
		p = m->op->start(m, &m->index);
	}
	// EOF or an error
	m->op->stop(m, p);
	m->count = 0;
	goto Done;
Fill:
	// one non-empty record is in the buffer; if they want more,
	// try to fit more in, but in any case we need to advance
	// the iterator once for every record shown.
	while (1) {
		size_t offs = m->count;
		loff_t pos = m->index;

		p = m->op->next(m, p, &m->index);
		if (pos == m->index) {
			pr_info_ratelimited("buggy .next function %ps did not update position index\n",
					    m->op->next);
			m->index++;
		}
		if (!p || IS_ERR(p))	// no next record for us
			break;
		if (m->count >= iov_iter_count(iter))
			break;
		err = m->op->show(m, p);
		if (err > 0) {		// ->show() says "skip it"
			m->count = offs;
		} else if (err || seq_has_overflowed(m)) {
			m->count = offs;
			break;
		}
	}
	m->op->stop(m, p);
	n = copy_to_iter(m->buf, m->count, iter);
	copied += n;
	m->count -= n;
	m->from = n;
Done:
	if (unlikely(!copied)) {
		copied = m->count ? -EFAULT : err;
	} else {
		iocb->ki_pos += copied;
		m->read_pos += copied;
	}
	mutex_unlock(&m->lock);
	return copied;
Enomem:
	err = -ENOMEM;
	goto Done;
}
EXPORT_SYMBOL(seq_read_iter);
```

# single_open
```C
static void *single_start(struct seq_file *p, loff_t *pos) {
	return NULL + (*pos == 0);
}
static void *single_next(struct seq_file *p, void *v, loff_t *pos) {
	++*pos;
	return NULL;
}
static void single_stop(struct seq_file *p, void *v){}

int single_open(struct file *file, int (*show)(struct seq_file *, void *),
		void *data) {
	struct seq_operations *op = kmalloc(sizeof(*op), GFP_KERNEL_ACCOUNT);
	int res = -ENOMEM;

	if (op) {
		op->start = single_start;
		op->next = single_next;
		op->stop = single_stop;
		op->show = show;
		res = seq_open(file, op);	
		if (!res)
			((struct seq_file *)file->private_data)->private = data;
		else
			kfree(op);
	}
	return res;
}
EXPORT_SYMBOL(single_open);

int single_open_size(struct file *file, int (*show)(struct seq_file *, void *),
		void *data, size_t size)
{
	char *buf = seq_buf_alloc(size);
	int ret;
	if (!buf)
		return -ENOMEM;
	ret = single_open(file, show, data);
	if (ret) {
		kvfree(buf);
		return ret;
	}
	((struct seq_file *)file->private_data)->buf = buf;
	((struct seq_file *)file->private_data)->size = size;
	return 0;
}
EXPORT_SYMBOL(single_open_size);
```

# dev
```C
/ include / linux / types.h
typedef u32 __kernel_dev_t;
typedef __kernel_dev_t		dev_t;

/ include / linux / device.h
struct device {
	struct kobject kobj;
	struct device		*parent;
	struct device_private	*p;
	const char		*init_name; /* initial name of the device */
	const struct device_type *type;

	const struct bus_type	*bus;	/* type of bus device is on */
	struct device_driver *driver;	/* which driver has allocated this device */
	void		*platform_data;	/* Platform specific data, device core doesn't touch it */
	void		*driver_data;	/* Driver data, set and get with  dev_set_drvdata/dev_get_drvdata */
	struct mutex		mutex;	/* mutex to synchronize calls to its driver. */

	struct dev_links_info	links;
	struct dev_pm_info	power;
	struct dev_pm_domain	*pm_domain;

#ifdef CONFIG_ENERGY_MODEL
	struct em_perf_domain	*em_pd;
#endif
#ifdef CONFIG_PINCTRL
	struct dev_pin_info	*pins;
#endif
	struct dev_msi_info	msi;
#ifdef CONFIG_ARCH_HAS_DMA_OPS
	const struct dma_map_ops *dma_ops;
#endif
	u64		*dma_mask;	/* dma mask (if dma'able device) */
	u64		coherent_dma_mask;/* Like dma_mask, but for
					     alloc_coherent mappings as
					     not all hardware supports
					     64 bit addresses for consistent
					     allocations such descriptors. */
	u64		bus_dma_limit;	/* upstream dma constraint */
	const struct bus_dma_region *dma_range_map;
	struct device_dma_parameters *dma_parms;
	struct list_head	dma_pools;	/* dma pools (if dma'ble) */
...
#ifdef CONFIG_NUMA
	int		numa_node;	/* NUMA node this device is close to */
#endif
	dev_t			devt;	/* dev_t, creates the sysfs "dev" */
	u32			id;	/* device instance */
...};

/ include / linux / device / driver.h
struct device_driver {
	const char		*name;
	const struct bus_type	*bus;

	struct module		*owner;
	const char		*mod_name;	/* used for built-in modules */

	bool suppress_bind_attrs;	/* disables bind/unbind via sysfs */
	enum probe_type probe_type;

	const struct of_device_id	*of_match_table;
	const struct acpi_device_id	*acpi_match_table;

	int (*probe) (struct device *dev);
	void (*sync_state)(struct device *dev);
	int (*remove) (struct device *dev);
	void (*shutdown) (struct device *dev);
	int (*suspend) (struct device *dev, pm_message_t state);
	int (*resume) (struct device *dev);
	const struct attribute_group **groups;
	const struct attribute_group **dev_groups;

	const struct dev_pm_ops *pm;
	void (*coredump) (struct device *dev);

	struct driver_private *p;
};

struct usb_driver {
	const char *name;

	int (*probe) (struct usb_interface *intf,
		      const struct usb_device_id *id);

	void (*disconnect) (struct usb_interface *intf);

	int (*unlocked_ioctl) (struct usb_interface *intf, unsigned int code,
			void *buf);

	int (*suspend) (struct usb_interface *intf, pm_message_t message);
	int (*resume) (struct usb_interface *intf);
	int (*reset_resume)(struct usb_interface *intf);

	int (*pre_reset)(struct usb_interface *intf);
	int (*post_reset)(struct usb_interface *intf);

	void (*shutdown)(struct usb_interface *intf);

	const struct usb_device_id *id_table;
	const struct attribute_group **dev_groups;

	struct usb_dynids dynids;
	struct device_driver driver;
	unsigned int no_dynamic_id:1;
	unsigned int supports_autosuspend:1;
	unsigned int disable_hub_initiated_lpm:1;
	unsigned int soft_unbind:1;
};

struct usb_device_driver {
	const char *name;

	bool (*match) (struct usb_device *udev);
	int (*probe) (struct usb_device *udev);
	void (*disconnect) (struct usb_device *udev);

	int (*suspend) (struct usb_device *udev, pm_message_t message);
	int (*resume) (struct usb_device *udev, pm_message_t message);

	int (*choose_configuration) (struct usb_device *udev);

	const struct attribute_group **dev_groups;
	struct device_driver driver;
	const struct usb_device_id *id_table;
	unsigned int supports_autosuspend:1;
	unsigned int generic_subclass:1;
};
```

# workqueu
```C
struct work_struct {
	atomic_long_t data;
	struct list_head entry;
	work_func_t func;
#ifdef CONFIG_LOCKDEP
	struct lockdep_map lockdep_map;
#endif
};

/*
 * The externally visible workqueue.  It relays the issued work items to
 * the appropriate worker_pool through its pool_workqueues.
 */
struct workqueue_struct {
	struct list_head	pwqs;		/* WR: all pwqs of this wq */
	struct list_head	list;		/* PR: list of all workqueues */

	struct mutex		mutex;		/* protects this wq */
	int			work_color;	/* WQ: current work color */
	int			flush_color;	/* WQ: current flush color */
	atomic_t		nr_pwqs_to_flush; /* flush in progress */
	struct wq_flusher	*first_flusher;	/* WQ: first flusher */
	struct list_head	flusher_queue;	/* WQ: flush waiters */
	struct list_head	flusher_overflow; /* WQ: flush overflow list */

	struct list_head	maydays;	/* MD: pwqs requesting rescue */
	struct worker		*rescuer;	/* MD: rescue worker */

	int			nr_drainers;	/* WQ: drain in progress */
	int			saved_max_active; /* WQ: saved pwq max_active */

	struct workqueue_attrs	*unbound_attrs;	/* PW: only for unbound wqs */
	struct pool_workqueue	*dfl_pwq;	/* PW: only for unbound wqs */

#ifdef CONFIG_SYSFS
	struct wq_device	*wq_dev;	/* I: for sysfs interface */
#endif
#ifdef CONFIG_LOCKDEP
	char			*lock_name;
	struct lock_class_key	key;
	struct lockdep_map	lockdep_map;
#endif
	char			name[WQ_NAME_LEN]; /* I: workqueue name */

	/*
	 * Destruction of workqueue_struct is RCU protected to allow walking
	 * the workqueues list without grabbing wq_pool_mutex.
	 * This is used to dump all workqueues from sysrq.
	 */
	struct rcu_head		rcu;

	/* hot fields used during command issue, aligned to cacheline */
	unsigned int		flags ____cacheline_aligned; /* WQ: WQ_* flags */
	struct pool_workqueue __percpu *cpu_pwqs; /* I: per-cpu pwqs */
	struct pool_workqueue __rcu *numa_pwq_tbl[]; /* PWR: unbound pwqs indexed by node */
};
```






# labs
## fortune 
```C
static struct proc_dir_entry *dir, *file;
static char kbuf[BUF_SIZE];
static int fpid = -1;
static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *offset) {
    struct task_struct *task = pid_task(find_vpid(fpid), PIDTYPE_PID);
    int len = snprintf(kbuf, BUF_SIZE, "pid=%d\n", task->pid);
    if (copy_to_user(buf, kbuf, len)) return -EFAULT; *offset += len; return len; }
static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset) {
    char ubuf[PARSE_SIZE];
    if (copy_from_user(ubuf, buf, count)) return -EFAULT;
    if (kstrtoint(ubuf, 10, &fpid))  return -EINVAL;
    return count; }
static int my_open(struct inode *inode, struct file *file){printk}
static int my_release(struct inode *inode, struct file *file){printk}
static const struct proc_ops proc_fops = {
    .proc_open = my_open,
    .proc_release = my_release,
    .proc_read = my_read,
    .proc_write = my_write, };
static int __init fortune_init(void) {
    dir = proc_mkdir(DIRNAME, NULL); if (dir == NULL) {...}
    file = proc_create(FILENAME, 0666, dir, &proc_fops); if (file == NULL)
    return 0; }
static void __exit fortune_exit(void) {
    proc_remove(file); proc_remove(dir); }
module_init(fortune_init); module_exit(fortune_exit);
```

## seq_files
```C
static struct proc_dir_entry *dir, *file;
static int pids[SIZEPIDS] = {-1, -1, -1};
static int npids = 0;
static int cur_ind = 0;
static char fortune_buf[BUF_SIZE];
static void *seq_start(struct seq_file *m, loff_t *offset) {
    if (cur_ind == SIZEPIDS) return NULL;
    return &pids[cur_ind];
}
static int seq_show(struct seq_file *m, void *v) {
    int fpid = *(int *)v;
    struct task_struct *task = pid_task(find_vpid(fpid), PIDTYPE_PID);
    int len = snprintf(fortune_buf, BUF_SIZE,"pid=%d\nppid=%d\n" task->pid, task->parent->pid);
    if (len < 0) { return -EFAULT; }
    seq_printf(m, fortune_buf); return 0; }
static void *seq_next(struct seq_file *m, void *v, loff_t *pos) {
    cur_ind++; if (cur_ind >= SIZEPIDS) return NULL;
    return &pids[cur_ind]; }
static void seq_stop(struct seq_file *m, void *v) {printk}
const struct seq_operations seq_ops = {
    .start = seq_start,
    .next  = seq_next,
    .stop  = seq_stop,
    .show  = seq_show};
static int my_open(struct inode *inode, struct file *file) {
    return seq_open(file, &seq_ops); }
static int my_release(struct inode *inode, struct file *file) {
    return seq_release(inode, file); }
static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *offset) {
    return seq_read(filp, buf, count, offset); }
static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset) {
    char ubuf[PARSE_SIZE]; if (copy_from_user(ubuf, buf, count)) return -EFAULT; 
    if (kstrtoint(ubuf, 10, &pids[npids])) return -EINVAL;
    npids++; *offset = count; return count; }
static const struct proc_ops proc_fops = {
    .proc_open = my_open,
    .proc_release = my_release,
    .proc_read = my_read,
    .proc_write = my_write,};
--||--



ssize_t seq_read_iter(...) {
	struct seq_file *m = ...;  ...
	p = m->op->start(m, &m->index);
	while (1) { ...
		err = m->op->show(m, p); // обраотка ошибки
		if (!m->count) { // в буфере есть место
			p = m->op->next(m, p, &m->index);
			continue; }
		// буфер заполнен
		m->op->stop(m, p);
		// увеличение буфера
		p = m->op->start(m, &m->index);
	}
	// EOF or an error
	m->op->stop(m, p);
	... }
```

## single
```C

static struct proc_dir_entry *file;
static char kbuf[BUF_SIZE];
static int my_show(struct seq_file *m, void *v) {
	struct task_struct *task = pid_task(find_vpid(fpid), PIDTYPE_PID);
    int len = snprintf(kbuf, BUF_SIZE, "pid=%d\n", task->pid);
    seq_printf(m, kbuf);}
static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset) {--||--}
static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *offset) {
    return seq_read(filp, buf, count, offset);}
static int my_open(struct inode *inode, struct file *file) {
    return single_open(file, my_show, NULL);}
static int my_release(struct inode *inode, struct file *file) {
    return single_release(inode, file);}
static const struct proc_ops proc_fops = {
    .proc_open = my_open,
    .proc_release = my_release,
    .proc_read = my_read,
    .proc_write = my_write,
};
--||--
```


```C
struct sembuf start_read[] = {
    {readers_queue, 1, 0},
    {active_writer, 0, 0},
    {writers_queue, 0, 0},
    {numb_of_readers, 1, 0},
    {readers_queue, -1, 0}
};
struct sembuf stop_read[] = { {numb_of_readers, -1, 0} };
struct sembuf start_write[] = {
    {writers_queue, 1, 0},
    {numb_of_readers, 0, 0},
    {bin_sem, -1, 0},
    {active_writer, 1, 0},
    {writers_queue, -1, 0}
};
struct sembuf stop_write[] = { {active_writer, -1, 0}, {bin_sem, 1, 0} };

main()
int semid = semget(key, 5, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
if (semid == -1)
if (semctl(semid, active_writer, SETVAL, 0) == -1) {err}
--||--
if (semop(semid, start_read, 5) == -1) {...}
CR3
if (semop(semid, stop_read, 1) == -1) {...}
```



