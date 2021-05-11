#include <mutex>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <malloc.h>
#include <iostream>
#include <sys/time.h>
#include <assert.h>


double now()
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec+(double)tv.tv_usec/1000000;
}


char* data;
std::string file_name;

static constexpr size_t data_size = 1*1024*1024;
static constexpr size_t data_align = 4*1024;


int do_test(bool use_2_fds)
{
  int fd0, fd1;
  off_t off0, off1;
  double t0,t1,t2;
  double ts0 = 0, ts1 = 0;
  int r;
  
  fd0 = open(file_name.c_str(), O_RDWR);
  assert(fd0 >= 0);
  fd1 = open(file_name.c_str(), O_RDWR);
  assert(fd1 >= 0);

  off0 = 0;
  off1 = data_size * 2 * 100;

  for (int i = 0; i < 100; i++) {
    r = pwrite(fd0, data, data_size, off0);
    assert(r == data_size);
    off0 += data_size*2;
    if (use_2_fds)
      r = pwrite(fd1, data, data_size, off1);
    else
      r = pwrite(fd0, data, data_size, off1);
    assert(r == data_size);
    off1 += data_size*2;
    
    t0 = now();
    fdatasync(fd0);
    t1 = now();
    fdatasync(fd1);
    t2 = now();

    ts0 += t1 - t0;
    ts1 += t2 - t1;
  }
  close(fd0);
  close(fd1);
  std::cout << "ts0=" << ts0 << " ts1=" << ts1 << std::endl;
    
  return 0;
}


int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage " << argv[0] << " test_file(block_dev)" << std::endl;
    return -1;
  }

  data = (char*) memalign(data_align, data_size);
  file_name = argv[1];
  do_test(true);
  do_test(false);
  
  return 0;
}
