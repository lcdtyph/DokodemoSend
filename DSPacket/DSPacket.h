#ifndef __DSPACKET_H__
#define __DSPACKET_H__

#include <QString>
#include <QVector>
#include <libnet.h>
#include <stdint.h>

class DSPacket {
    
public:
    DSPacket(const QString &device, const QString &srcIp, const QString &dstIp,
                int ipFamily, int16_t proto, uint8_t ttl = 64, time_t delay = 0)
        : family(ipFamily), proto(proto), ttl(ttl), ready(false), delay(delay),
            handle(nullptr), source(srcIp), destination(dstIp), device(device) { }

    virtual ~DSPacket() { }

    virtual int send() const;
    virtual int length() const;
    int status() const;
    void setTTL(uint8_t ttl) { this->ttl = ttl; ready = false; }
    void setDelay(time_t delay) { this->delay = delay; ready = false; }
    virtual void updateParameter() = 0;

    virtual QString getProto() const = 0;
    QString getSourceIp() const { return source; }
    QString getDestinationIp() const { return destination; }
    QString getCapture() const { return device; }

    virtual void dumpPacket(QString &buf) const;

protected:
    int family;
    uint16_t proto;
    uint8_t ttl;
    bool ready;
    time_t delay;
    libnet_t *handle;
    QString source, destination, device;
    QVector<uint8_t> packet;

    void updateIpHeader(int protoSize);
    void initHandle();
    void updatePacketData();
};

class DSTcpPacket : public DSPacket {

public:
    DSTcpPacket(const QString &device, int ipFamily, const QString &srcIp, const QString &dstIp);
    ~DSTcpPacket() { }

    void setupParameter(uint16_t srcPort, uint16_t dstPort,
            int seq, int ack, int flag, int windowsize,
            uint16_t urgpoint, const QVector<uint8_t> &option,
            const QVector<uint8_t> &payload);
    void updateParameter();
    QString getProto() const { return "TCP"; }

private:
    uint16_t srcPort, dstPort, urgpoint;
    int seq, ack, flag, windowsize;
    QVector<uint8_t> option, payload;

};

class DSUdpPacket : public DSPacket{

public:
    DSUdpPacket(const QString &device, int ipFamily, const QString &srcIp, const QString &dstIp);
    ~DSUdpPacket() { }

    void setupParameter(uint16_t srcPort, uint16_t dstPort, const QVector<uint8_t> &payload);
    void updateParameter();
    QString getProto() const { return "UDP"; }

private:
    uint16_t srcPort, dstPort;
    QVector<uint8_t> payload;

};

#endif

