import logging
import os
import socket
import sys
import time
import threading
from logging.handlers import SysLogHandler
from typing import Iterable

def create_message(number: int, keyword=None) -> str:
    def _f():
        if keyword:
            return f"[{keyword}]"
    return f"This is message which is now somewhat longer than it used to be with serial number {number}{_f()}\n"

def get_messages(total: int, keyword=None) -> Iterable[str]:
    for i in range(0, total):
        yield create_message(i, keyword)

def write_to_file(filename: str):
    with open(filename, "w") as f:
        f.writelines(get_messages(1000*1000*100)) # 4.7gb

def dump_messages(logger, messages):
    for message in messages:
        logger.info(message)

def syslogtest(server="journal"):
    logger = logging.getLogger("LogTestLogger")
    logger.setLevel(logging.DEBUG)
    log_fmt = logging.Formatter("%(levelname)s %(message)s")
    handler = SysLogHandler(address="/dev/log")
    if server == "journal":
        pass
    elif server == "syslog":
        handler = SysLogHandler(address="/run/systemd/journal/syslog")
    elif server == "ekanite":
        handler = SysLogHandler(
                address=("localhost", 4444),
                socktype=socket.SOCK_STREAM)
    else:
        print("Unsupported option")
        raise ValueError(f"Unsupported server {server}")
    handler.setLevel(logging.DEBUG)
    handler.setFormatter(log_fmt)
    logger.addHandler(handler)

    num_messages = 1000*1000*500
    dump_messages(logger, get_messages(num_messages, "rsystestpre"))


def _get_kill_internal(seconds: int):
    def _kill_local():
        time.sleep(seconds)
        os._exit(0)
    return _kill_local

def kill_me_after(seconds: int):
    thread = threading.Thread(target=_get_kill_internal(seconds))
    print("starting thread")
    thread.start()
    print("started")
    return thread


if __name__ == "__main__":
    assert len(sys.argv) == 2, "Requires filename argument, or syslog"
    print("Syslog test")
    thread = kill_me_after(10)
    syslogtest(sys.argv[1])
    thread.join()
    # else:
    #     print("unsupported
    #     exit(1)
    #     begin_write = time.time()
    #     write_to_file(sys.argv[1])
    #     end_write =  time.time()
    #     print(f"Total write time {end_write - begin_write}")

