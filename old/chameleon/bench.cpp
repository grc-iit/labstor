#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <mpi.h>
#include <string>
#include <cstring>
#include <ctime>

void log_end(int rank, int nprocs, size_t block_size, int count, double time, std::string path, char *fs_type, char *store_type, char *dss, int direct) {
    if(rank != 0) { return; }
    size_t size_per_proc = block_size*count;
    size_t agg_size = size_per_proc*nprocs;
    double bw = agg_size/time;
    double iops = agg_size/(block_size*time);
    printf("FINISHED: PATH=%s BS=%lu COUNT=%d NPROCS=%d TIME=%lf BW=%lf (B/s) THRPT=%lf (IOPS)\n", path.c_str(), block_size, count, nprocs, time, bw, iops);

    FILE *out = fopen("log.txt", "a+");
    fprintf(out, "%d %lu %d %d %lf %lf %lf %s %s %s %s %d\n",
            nprocs, block_size, count, nprocs, time, bw, iops, path.c_str(), fs_type, store_type, dss, direct);
    fclose(out);
}

size_t to_size(char *num)
{
    int len = strlen(num);
    if(num[len-1] == 'k' || num[len-1] == 'K') {
        return (size_t)atoi(num)*(1ull<<10);
    }
    else if(num[len-1] == 'm' || num[len-1] == 'M') {
        return (size_t)atoi(num)*(1ull<<20);
    }
    else if(num[len-1] == 'g' || num[len-1] == 'G') {
        return (size_t)atoi(num)*(1ull<<30);
    }
    return (size_t)atoi(num);
}

int parallel_open(std::string path, int direct) {
    int rank, nprocs, fd;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    for(int i = 0; i < nprocs; ++i) {
        if(rank == i) {
            if (direct) {
              fd = open(path.c_str(), O_RDWR | O_CREAT | O_DIRECT, 0666);
            } else {
              fd = open(path.c_str(), O_RDWR | O_CREAT, 0666);
            }
            if (fd < 0) {
                perror("Open failed");
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    return fd;
}

double direct_write(size_t block_size, int count, int rank, std::string path, int direct)
{
    char *buffer = (char*)aligned_alloc(4096, block_size);
    if(buffer == NULL) {
        printf("Could not allocate memory (direct_write)\n");
        exit(1);
    }
    memset(buffer, 0, block_size);

    int fd = parallel_open(path, direct);
    double start_time = clock();
    if(fd < 0) {
        printf("Could not open file (direct_write) (%s)\n", path.c_str());
        perror("");
        exit(1);
    }
    size_t offset = rank*block_size*count;
    lseek(fd, offset, SEEK_SET);
    for(int i = 0; i < count; i++) {
        int cnt = write(fd, buffer, block_size);
        if (cnt < 0) {
            printf("Could not write to file (direct_write) (%s)\n", path.c_str());
            perror("");
            exit(1);
        }
    }
    close(fd);
    free(buffer);
    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = clock();
    return (end_time - start_time)/CLOCKS_PER_SEC;
}

double direct_write_fpp(size_t block_size, int count, int rank, std::string path, int direct)
{
    char *buffer = (char*)aligned_alloc(4096, block_size);
    if(buffer == NULL) {
        printf("Could not allocate memory (direct_write)\n");
        exit(1);
    }
    memset(buffer, 0, block_size);

    path = path + std::to_string(rank);
    int fd = parallel_open(path, direct);
    double start_time = MPI_Wtime();
    if(fd < 0) {
        printf("Could not open file (direct_write) (%s)\n", path.c_str());
        perror("");
        exit(1);
    }
    for(int i = 0; i < count; i++) {
        int cnt = write(fd, buffer, block_size);
        if (cnt < 0) {
            printf("Could not write to file (direct_write) (%s)\n", path.c_str());
            perror("");
            exit(1);
        }
    }
    close(fd);
    free(buffer);
    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();
    return end_time - start_time;
}

int main(int argc, char **argv) {
  int rank, nprocs;
  double time;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  if(argc != 9) {
      printf("USAGE: test_pmem [path] [block-size (K/M/G)] [count] [fpp] [fs-type] [storage-type] [dss] [direct]");
      return -1;
  }
  std::string path = std::string(argv[1]);
  size_t block_size = to_size(argv[2]);
  int count = atoi(argv[3]);
  int fpp = atoi(argv[4]);
  char *fs_type = argv[5];
  char *store_type = argv[6];
  char *dss = argv[7];
  int direct = atoi(argv[8]);
  if(rank == 0) {
    printf("Write of %lu bytes %d times per process to %s %s\n", block_size, count, path.c_str(), fpp ? "with fpp" : "without fpp");
  }

  switch(fpp) {
      case 0: {
          time = direct_write(block_size, count, rank, path, direct);
          break;
      }
      case 1: {
          time = direct_write_fpp(block_size, count, rank, path, direct);
          break;
      }
  }

  log_end(rank, nprocs, block_size, count, time, path, fs_type, store_type, dss, direct);
  MPI_Finalize();
  return 0;
}
