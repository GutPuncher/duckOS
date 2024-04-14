/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#include <libdebug/LiveDebugger.h>
#include <csignal>
#include <memory>
#include <sys/wait.h>
#include <libduck/FormatStream.h>
#include <libduck/Args.h>
#include <ctime>

using namespace Debug;
using Duck::OutputStream;

int pid;
int interval = 10;
int duration = 5000;
std::string filename;

struct Thread {
	tid_t tid;
	LiveDebugger debugger;
	std::vector<std::vector<size_t>> stacks;
};

Duck::Result profile() {
	auto proc = TRY(Sys::Process::get(pid));

	// First, attach to all the threads
	const int loops = duration / interval;
	std::vector<Duck::Ptr<Thread>> threads;
	for (auto tid : proc.threads()) {
		auto thread = std::make_shared<Thread>();
		thread->tid = tid;
		thread->stacks.reserve(loops);
		auto attach_res = thread->debugger.attach(pid, tid);
		if (attach_res.is_error())
			Duck::printerrln("Warning: Failed to attach to thread {}: {}", tid, attach_res);
		else
			threads.push_back(thread);
	}

	if (threads.empty())
		return Duck::Result("Could not attach to any threads");

	Duck::println("Sampling {} for ~{}ms...", proc.name(), duration);
	for (size_t i = 0; i < loops; i++) {
		waitpid(pid, nullptr, 0);
		for (auto& thread : threads)
			thread->stacks.push_back(TRY(thread->debugger.walk_stack_unsymbolicated()));
		kill(pid, SIGCONT);
		usleep(interval * 1000);
		kill(pid, SIGSTOP);
	}
	waitpid(pid, nullptr, 0);
	kill(pid, SIGCONT);

	Duck::println("Done! Symbolicating and dumping...");
	std::map<size_t, AddressInfo> symbols;

	if (filename.empty())
		filename = "profile-" + proc.name() + "-" + std::to_string(std::time(nullptr)) + ".txt";
	auto out = TRY(Duck::File::open(filename, "w"));
	Duck::FileOutputStream stream {out};

	for (auto& thread : threads) {
		for (auto& stk : thread->stacks) {
			stream << "thread " << thread->tid << ";";
			for(size_t i = stk.size(); i > 0; i--) {
				auto pos = stk[i - 1];
				auto info = symbols.find(pos);
				if (info == symbols.end()) {
					auto info_res = thread->debugger.info_at(pos);
					if(info_res.is_error())
						symbols[pos] = {"???", pos, nullptr};
					else
						symbols[pos] = info_res.value();
					info = symbols.find(pos);
				}

				auto& symbol = info->second;
				if (!symbol.object)
					stream % "?? @ {#x}" % symbol.symbol_offset;
				else if(symbol.symbol_name == "__syscall_trap__")
					stream << "<kernel>";
				else
					stream % "{} @ {}" % symbol.symbol_name % symbol.object->name;

				if (i != 1)
					stream << ";";
			}
			stream << " 1\n";
		}
	}

	out.close();

	Duck::println("Done! Saved to {}.", filename);
	return Duck::Result::SUCCESS;
}

int main(int argc, char** argv) {
	Duck::Args args;
	args.add_positional(pid, true, "pid", "The PID of the program to profile.");
	args.add_named(interval, "i", "interval", "The interval with which to sample, in ms. (Default: 10)");
	args.add_named(duration, "d", "duration", "The duration to sample for, in ms. (Default: 5000)");
	args.add_named(filename, "o", "output", "The output file.");
	args.parse(argc, argv);

	auto res = profile();
	if (res.is_error()) {
		printf("Error: %s\n", res.message().c_str());
		return res.code();
	}
	return 0;
}