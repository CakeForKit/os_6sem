# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/init.h>
# include <linux/fs.h>
# include <linux/time.h>
#include <linux/slab.h>      // For kmem_cache_free, kfree
#include <linux/slab_def.h>  // For kmem_cache related functions

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kathrine");

#define MYFS_MAGIC_NUMBER 0x13131313
#define MYFS_SLAB_NAME "myfs_slab"
#define MYFS_CACHE_SIZE 1

struct myfs_inode
{
    int i_mode;
    unsigned long i_ino;
};

struct myfs_cache_entry {
    struct myfs_inode inode;
    struct myfs_inode children[127];
};

struct myfs_sb_info {
    struct myfs_cache_entry **cache;
    int num_cache;
};

static struct kmem_cache *myfs_slab;

static void myfs_put_super(struct super_block * sb) {
    printk(KERN_INFO "+ myfs: super block destroyed\n");
}

static struct super_operations const myfs_super_ops = {
    .put_super = myfs_put_super,
    .statfs = simple_statfs,
    .drop_inode = generic_delete_inode,
};

static struct inode * myfs_make_inode(struct super_block * sb, int mode)
{
    struct inode * ret = new_inode(sb);
    if (ret) {
        inode_init_owner(&init_user_ns, ret, NULL, mode);
        ret->i_size = PAGE_SIZE;
        ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
        ret->i_ino = 1;
    }
    return ret;
}

static int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *root = NULL;
    sb->s_blocksize = PAGE_SIZE;
    sb->s_blocksize_bits = PAGE_SHIFT;
    sb->s_magic = MYFS_MAGIC_NUMBER;
    sb->s_op = &myfs_super_ops;

    root = myfs_make_inode(sb, S_IFDIR | 0755);
    if (!root) {
        printk(KERN_ERR "+ myfs: error myfs_make_inode\n");
        return -ENOMEM;
    }
    root->i_op = &simple_dir_inode_operations;
    root->i_fop = &simple_dir_operations;

    sb->s_root = d_make_root(root);
    if (!sb->s_root) {
        printk(KERN_ERR "+ myfs: error d_make_root\n");
        iput(root);
        return -ENOMEM;
    }

    printk(KERN_INFO "+ myfs: vfs root created\n");
    return 0;
}

static void kill_myfs(struct super_block *sb) {
    struct myfs_sb_info *sbi = sb->s_fs_info;
    int i;
    for (i = 0; i < sbi->num_cache; i++) {
        struct myfs_cache_entry *entry = sbi->cache[i];
        if (entry) {
            kmem_cache_free(myfs_slab, entry);
        }
    }
    kfree(sbi->cache);
    sbi->cache = NULL;
    sbi->num_cache = 0;
    kill_block_super(sb);
}

static struct dentry *myfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data) {
    int i;
    struct dentry *root = mount_bdev(fs_type, flags, dev_name, data, myfs_fill_super);
    if (IS_ERR(root)) {
        printk(KERN_ERR "+ myfs: mount_bdev failed\n");
        return root;
    }
    struct super_block *sb = root->d_sb;
    struct myfs_sb_info *sbi = kmalloc(sizeof(struct myfs_sb_info), GFP_KERNEL);
    if (!sbi) {
        printk(KERN_ERR "+ myfs: kmalloc failed\n");
        kill_myfs(sb);
        return ERR_PTR(-ENOMEM);
    }
    memset(sbi, 0, sizeof(struct myfs_sb_info));

    printk(KERN_INFO "+ myfs: mount_bdev succeeded\n");

    sbi->num_cache = MYFS_CACHE_SIZE;
    sbi->cache = kcalloc(sbi->num_cache, sizeof(struct myfs_cache_entry*), GFP_KERNEL);
    if (!sbi->cache) {
        printk(KERN_ERR "+ myfs: cache allocation failed\n");
        kill_myfs(sb);
        return ERR_PTR(-ENOMEM);
    }

    for (i = 0; i < sbi->num_cache; i++) {
        struct myfs_cache_entry *entry = kmem_cache_alloc(myfs_slab, GFP_KERNEL);
        if (!entry) {
            printk(KERN_ERR "+ myfs: slab allocation failed for entry %d\n", i);
            kill_myfs(sb);
            return ERR_PTR(-ENOMEM);
        }

        entry->inode.i_mode = S_IFREG | 0644;
        entry->inode.i_ino = i + 1; 
        sbi->cache[i] = entry;
    }

    sb->s_fs_info = sbi;

    return root;
}


struct file_system_type myfs_type = {
    .owner =    THIS_MODULE,
    .name =     "myfs",
    .mount =    myfs_mount,
    .kill_sb =  kill_myfs,
    .fs_flags = FS_REQUIRES_DEV,
};

static int __init myfs_init(void) {
    int ret;
    printk(KERN_INFO "+ myfs: init\n");
    
    myfs_slab = kmem_cache_create(MYFS_SLAB_NAME, 
        sizeof(struct myfs_cache_entry),
        0, 
        0, //SLAB_HWCACHE_ALIGN | SLAB_PANIC | SLAB_ACCOUNT,
        NULL);
    
    if (!myfs_slab) {
        printk(KERN_ERR "+ myfs: kmem_cache_create failed\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "+ myfs: slab cache created\n");

    ret = register_filesystem(&myfs_type);
    if (ret) {
        printk(KERN_ERR "+ myfs: register_filesystem failed\n");
        return ret;
    }
    printk(KERN_INFO "+ myfs: register_filesystem succeeded\n");
    return 0;
}

static void __exit myfs_exit(void) {
    int ret;
    printk(KERN_INFO "+ myfs: exit\n");
    ret = unregister_filesystem(&myfs_type);
    if (ret) {
        printk(KERN_ERR "+ myfs: unregister_filesystem failed\n");
        return;
    }
    printk(KERN_INFO "+ myfs: unregister_filesystem succeeded\n");
}

module_init(myfs_init);
module_exit(myfs_exit);