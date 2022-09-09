#include "dCacheBuf.hh"

#include <algorithm>
#include <csignal>
#include <iostream>
#include <string.h>

#include "dcap.h"

using namespace std;

void *read_ahead(void *data)
{
    // cast the argument in a usable type
    dCacheBuf::read_ahead_data *rhd = (dCacheBuf::read_ahead_data *)data;
    // try to read data, at max bufSize characters and write into the buffer
    rhd->aheadsize = dc_read(rhd->file, rhd->buffer, rhd->bufsize);
    // looks like we're done here
    pthread_exit(NULL);
}

dCacheBuf::dCacheBuf()
    : streambuf(), m_timeout(3600),
      // get one megabyte of buffer space
      bufsize(1048576),
      // we don't have a file open yet
      filename(), file(0), readsize(0), read_active(false)
{
    // take one permille as push back buffer, but at least 10 characters
    pbsize = max(bufsize / 1000, (long int)10);
    // get two buffer of appropriate size
    // one satisfy the consumers and one to read into at the same time
    active_buffer = new char[bufsize + pbsize];
    readahead_buffer = new char[bufsize + pbsize];

    // set all pointer to the start of the active buffer, behind the push back area
    setg(active_buffer + pbsize, active_buffer + pbsize, active_buffer + pbsize);
}

dCacheBuf::~dCacheBuf()
{
    // close file, if still open
    close();
    // get rid of the buffers
    delete[] active_buffer;
    delete[] readahead_buffer;
}

dCacheBuf *dCacheBuf::open(const char *name, unsigned int timeout)
{
    // fail if we already got a file open
    if (is_open())
    {
        return 0;
    }

    // reset timeout value
    m_timeout = timeout;

    // first register the signals
    register_signals();
    // open a file read only
    dc_setOpenTimeout(m_timeout);
    file = dc_open(name, O_RDONLY);
    // check if it worked
    if (file > 0)
    {
        // switch off read-ahead
        dc_noBuffering(file);
        // looks good, so store the filename
        filename = name;
        // reset the pointers
        setg(active_buffer + pbsize, active_buffer + pbsize, active_buffer + pbsize);
        // start a read-ahead
        start_read_ahead(readahead_buffer + pbsize);
        // and return this
        return this;
    }
    else
    {
        // clean up if we failed
        close();
        return 0;
    }
}

dCacheBuf *dCacheBuf::close()
{
    // wait fo the read ahead thread to finish
    if (read_active)
    {
        read_ahead_wait();
    }
    // clean up
    filename.clear();
    readsize = 0;
    // reset the pointers
    setg(active_buffer + pbsize, active_buffer + pbsize, active_buffer + pbsize);
    // if we have a file open, close it
    bool success;
    if (is_open())
    {
        // try to close it
        if (dc_close(file) == 0)
        {
            // closing successful
            success = true;
        }
        else
        {
            // closing failed
            success = false;
        }
    }
    else
    {
        // nothing to close
        success = false;
    }

    // unregister signals
    unregister_signals();

    // reset file descriptor
    file = 0;

    // return failure or success
    return success ? this : 0;
}

int dCacheBuf::underflow()
{
    // we can't read if no file is open
    if (!is_open())
        throw dCache_error("No file opened.");

    // this is something which should not happen
    // underflow was called, but there is still data in the buffer
    if (gptr() < egptr())
    {
        return traits_type::to_int_type(*gptr());
    }

    // compute the number of characters for the push back area
    // it can't be larger than the push back area
    // and also not larger than the number of characters we've already read
    streamsize numPutback = min(pbsize, gptr() - eback());
    char *putback_buf = new char[numPutback];
    // move the numPutback characters before the current pointer position into the push back area
    memmove(putback_buf, gptr() - numPutback, numPutback);

    // wait for the read-ahead thread
    streamsize num = read_ahead_wait();
    // check for error
    if (num < 0)
    {
        cerr << "dCacheBuf: Read failure after " << readsize << " bytes, error message:" << endl;
        dc_perror("dCacheBuf: ");
        // close old connection
        cerr << "dCacheBuf: Closing old connection with file descriptor " << file << "..." << endl;
        if (dc_close2(file) != 0)
        {
            cerr << "dCacheBuf: Error while closing file descriptor " << file << ":" << endl;
            dc_perror("dCacheBuf: ");
            cerr << "dCacheBuf: Going on anyway...." << endl;
        }
        cerr << "dCacheBuf: Closed, trying to reconnect..." << endl;
        // reset the file descriptor
        file = 0;
        // re-open the file read only
        dc_setOpenTimeout(m_timeout);
        file = dc_open(filename.c_str(), O_RDONLY);
        dc_noBuffering(file);
        // check that it worked
        if (file == 0)
        {
            cerr << "dCacheBuf: Failed to open file " << filename << ", error message:" << endl;
            dc_perror("dCacheBuf: ");
            throw dCache_error("Failed to re-open file.");
        }
        // jump to where we last tried to read from
        cerr << "dCacheBuf: Reconnected with file descriptor " << file << ", seeking old position at " << readsize
             << "..." << endl;
        if (readsize != dc_lseek(file, readsize, SEEK_SET))
        {
            cerr << "dCacheBuf: Failed to seek to position " << readsize << ", error message:" << endl;
            dc_perror("dCacheBuf: ");
            throw dCache_error("Failed to jump to previous file position.");
        }
        // looks all fine, so try to read again
        cerr << "dCacheBuf: Back to old position, reading again into buffer: " << (void *)readahead_buffer << endl;
        num = dc_read(file, readahead_buffer + pbsize, bufsize);
        // and again check if it worked
        if (num < 0)
        {
            cerr << "dCacheBuf: Read failure, error message:" << endl;
            dc_perror("dCacheBuf: ");
            throw dCache_error("Failed to read from re-opened file.");
        }
        cerr << "dCacheBuf: Read " << num << " bytes, seems we're back in business!" << endl;
    }
    // in case we got nothing, return EOF
    if (num == 0)
    {
        return traits_type::eof();
    }

    // move the numPutback characters before the current pointer position into the push back area
    memmove(readahead_buffer + pbsize - numPutback, putback_buf, numPutback);

    // now exchange active and read-ahead buffer
    char *temp_buffer = active_buffer;
    active_buffer = readahead_buffer;
    readahead_buffer = temp_buffer;

    // reading worked, reset the pointers
    setg(active_buffer + pbsize - numPutback, // beginning of the push back area
         active_buffer + pbsize,              // end of the push back area
         active_buffer + pbsize + num);       // end of the push back area plus the number of character we've read

    // increase the number of characters we've already read
    readsize += num;

    // and start a new read-ahead
    start_read_ahead(readahead_buffer + pbsize);

    // return the next character
    return traits_type::to_int_type(*gptr());
}

streamsize dCacheBuf::showmanyc()
{
    // WARNING: This function is quite a waste
    // but I'm too lazy to do it better

    // wait fo the read ahead thread to finish
    if (read_active)
        read_ahead_wait();
    // jump to the end of the file to get the current file size
    streamsize filesize = dc_lseek(file, 0, SEEK_END);
    // jump back to where we came from
    dc_lseek(file, readsize, SEEK_SET);
    // restart the read ahead
    start_read_ahead(readahead_buffer + pbsize);
    // return remaining characters
    return filesize - readsize;
}

void dCacheBuf::start_read_ahead(char *buffer)
{
    // set the data the thread needs
    rhd.bufsize = bufsize;
    rhd.file = file;
    rhd.aheadsize = 0;
    rhd.buffer = buffer;
    // start a thread
    pthread_create(&thread, NULL, read_ahead, (void *)&rhd);
    // and store that we're reading
    read_active = true;
}

std::streamsize dCacheBuf::read_ahead_wait()
{
    void *status;
    // wait for our read ahead thread to finish
    pthread_join(thread, &status);
    // finished, so set no active read
    read_active = false;
    // return how much we've read
    return rhd.aheadsize;
}

void dCacheBuf::handle_signal(int signum)
{
    std::cerr << "Handling signal: " << signum << std::endl;
    if (signum == SIGTERM || signum == SIGFPE || signum == SIGILL || signum == SIGSEGV || signum == SIGBUS ||
        signum == SIGABRT || signum == SIGHUP || signum == SIGINT || signum == SIGQUIT || signum == SIGTERM)
    {
        std::cerr << "It's a terminating signal, closing file..." << std::endl;
        close();
        std::cerr << "Closed." << std::endl;
    }
}

void dCacheBuf::register_signals()
{
    Tools::SignalHandler::handler()->register_handler(SIGTERM, this);
    Tools::SignalHandler::handler()->register_handler(SIGFPE, this);
    Tools::SignalHandler::handler()->register_handler(SIGILL, this);
    Tools::SignalHandler::handler()->register_handler(SIGSEGV, this);
    Tools::SignalHandler::handler()->register_handler(SIGBUS, this);
    Tools::SignalHandler::handler()->register_handler(SIGABRT, this);
    Tools::SignalHandler::handler()->register_handler(SIGHUP, this);
    Tools::SignalHandler::handler()->register_handler(SIGINT, this);
    Tools::SignalHandler::handler()->register_handler(SIGQUIT, this);
    Tools::SignalHandler::handler()->register_handler(SIGTERM, this);
}

void dCacheBuf::unregister_signals()
{
    Tools::SignalHandler::handler()->remove_handler(SIGTERM, this);
    Tools::SignalHandler::handler()->remove_handler(SIGFPE, this);
    Tools::SignalHandler::handler()->remove_handler(SIGILL, this);
    Tools::SignalHandler::handler()->remove_handler(SIGSEGV, this);
    Tools::SignalHandler::handler()->remove_handler(SIGBUS, this);
    Tools::SignalHandler::handler()->remove_handler(SIGABRT, this);
    Tools::SignalHandler::handler()->remove_handler(SIGHUP, this);
    Tools::SignalHandler::handler()->remove_handler(SIGINT, this);
    Tools::SignalHandler::handler()->remove_handler(SIGQUIT, this);
    Tools::SignalHandler::handler()->remove_handler(SIGTERM, this);
}
