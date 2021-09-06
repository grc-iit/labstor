struct pkg {
  char pkg_id[256];
  void (*process_rq_fn)(void *request);
  void* (*get_ops)(void);
  int req_size;
};

struct posix_ops {
    int (*open)(const char *pathname, int flags, mode_t mode);
    ssize_t (*read)(int fd, void *buf, ssize_t count);
    ssize_t (*write)(int fd, void *buf, ssize_t count);
    off_t (*lseek)(int fd, off_t offset, int whence);
    int (*close)(int fd);
};

struct posix_open_request {
    char *path;
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
        struct posix_open_request open;
        struct posix_io_request io;
    };
};

void process_posix_rqs(struct posix_io_request *rq) {
  struct pkg *fs_pkg;
  struct posix_ops *ops;
  switch(rq->op) {
    case POSIX_OPEN: {
      namespace_.find(rq->open.path, &pkg, nullptr);
      ops = pkg->get_ops();
      queue_manager.allocate();
      ops->open(rq->open.path, rq->open.flags, rq->open.mode);
      break;
    }
    case POSIX_OPEN_COMPLETE {
      break;
    }
  }
}
