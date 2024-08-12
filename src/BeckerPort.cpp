#include "BeckerPort.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h> // write(), read(), close()
#include <errno.h> // Error integer and strerror() function
#include <fcntl.h> // Contains file controls like O_RDWR
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "globals.h"

#ifndef closesocket
#include <unistd.h>
#define closesocket(x) ::close(x)
#endif

#define BECKER_STATUS_PORT      BECKER_PORT_ADR
#define BECKER_DATA_PORT        (BECKER_PORT_ADR + 1)
#define BECKER_DEFAULT_PORT     65504
#define BECKER_IOWAIT_MS        500
#define BECKER_CONNECT_TMOUT    2000
#define BECKER_SUSPEND_MS       5000
#define BECKER_BUFFER_SIZE      1024

#ifndef MSG_NOSIGNAL
#  define MSG_NOSIGNAL 0
#  if defined(__APPLE__) || defined(__MACH__)
// MSG_NOSIGNAL does not exists on older macOS, use SO_NOSIGPIPE
#    define USE_SO_NOSIGPIPE
#  endif
#endif

/* borrowed from lwip/ip4_addr.h */
/** 255.255.255.255 */
#define IPADDR_NONE         ((uint32_t)0xffffffffUL)
/** 127.0.0.1 */
#define IPADDR_LOOPBACK     ((uint32_t)0x7f000001UL)
/** 0.0.0.0 */
#define IPADDR_ANY          ((uint32_t)0x00000000UL)


// Return a single IP4 address given a hostname
static in_addr_t get_ip4_addr_by_name(const char *hostname)
{
    in_addr_t result = IPADDR_NONE;

    debug("BeckerPort", "Resolving hostname \"%s\"", hostname);
    struct hostent *info = gethostbyname(hostname);

    if(info == nullptr)
    {
        warning("BeckerPort", "Name failed to resolve");
    }
    else
    {
        if(info->h_addr_list[0] != nullptr)
        {
            result = *((in_addr_t*)(info->h_addr_list[0]));
            struct in_addr sin;
            sin.s_addr = result;
            debug("BeckerPort", "Resolved to address %s", inet_ntoa(sin));
        }
    }
    return result;
}


// reference timestamp
static uint64_t _get_start_millis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec*1000ULL+tv.tv_usec/1000ULL);
}
static uint64_t _start_millis = _get_start_millis();

static uint64_t get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec*1000UL+tv.tv_usec/1000UL) - _start_millis;
}

// Constructor
BeckerPort::BeckerPort() :
    _host{0},
    _ip(IPADDR_NONE),
    _port(BECKER_DEFAULT_PORT),
    _listening(false),
    _fd(-1),
    _listen_fd(-1),
    _state(&BeckerStopped::getInstance()),
    _errcount(0),
    _timestamp_ms(get_timestamp())
{
    // pre-allocate buffer
    _tx_buffer.reserve(BECKER_BUFFER_SIZE);
}

BeckerPort::~BeckerPort()
{
    end();
}

void BeckerPort::ResetDevice(int ticks)
{
    debug("BeckerPort", "ResetDevice");
    _tx_buffer.clear();
}

void BeckerPort::WriteToDevice(BYTE port, BYTE value, int ticks)
{
    if (port == BECKER_DATA_PORT) 
    {
        debug("BeckerPort", "OUT: %02X %c", value, isprint(value) ? value : ' ');
        // TODO write()
        if (_tx_buffer.size() < _tx_buffer.capacity())
            _tx_buffer.push_back(value);
        else
            warning("BeckerPort", "TX buffer is full");
    }
}

BYTE BeckerPort::ReadFromDevice(BYTE port, int ticks)
{
    BYTE value = 0x00;
    if (port == BECKER_DATA_PORT) 
    {
        read(&value, 1);
        debug("BeckerPort", "IN: %02X %c", value, isprint(value) ? value : ' ');
    }
    else if (port == BECKER_STATUS_PORT) 
    {
        value = (available() > 0) ? 0x02 : 0x00;
    }
    return value;
}

void BeckerPort::setState(BeckerState& state)
{
    _timestamp_ms = get_timestamp();
    _state = &state;
}

void BeckerPort::begin()
{
    if (_state != &BeckerStopped::getInstance())
        end();

    // listen or connect
    start_connection();
}

void BeckerPort::end()
{
    // close sockets
    if (_fd >= 0)
    {
        shutdown(_fd, 0);
        closesocket(_fd);
        _fd  = -1;
    }
    if (_listen_fd >= 0)
    {
        closesocket(_listen_fd);
        _listen_fd  = -1;
    }

    debug("BeckerPort", "Stopped");
    setState(BeckerStopped::getInstance());
}

int BeckerPort::available()
{
    // only in connected state
    if (_state != &BeckerConnected::getInstance())
        return 0;

    // check if socket is still connected
    if (!connected())
    {
        // connection was closed or it has an error
        suspend_on_disconnect();
        return 0;
    }

#if defined(_WIN32)
    unsigned long count;
    int res = ioctlsocket(_fd, FIONREAD, &count);
    res = res != 0 ? -1 : count;
#else
    int count;
    int res = ioctl(_fd, FIONREAD, &count);
    res = res < 0 ? -1 : count;
#endif
    return res;
}

// /* Discards anything in the input buffer
// */
// void BeckerPort::flush_input()
// {
//     // only in connected state
//     if (_state != &BeckerConnected::getInstance())
//         return;

//     // waste all input data
//     uint8_t rxbuf[256];
//     int avail;
//     while ((avail = available()) > 0)
//     {
//         recv(_fd, (char *)rxbuf, avail > sizeof(rxbuf) ? sizeof(rxbuf) : avail, 0);
//     }
// }

// /* Clears input buffer and flushes out transmit buffer waiting at most
//    waiting MAX_FLUSH_WAIT_TICKS until all sends are completed
// */
// void BeckerPort::flush()
// {
//     // only in connected state
//     if (_state != &BeckerConnected::getInstance())
//         return;

//     wait_sock_writable(250);
// }

// // specific to BeckerPort
// void BeckerPort::set_host(const char *host, int port)
// {
//     if (host != nullptr)
//         strlcpy(_host, host, sizeof(_host));
//     else
//         _host[0] = 0;

//     _port = port;
// }

// const char* BeckerPort::get_host(int &port)
// {
//     port = _port;
//     return _host;
// }

void BeckerPort::start_connection()
{
    _errcount = 0;
    if (_listening)
        listen_for_connection();
    else
        make_connection();
}

void BeckerPort::listen_for_connection()
{
    debug("BeckerPort", "Set up to listen on %s:%d", _host, _port);

    // Create listening socket
    _listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (_listen_fd < 0)
    {
        warning("BeckerPort", "Failed to create socket: %d - %s",
            errno, strerror(errno));
        suspend(BECKER_SUSPEND_MS);
		return;
	}

    // Set socket option
    int enable = 1;
// #if defined(_WIN32)
//     if (setsockopt(_listen_fd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *) &enable, sizeof(enable)) != 0)
// #else
    if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) != 0)
// #endif
    {
        warning("BeckerPort", "setsockopt failed: %d - %s", 
            errno, strerror(errno));
        closesocket(_listen_fd);
        _listen_fd = -1;
        suspend(BECKER_SUSPEND_MS);
        return;
    }

    // Local address to listen on
    if (_host[0] == '\0' || !strcmp(_host, "*"))
    {
        _ip = IPADDR_ANY;
    }
    else
    {
        _ip = get_ip4_addr_by_name(_host);
        if (_ip == IPADDR_NONE)
        {
            warning("BeckerPort", "Failed to resolve host name");
            closesocket(_listen_fd);
            _listen_fd = -1;
            suspend(BECKER_SUSPEND_MS);
            return;
        }
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = _ip;
    addr.sin_port = htons(_port);

    // Bind to listening address
    if (bind(_listen_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        warning("BeckerPort", "bind failed: %d - %s", 
            errno, strerror(errno));
        closesocket(_listen_fd);
        _listen_fd = -1;
        suspend(BECKER_SUSPEND_MS);
        return;
    }

    // Listen for incoming connection
    if (listen(_listen_fd, 1) != 0)
    {
        warning("BeckerPort", "listen failed: %d  %s", 
            errno, strerror(errno));
        closesocket(_listen_fd);
        _listen_fd = -1;
        suspend(BECKER_SUSPEND_MS);
        return;
    }

    // Set socket non-blocking
    if (fcntl(_listen_fd, F_SETFL, fcntl(_listen_fd, F_GETFL, 0) | O_NONBLOCK) != 0)
    {
        warning("BeckerPort", "Failed to set non-blocking mode: %d - %s", 
            errno, strerror(errno));
        closesocket(_listen_fd);
        _listen_fd = -1;
        suspend(BECKER_SUSPEND_MS);
        return;
    }

    // Finally setup
    _errcount = 0; // used by suspend()
    setState(BeckerWaitConn::getInstance());
    debug("BeckerPort", "Accepting connections");
}

void BeckerPort::make_connection()
{
    debug("BeckerPort", "Connect to %s:%d", _host[0] ? _host : "127.0.0.1", _port);

    // Create connection socket
    _fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (_fd < 0)
    {
        warning("BeckerPort", "Failed to create socket: %d - %s", 
            errno, strerror(errno));
        suspend(BECKER_SUSPEND_MS);
		return;
	}

#ifdef USE_SO_NOSIGPIPE
    // Set NOSIGPIPE socket option (old macOS)
    int enable = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_NOSIGPIPE, (char *)&enable, sizeof(enable)) < 0)
    {
        warning("BeckerPort", "setsockopt failed: %d - %s", 
            errno, strerror(errno));
        suspend(BECKER_SUSPEND_MS);
        return;
    }
#endif

    // Set socket non-blocking
    if (fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL, 0) | O_NONBLOCK) != 0)
    {
        warning("BeckerPort", "failed to set non-blocking mode: %d - %s", 
            errno, strerror(errno));
        suspend(BECKER_SUSPEND_MS);
        return;
    }

    // Remote address
    if (_host[0] == '\0')
    {
        _ip = htonl(IPADDR_LOOPBACK);
    }
    else
    {
        _ip = get_ip4_addr_by_name(_host);
        if (_ip == IPADDR_NONE)
        {
            warning("BeckerPort", "Failed to resolve host name");
            suspend(BECKER_SUSPEND_MS);
            return;
        }
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = _ip;
    addr.sin_port = htons(_port);

    // Connect to remote address
    int res = connect(_fd, (struct sockaddr *)&addr, sizeof(addr));

    if (res < 0 && errno != EINPROGRESS)
    {
        warning("BeckerPort", "connect failed: %d - %s", 
            errno, strerror(errno));
        suspend(BECKER_SUSPEND_MS);
        return;
    }

    if (res < 0)
    {
        // Still connecting
        setState(BeckerConnecting::getInstance());
        debug("BeckerPort", "Connecting...");
    }
    else 
    {
        // Connected
        _errcount = 0;
        setState(BeckerConnected::getInstance());
        debug("BeckerPort", "Connected");
    }
}

bool BeckerPort::connection_established()
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    int as = sizeof(addr);

    if (getpeername(_fd, (struct sockaddr *)&addr, (socklen_t *)&as) == 0)
    {
        // Connected
        setState(BeckerConnected::getInstance());
        debug("BeckerPort", "Connected");
        return true;
    }
    // test for connection timeout, suspend on timeout
    if (get_timestamp() - _timestamp_ms > BECKER_CONNECT_TMOUT)
    {
        warning("BeckerPort", "Connect timed out");
        suspend(BECKER_SUSPEND_MS);
    }
    return false;
}

// bool BeckerPort::accept_pending_connection(int ms)
// {
//     // if listening socket has new connection accept it
//     return(wait_sock_readable(ms, true) && accept_connection());
// }

// bool BeckerPort::accept_connection()
// {
//     struct sockaddr_in addr;
//     int as = sizeof(addr);

//     // Accept connection
//     _fd = accept(_listen_fd, (struct sockaddr *)&addr, (socklen_t *)&as);
//     if (_fd < 0)
//     {
//         debug("BeckerPort", "accept failed: %d - %s",
//             errno, strerror(errno));
//         return false;
//     }
//     debug("BeckerPort", "connection from: %s", inet_ntoa(addr.sin_addr));

//     // Set socket options
//     int val = 1;
//     if (setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&val, sizeof(val)) < 0)
//     {
//         warning("BeckerPort", "Failed to set KEEPALIVE on socket");
//     }
//     if (setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&val, sizeof(val)) < 0)
//     {
//         warning("BeckerPort", "Failed to set NODELAY on socket");
//     }

//     // Set socket non-blocking
//     if (fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL, 0) | O_NONBLOCK) != 0)
//     {
//         debug("BeckerPort", "failed to set non-blocking connection: %d - %s", 
//             errno, strerror(errno));
//         shutdown(_fd, 0);
//         closesocket(_fd);
//         _fd = -1;
//         return false;
//     }

//     // We are connected !
//     debug("BeckerPort", "Connected");
//     setState(BeckerConnected::getInstance());
//     return true;
// }

void BeckerPort::suspend(int short_ms, int long_ms, int threshold)
{
    if (_fd >= 0)
    {
        closesocket(_fd);
        _fd  = -1;
    }
    _errcount++;
    _suspend_period = short_ms;
    if (threshold > 0 && _errcount > threshold && long_ms > 0)
        _suspend_period = long_ms;
    debug("BeckerPort", "Suspending for %d ms", _suspend_period);
    setState(BeckerSuspended::getInstance());
}

void BeckerPort::suspend_on_disconnect()
{
    if (_listening && _listen_fd >=0)
    {
        if (_fd >= 0)
        {
            closesocket(_fd);
            _fd = -1;
        }
        // go directly into waiting for connection state
        setState(BeckerWaitConn::getInstance());
    }
    else
    {
        // wait before reconnecting
        suspend(BECKER_SUSPEND_MS);
    }
}

bool BeckerPort::resume()
{
    debug("BeckerPort", "Resuming...");
    if (_listening)
    {
        if (_listen_fd >= 0)
        {
            // go directly into waiting for connection state
            setState(BeckerWaitConn::getInstance());
            return true;
        }

    }
    // listen or connect
    start_connection();
    return (_state != &BeckerSuspended::getInstance());
}

bool BeckerPort::suspend_period_expired()
{
    return (get_timestamp() - _timestamp_ms > _suspend_period);
}

bool BeckerPort::connected()
{
    uint8_t dummy;
    bool con = false;
    int res = recv(_fd, (char *)&dummy, 1, MSG_PEEK);
    if (res > 0)
    {
        con = true;
    }
    else if (res == 0)
    {
        debug("BeckerPort", "Disconnected");
    }
    else
    {
        int err = errno;
        switch (err)
        {
#if defined(_WIN32)
        case WSAEWOULDBLOCK:
#else
        case EWOULDBLOCK:
        case ENOENT: // Caused by VFS
#endif
            con = true;
            break;
        default:
            warning("BeckerPort", "Connection error: %d - %s", errno, strerror(err));
            break;
        }
    }
    return con;
}

bool BeckerPort::poll_connection()
{
    if (!connected())
    {
        // connection was closed or it has an error
        suspend_on_disconnect();
    }

    // send TX buffer
    if (!_tx_buffer.empty())
    {
        ssize_t result = write_sock(_tx_buffer.data(), _tx_buffer.size());
        if (result > 0)
        {
            if (result < _tx_buffer.size())
                _tx_buffer.erase(_tx_buffer.begin(), _tx_buffer.begin()+result);
            else
                _tx_buffer.clear();
        }
    }
    return false;
}

// timeval BeckerPort::timeval_from_ms(const uint32_t millis)
// {
//   timeval tv;
//   tv.tv_sec = millis / 1000;
//   tv.tv_usec = (millis - (tv.tv_sec * 1000)) * 1000;
//   return tv;
// }

ssize_t BeckerPort::do_read(uint8_t *buffer, size_t size)
{
    // int result;
    // int rxbytes = 0;
    // int to_recv;

    // while (rxbytes < size)
    // {
    //     to_recv = size - rxbytes;
    //     result = read_sock(buffer+rxbytes, to_recv);
    //     if (result > 0)
    //         rxbytes += result;
    //     else // result <= 0 for disconnected or read error
    //         break;
    // }
    // return rxbytes;
    return read_sock(buffer, size);
}

ssize_t BeckerPort::do_write(const uint8_t *buffer, size_t size)
{
    // int result;
    // int txbytes = 0;
    // int to_send;

    // while (txbytes < size)
    // {
    //     to_send = size - txbytes;
    //     result = write_sock(buffer+txbytes, to_send);
    //     if (result > 0)
    //         txbytes += result;
    //     else if (result < 0) // write error
    //         break;
    // }
    // return txbytes;
    return write_sock(buffer, size);
}

ssize_t BeckerPort::read_sock(const uint8_t *buffer, size_t size)
{
    // if (!wait_sock_readable(timeout_ms))
    // {
    //     warning("BeckerPort", "read_sock() TIMEOUT");
    //     return -1;
    // }

    ssize_t result = recv(_fd, (char *)buffer, size, 0);
    if (result < 0)
    {
        if (errno != EWOULDBLOCK)
        {
            warning("BeckerPort", "read_sock() error: %d - %s",
                errno, strerror(errno));
            suspend_on_disconnect();
        }
    }
    else if (result == 0)
    {
        debug("BeckerPort", "Disconnected");
        suspend_on_disconnect();
    }
    return result;
}

ssize_t BeckerPort::write_sock(const uint8_t *buffer, size_t size)
{
//     if (!wait_sock_writable(timeout_ms))
//     {
//         int err = errno;
// #if defined(_WIN32)
//         if (err == WSAETIMEDOUT)
// #else
//         if (err == ETIMEDOUT)
// #endif
//         {
//             debug("BeckerPort", "write_sock() TIMEOUT");
//         }
//         else
//         {
//             suspend_on_disconnect();
//         }
//         return -1;
//     }

    ssize_t result = send(_fd, (char *)buffer, size, 0);
    if (result < 0)
    {
        if (errno != EWOULDBLOCK)
        {
            warning("BeckerPort", "write_sock() error %d: %s", 
                errno, strerror(errno));
            suspend_on_disconnect();
        }
    }
    return result;
}

// bool BeckerPort::wait_sock_readable(uint32_t timeout_ms, bool listener)
// {
//     timeval timeout_tv;
//     fd_set readfds;
//     int result;
//     int fd = listener ? _listen_fd : _fd;

//     for(;;)
//     {
//         // Setup a select call to block for socket data or a timeout
//         timeout_tv = timeval_from_ms(timeout_ms);
//         FD_ZERO(&readfds);
//         FD_SET(fd, &readfds);
//         result = select(fd + 1, &readfds, nullptr, nullptr, &timeout_tv);

//         // select error
//         if (result < 0)
//         {
//             int err = errno;
// #if defined(_WIN32)
//             if (err == WSAEINTR)
// #else
//             if (err == EINTR) 
// #endif
//             {
//                 // TODO adjust timeout_tv
//                 continue;
//             }
//             warning("BeckerPort", "wait_sock_readable() select error %d: %s", err, strerror(err));
//             return false;
//         }

//         // select timeout
//         if (result == 0)
//             return false;

//         // this shouldn't happen, if result > 0 our fd has to be in the list!
//         if (!FD_ISSET(fd, &readfds))
//         {
//             warning("BeckerPort", "wait_sock_readable() unexpected select result");
//             return false;
//         }
//         break;
//     }
//     return true;
// }

// bool BeckerPort::wait_sock_writable(uint32_t timeout_ms)
// {
//     timeval timeout_tv;
//     fd_set writefds;
//     fd_set errfds;
//     int result;

//     for(;;)
//     {
//         timeout_tv = timeval_from_ms(timeout_ms);
//         // select for write
//         FD_ZERO(&writefds);
//         FD_SET(_fd, &writefds);
//         // select for error too
//         FD_ZERO(&errfds);
//         FD_SET(_fd, &errfds);

//         result = select(_fd + 1, nullptr, &writefds, &errfds, &timeout_tv);

//         // select error
//         if (result < 0) 
//         {
//             int err = errno;
// #if defined(_WIN32)
//             if (err == WSAEINTR)
// #else
//             if (err == EINTR) 
// #endif
//             {
//                 // TODO adjust timeout_tv
//                 continue;
//             }
//             warning("BeckerPort", "wait_sock_writable() select error %d: %s", err, strerror(err));
//             return false;
//         }

//         // select timeout
//         if (result == 0)
//         {
// #if defined(_WIN32)
//             int err = WSAETIMEDOUT;
// #else
//             int err = ETIMEDOUT;
// #endif
//             // set errno
//             //compat_setsockerr(err); // TODO
//             errno = err;
//             return false;
//         }
//         // Check for error on socket
//         {
//             int sockerr;
//             socklen_t len = (socklen_t)sizeof(int);
//             // Store any socket error value in sockerr
//             int res = getsockopt(_fd, SOL_SOCKET, SO_ERROR, (char *)&sockerr, &len);
//             if (res < 0)
//             {
//                 // Failed to retrieve SO_ERROR
//                 int err = errno;
//                 warning("BeckerPort", "getsockopt on fd %d, errno: %d - %s", _fd, err ,strerror(err));
//                 return false;
//             }
//             // Retrieved SO_ERROR and found that we have an error condition
//             if (sockerr != 0)
//             {
//                 warning("BeckerPort", "socket error on fd %d, errno: %d - %s", _fd, sockerr, strerror(sockerr));
//                 // set errno
//                 //compat_setsockerr(sockerr); // TODO
//                 errno = sockerr;
//                 return false;
//             }
//         }
//         //Debug_print("socket is ready for write\n");
//         break;
//     }
//     return true;
// }

//
// Becker state handlers
//

// Stopped state

bool BeckerStopped::poll(BeckerPort *port)
{
    return false;
}

size_t BeckerStopped::read(BeckerPort *port, uint8_t *buffer, size_t size)
{
    return 0;
}

ssize_t BeckerStopped::write(BeckerPort *port, const uint8_t *buffer, size_t size)
{
    return 0;
}

// Suspended state

bool BeckerSuspended::poll(BeckerPort *port)
{
    if (port->suspend_period_expired())
        // resume
        return port->resume();
    return false;
}

size_t BeckerSuspended::read(BeckerPort *port, uint8_t *buffer, size_t size)
{
    if (port->suspend_period_expired() && port->resume())
        // connection was resumed, we can proceed with read
        return port->getState()->read(port, buffer, size);
    return 0;
}

ssize_t BeckerSuspended::write(BeckerPort *port, const uint8_t *buffer, size_t size)
{
    if (port->suspend_period_expired() && port->resume())
        // connection was resumed, we can proceed with write
        return port->getState()->write(port, buffer, size);
    return 0;
}

// Waiting for connection

bool BeckerWaitConn::poll(BeckerPort *port)
{
    return false;
    // return port->accept_pending_connection(ms); // true if new connection was accepted
}

size_t BeckerWaitConn::read(BeckerPort *port, uint8_t *buffer, size_t size)
{
    return 0;
    // if (port->accept_pending_connection(BECKER_IOWAIT_MS))
    //     // connection was accepted, we can proceed with read
    //     return port->getState()->read(port, buffer, size);
    // return 0;
}

ssize_t BeckerWaitConn::write(BeckerPort *port, const uint8_t *buffer, size_t size)
{
    return 0;
    // if (port->accept_pending_connection(BECKER_IOWAIT_MS))
    //     // connection was accepted, we can proceed with write
    //     return port->getState()->write(port, buffer, size);
    // return 0;
}

// Connecting

bool BeckerConnecting::poll(BeckerPort *port)
{
    return port->connection_established(); // true if connection was established
}

size_t BeckerConnecting::read(BeckerPort *port, uint8_t *buffer, size_t size)
{
    if (port->connection_established())
        // connection was established, we can proceed with read
        return port->getState()->read(port, buffer, size);
    return 0;
}

ssize_t BeckerConnecting::write(BeckerPort *port, const uint8_t *buffer, size_t size)
{
    if (port->connection_established())
        // connection was established, we can proceed with write
        return port->getState()->write(port, buffer, size);
    return 0;
}

// Connected

bool BeckerConnected::poll(BeckerPort *port)
{
    return port->poll_connection();
}

size_t BeckerConnected::read(BeckerPort *port, uint8_t *buffer, size_t size)
{
    return port->do_read(buffer, size);
}

ssize_t BeckerConnected::write(BeckerPort *port, const uint8_t *buffer, size_t size)
{
    return port->do_write(buffer, size);
}
