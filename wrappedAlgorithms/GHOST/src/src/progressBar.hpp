#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>

typedef boost::posix_time::microsec_clock bclock;
typedef boost::posix_time::ptime ptime;

// may drop counts in parallel
class ProgressBar
{
  int prev=0, count=0, max;
  double progress=0;
  int barWidth=64;
  ptime time;
  boost::mutex mutex;
  public:
    ProgressBar(int i, ptime awastin) {max=i; time = awastin;};
    ProgressBar(int i) {max=i; time = bclock::local_time();};
    void update();
};

void ProgressBar::update()
{
  count++;
  progress = (double)(count) / max;
  if(2*barWidth*progress > prev)
    if(mutex.try_lock())
    {
      prev++;
      int n = (int)(progress*100.0);
      std::cout << n << "%" << (n<10?"  [":n<100?" [":"[");
      int pos = barWidth * progress;
      for(int i=0;i<barWidth;i++)
      {
        if(i<pos) std::cout << "=";
        else if(i==pos) std::cout << ">";
        else std::cout << " ";
      }
      auto elapsed = (bclock::local_time() - time).total_seconds();
      int eta = elapsed>0?(int)(elapsed / progress - elapsed):0;
      std::cout << "] ETA " << eta << 
        (eta<10?"s   \r":eta<100?"s  \r":eta<1000?"s \r":"s\r");
      std::cout.flush();
      mutex.unlock();
    }
}
