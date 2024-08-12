#ifndef DWBECKER_H
#define DWBECKER_H

#include <stdint.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <vector>

#include "PeripheralDevice.h"

// Becker port using 0x3E and 0x3F 
#define BECKER_PORT_MASK        0xFE
#define BECKER_PORT_ADR         0x3E

class BeckerPort;

class BeckerState
{
public:
    virtual bool poll(BeckerPort *port) = 0;
    virtual size_t read(BeckerPort *port, uint8_t *buffer, size_t size) = 0;
    virtual ssize_t write(BeckerPort *port, const uint8_t *buffer, size_t size) = 0;
    virtual ~BeckerState() {}
};

class BeckerStopped : public BeckerState
{
public:
    virtual bool poll(BeckerPort *port) override;
    virtual size_t read(BeckerPort *port, uint8_t *buffer, size_t size) override;
    virtual ssize_t write(BeckerPort *port, const uint8_t *buffer, size_t size) override;
    static BeckerStopped& getInstance() { static BeckerStopped instance; return instance; }

private:
	BeckerStopped() {}
	BeckerStopped(const BeckerStopped& other);
	BeckerStopped& operator=(const BeckerStopped& other);
};

class BeckerWaitConn : public BeckerState
{
public:
    virtual bool poll(BeckerPort *port) override;
    virtual size_t read(BeckerPort *port, uint8_t *buffer, size_t size) override;
    virtual ssize_t write(BeckerPort *port, const uint8_t *buffer, size_t size) override;
    static BeckerWaitConn& getInstance() { static BeckerWaitConn instance; return instance; }

private:
	BeckerWaitConn() {}
	BeckerWaitConn(const BeckerWaitConn& other);
	BeckerWaitConn& operator=(const BeckerWaitConn& other);
};

class BeckerConnecting : public BeckerState
{
public:
    virtual bool poll(BeckerPort *port) override;
    virtual size_t read(BeckerPort *port, uint8_t *buffer, size_t size) override;
    virtual ssize_t write(BeckerPort *port, const uint8_t *buffer, size_t size) override;
    static BeckerConnecting& getInstance() { static BeckerConnecting instance; return instance; }
private:
	BeckerConnecting() {}
	BeckerConnecting(const BeckerConnecting& other);
	BeckerConnecting& operator=(const BeckerConnecting& other);
};

class BeckerConnected : public BeckerState
{
public:
    virtual bool poll(BeckerPort *port) override;
    virtual size_t read(BeckerPort *port, uint8_t *buffer, size_t size) override;
    virtual ssize_t write(BeckerPort *port, const uint8_t *buffer, size_t size) override;
    static BeckerConnected& getInstance() { static BeckerConnected instance; return instance; }
private:
	BeckerConnected() {}
	BeckerConnected(const BeckerConnected& other);
	BeckerConnected& operator=(const BeckerConnected& other);
};

class BeckerSuspended : public BeckerState
{
public:
    virtual bool poll(BeckerPort *port) override;
    virtual size_t read(BeckerPort *port, uint8_t *buffer, size_t size) override;
    virtual ssize_t write(BeckerPort *port, const uint8_t *buffer, size_t size) override;
    static BeckerSuspended& getInstance() { static BeckerSuspended instance; return instance; }
private:
	BeckerSuspended() {}
	BeckerSuspended(const BeckerSuspended& other);
	BeckerSuspended& operator=(const BeckerSuspended& other);
};


class BeckerPort: public PeripheralDevice
{
private:
    char _host[64]; // TODO change to std::string
    in_addr_t _ip;
    uint16_t _port;

    // is waiting for connection (listening) or connecting to?
    bool _listening;

    // file descriptors, sockets
    int _fd;
    int _listen_fd;

    // state machine handlers for poll(), read() and write()
    BeckerState *_state;

    // error counter
    int _errcount;
    uint64_t _timestamp_ms;
    int _suspend_period;

    // buffers
    std::vector<uint8_t> _tx_buffer;

protected:
	void start_connection();
	void listen_for_connection();
	void make_connection();
	bool accept_connection();

	void suspend(int short_ms, int long_ms=0, int threshold=0);
	void suspend_on_disconnect();
	bool resume();
	bool suspend_period_expired();

	bool connection_established();
	// bool accept_pending_connection();

	bool connected();
	bool poll_connection();

	// static timeval timeval_from_ms(const uint32_t millis);

	ssize_t do_read(uint8_t *buffer, size_t size);
	ssize_t do_write(const uint8_t *buffer, size_t size);

    ssize_t read_sock(const uint8_t *buffer, size_t size);
    ssize_t write_sock(const uint8_t *buffer, size_t size);

	// bool wait_sock_readable(uint32_t timeout_ms, bool listener=false);
    // bool wait_sock_writable(uint32_t timeout_ms);

public:

    BeckerPort();
    virtual ~BeckerPort();

    virtual void ResetDevice(int ticks) override;
    virtual void WriteToDevice(BYTE port, BYTE value, int ticks) override;
    virtual BYTE ReadFromDevice(BYTE port, int ticks) override;

    void begin();
	void end();

    int available();
    void flush();
    void flush_input();

    // keep BeckerPort alive
	bool poll() { return _state->poll(this); }
    // read bytes into buffer
    size_t read(uint8_t *buffer, size_t size) { return _state->read(this, buffer, size); }
    // write buffer
    ssize_t write(const uint8_t *buffer, size_t size) { return _state->write(this, buffer, size); }

    // specific to BeckerPort
    void set_host(const char *host, int port);
    const char* get_host(int &port);

	inline BeckerState* getState() const { return _state; }
	void setState(BeckerState& state);

    // friends, state handlers
    friend class BeckerStopped;
    friend class BeckerWaitConn;
    friend class BeckerConnecting;
    friend class BeckerConnected;
    friend class BeckerSuspended;
};

#endif // DWBECKER_H
