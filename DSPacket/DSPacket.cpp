#include "DSPacket.h"
#include <unistd.h>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <QDebug>

int DSPacket::send() const {
    usleep(delay);
    qDebug() << "packet is" << (ready ? "ready" : "not ready");
    int sent = ready ? libnet_write(handle) : 0;
    qDebug() << sent << "bytes sent.";
    if (sent == -1) qDebug() << libnet_geterror(handle);
    return sent;
}

int DSPacket::length() const {
    return packet.length();
}

void DSPacket::dumpPacket(QString &buf) const {
    std::ostringstream oss;
    buf.clear();
    for (auto c : packet)
        oss << std::hex << std::uppercase
            << std::setw(2) << std::setfill('0')
            << (uint32_t)c << " ";
    buf = oss.str().c_str();
}

void DSPacket::updateIpHeader(int protoSize, const uint8_t *ipPayload, uint32_t ipPayloadSize) {
    libnet_ptag_t ptag = 0;
    libnet_seed_prand(handle);

    if (family == AF_INET) {
        qDebug() << "building ipv4...";
        ptag = libnet_build_ipv4(LIBNET_IPV4_H + protoSize + ipPayloadSize,
                0, libnet_get_prand(LIBNET_PRu16), 0, ttl, proto, 0,
                libnet_name2addr4(handle,
                    const_cast<char *>(source.toStdString().c_str()), LIBNET_DONT_RESOLVE),
                libnet_name2addr4(handle,
                    const_cast<char *>(destination.toStdString().c_str()), LIBNET_DONT_RESOLVE),
                ipPayload, ipPayloadSize, handle, 0);
    } else if (family == AF_INET6) {
        qDebug() << "building ipv6...";
        ptag = libnet_build_ipv6(0, 0,
                protoSize, proto, ttl,
                libnet_name2addr6(handle,
                    source.toStdString().c_str(), LIBNET_DONT_RESOLVE),
                libnet_name2addr6(handle,
                    destination.toStdString().c_str(), LIBNET_DONT_RESOLVE),
                ipPayload, ipPayloadSize, handle, 0);
    }
    if (ptag == -1) qDebug() << "build ip header failed: " << libnet_geterror(handle);
}

void DSPacket::updateLinkHeader(const QVector<uint8_t> &src, const QVector<uint8_t> &dst, uint16_t type) {
    libnet_ptag_t ptag = 0;

    qDebug() << "building ethernet...";
    ptag = libnet_build_ethernet(dst.constData(), src.constData(),
                                 type, nullptr, 0, handle, 0);

    if (ptag == -1) qDebug() << "build ethernet header failed: " << libnet_geterror(handle);

}

void DSPacket::initIpHandle() {
    char errbuf[LIBNET_ERRBUF_SIZE];
    
    if (handle) libnet_destroy(handle);

    handle = libnet_init(family == AF_INET ? LIBNET_RAW4 : LIBNET_RAW6,
                device.toStdString().c_str(), errbuf);

    if (handle == nullptr) qDebug() << "libnet init ip failed: " << errbuf;
}

void DSPacket::initLinkHandle() {
    char errbuf[LIBNET_ERRBUF_SIZE];

    if (handle) libnet_destroy(handle);

    handle = libnet_init(LIBNET_LINK, device.toStdString().c_str(), errbuf);

    if (handle == nullptr) qDebug() << "libnet init link failed: " << errbuf;
}

void DSPacket::updatePacketData() {

    packet.clear();
    uint8_t *packetBuf = nullptr;
    uint32_t packetSize;

    libnet_pblock_coalesce(handle, &packetBuf, &packetSize);
    if (packetBuf == nullptr) {
        qDebug() << "get pblock failed";
        return;
    }
    std::copy_n(packetBuf, packetSize, std::back_inserter(packet));
    free(packetBuf - handle->aligner);
}
