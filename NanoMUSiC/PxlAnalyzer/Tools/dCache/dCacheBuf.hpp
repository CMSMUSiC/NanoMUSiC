#include <pthread.h>
#include <stdexcept>
#include <streambuf>
#include <string>

#include "Tools/SignalHandler.hpp"

class dCache_error : public std::runtime_error
{
  public:
    explicit dCache_error(const std::string &what_arg) : runtime_error(what_arg)
    {
    }
};

class dCacheBuf : public std::streambuf, public Tools::EventHandler
{
  public:
    struct read_ahead_data
    {
        // max bytes to read
        std::streamsize bufsize;
        // file to read from
        int file;
        // number of bytes read in last read ahead
        std::streamsize aheadsize;
        // buffer to write to
        char *buffer;
    };

    dCacheBuf();
    ~dCacheBuf();

    // tell us, if there is a file already open
    bool is_open()
    {
        return file > 0;
    }
    // open the file denoted by name
    // name must something that's understood by dCache (precisely by dc_open() )
    // returns the this pointer if successful, 0 otherwise
    dCacheBuf *open(const char *name, unsigned int timeout = 3600);
    dCacheBuf *open(const std::string &name, unsigned int timeout = 3600)
    {
        return open(name.c_str(), timeout);
    }
    // close file, if there is one
    // returns the this pointer, if the file was successfully closed, 0 otherwise
    //(closing a non-opened file is a failure, too
    dCacheBuf *close();

    // how to handle various signals
    virtual void handle_signal(int signum);

  protected:
    // how many chars are left in the file
    std::streamsize showmanyc();
    // called when the buffer runs low, refills the buffer
    int underflow();

  private:
    // open timeout
    unsigned int m_timeout;
    // size of the buffer
    std::streamsize bufsize;
    // size of push back buffer
    std::streamsize pbsize;
    // buffer to hold data
    char *active_buffer;
    char *readahead_buffer;
    char *canard_front, *canard_center, *canard_rear;
    // name of the last successfully opened file
    std::string filename;
    // file descriptor
    int file;
    // number of characters already read
    std::streamsize readsize;

    // arguments to pass to the thread
    read_ahead_data rhd;

    // read ahead thread
    pthread_t thread;

    // true if a thread is busy doing a read-ahead
    // false if nothing is being read
    bool read_active;

    // read ahead functions
    void start_read_ahead(char *buffer);
    std::streamsize read_ahead_wait();

    // manage signal handling
    void register_signals();
    void unregister_signals();
};
