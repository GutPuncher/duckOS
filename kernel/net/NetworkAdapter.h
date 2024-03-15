/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "../api/net.h"
#include "../kstd/string.h"
#include "../kstd/vector.hpp"
#include "../Result.hpp"
#include "../memory/SafePointer.h"
#include "ARP.h"
class NetworkAdapter: public kstd::ArcSelf<NetworkAdapter> {
public:
	virtual ~NetworkAdapter() = default;
	static void setup();

	struct Packet {
		uint8_t buffer[8192]; /* TODO: We need non-constant packet sizes... */
		union {
			size_t size;
			Atomic<bool> used = false;
		};
		Packet* next = nullptr;
	};

	struct FrameHeader {
		MACAddress destination;
		MACAddress source;
		BigEndian<uint16_t> type;
		uint32_t payload[];
	} __attribute__((packed));

	void send_arp_packet(MACAddress dest, const ARPPacket& packet);
	void send_raw_packet(SafePointer<uint8_t> bytes, size_t count);
	void send_packet(Packet* packet);
	Packet* dequeue_packet();
	Packet* alloc_packet(size_t size);
	IPv4Packet* setup_ipv4_packet(Packet* packet, const MACAddress& dest, const IPv4Address& dest_addr, IPv4Proto proto, size_t payload_size, uint8_t dscp, uint8_t ttl);

	[[nodiscard]] IPv4Address ipv4_address() const { return m_ipv4_addr; }
	[[nodiscard]] MACAddress mac_address() const { return m_mac_addr; }
	[[nodiscard]] IPv4Address netmask() const { return m_ipv4_netmask; }
	const kstd::string& name() const;

	static ResultRet<kstd::Arc<NetworkAdapter>> get_interface(const kstd::string& name);
	static const kstd::vector<kstd::Arc<NetworkAdapter>>& interfaces();

protected:
	explicit NetworkAdapter(kstd::string name);
	static void register_interface(kstd::Arc<NetworkAdapter> adapter);

	void set_mac(MACAddress addr);
	void set_ipv4(IPv4Address addr);
	void set_netmask(IPv4Address mask);

	virtual void send_bytes(SafePointer<uint8_t> bytes, size_t count) = 0;
	void receive_bytes(SafePointer<uint8_t> bytes, size_t count);

private:
	static kstd::vector<kstd::Arc<NetworkAdapter>> s_interfaces;

	kstd::string m_name;
	IPv4Address m_ipv4_addr {10, 0, 2, 15};
	IPv4Address m_ipv4_netmask = {0, 0, 0, 0};
	MACAddress m_mac_addr;
	Packet* m_packet_queue = nullptr;
	Packet m_packets[32];
};
