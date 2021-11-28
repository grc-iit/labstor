
struct labstor_module {
  char module_id[256];
  void (*process_rq_fn)(void *request, struct credentials *cred);
  void* (*get_ops)(void);
};

struct posix_ops {
    void (*mount)(const char *dev_path, const char *path);
    int (*open)(const char *pathname, int flags, mode_t mode);
    ssize_t (*read)(int fd, void *buf, ssize_t count);
    ssize_t (*write)(int fd, void *buf, ssize_t count);
    labstor::off_t (*lseek)(int fd, labstor::off_t offset, int whence);
    int (*close)(int fd);
};

struct posix_mount_request {
    char dev_path[256];
    char path[256];
};

struct posix_open_request {
    char path[256];
    int flags;
    mode_t mode;
};

struct posix_io_request {
    int fd;
    void *buf;
    ssize_t count;
};

struct posix_request {
    int op;
    union {
        struct posix_mount_request mount;
        struct posix_open_request open;
    };
};

void generic_process_posix_rqs(struct posix_io_request *rq) {
  struct pkg *fs_pkg;
  struct posix_ops *ops;
  switch(rq->op) {
    case POSIX_MOUNT: {
      namespace_.find(rq->mount.path, &pkg, );
      ops = pkg->ops();
      ops->mount(rq->dev_path, rq->path);
    }
    case POSIX_OPEN: {
      namespace_.find(rq->open.path, &pkg, nullptr);
      ops = pkg->get_ops();
      ops->open(rq->open.path, rq->open.flags, rq->open.mode);
      break;
    }
  }
}
