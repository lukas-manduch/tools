#!/usr/bin/env python3

# Copyright 2025 Lukas Manduch


# TODO: Environment
# TODO: Fix exec flags
import argparse
import grp
import hashlib
import json
import os
import pathlib
import pwd
import re
import shutil
import subprocess
import sys

import yaml


global_files = [
    "/etc/passwd",
    "/etc/shadow",
    "/etc/pam.d",
    "/etc/pam.conf",
    "/etc/ssh",
    "/etc/init.d",
    "/usr/bin",
    "/usr/sbin",
    "/sbin/init/",
    "/lib/systemd/systemd",
    "/lib/systemd/systemd-ac-power",
    "/lib/systemd/systemd-backlight",
    "/lib/systemd/systemd-binfmt",
    "/lib/systemd/systemd-bless-boot",
    "/lib/systemd/systemd-boot-check-no-failures",
    "/lib/systemd/systemd-cgroups-agent",
    "/lib/systemd/systemd-cryptsetup",
    "/lib/systemd/systemd-fsck",
    "/lib/systemd/systemd-fsckd",
    "/lib/systemd/systemd-growfs",
    "/lib/systemd/systemd-hibernate-resume",
    "/lib/systemd/systemd-hostnamed",
    "/lib/systemd/systemd-initctl",
    "/lib/systemd/systemd-journald",
    "/lib/systemd/systemd-localed",
    "/lib/systemd/systemd-logind",
    "/lib/systemd/systemd-makefs",
    "/lib/systemd/systemd-modules-load",
    "/lib/systemd/systemd-network-generator",
    "/lib/systemd/systemd-networkd",
    "/lib/systemd/systemd-networkd-wait-online",
    "/lib/systemd/systemd-pstore",
    "/lib/systemd/systemd-quotacheck",
    "/lib/systemd/systemd-random-seed",
    "/lib/systemd/systemd-remount-fs",
    "/lib/systemd/systemd-reply-password",
    "/lib/systemd/systemd-resolved",
    "/lib/systemd/systemd-rfkill",
    "/lib/systemd/systemd-shutdown",
    "/lib/systemd/systemd-sleep",
    "/lib/systemd/systemd-socket-proxyd",
    "/lib/systemd/systemd-sulogin-shell",
    "/lib/systemd/systemd-sysctl",
    "/lib/systemd/systemd-sysv-install",
    "/lib/systemd/systemd-time-wait-sync",
    "/lib/systemd/systemd-timedated",
    "/lib/systemd/systemd-udevd",
    "/lib/systemd/systemd-update-utmp",
    "/lib/systemd/systemd-user-runtime-dir",
    "/lib/systemd/systemd-user-sessions",
    "/lib/systemd/systemd-veritysetup",
    "/lib/systemd/systemd-volatile-root",
    "/lib/systemd/systemd-xdg-autostart-condition",
        ]

def get_unit_section(content, section_name):
    """Return key value pairs from content, that are under [section_name]"""
    section_pattern = re.compile(r"^\s*\[(?P<name>\w+)\]\s*$")
    line_pattern = re.compile(r"^\s*(?P<key>\w+)=(?P<value>.+)$")
    lines_inside = list()
    is_inside = False
    for line in content.splitlines():
        match = section_pattern.match(line)
        if match:
            if match["name"] == section_name:
                is_inside = True
            else:
                is_inside = False
            continue
        if is_inside:
            lines_inside.append(line)
    results = list()
    for line in lines_inside:
        if match := line_pattern.match(line):
            results.append(
                (
                    match["key"],
                    match["value"],
                )
            )
    return results

def _cut_quotes(line, quote):
    position = line.find(quote)
    if position == -1:
        return None
    pos2 = line.find(quote, position+1)
    if pos2 == -1:
        return None
    return line[position+1: pos2], line[pos2+1:]

def _try_remove_systemd_specials(path):
    """Remove special systemd symbols that are before exec commands"""
    return str(path).lstrip("-+!:@")



def get_some_bytes(absolute):
    filename = FileHasher.try_get_absolute(absolute)
    try:
        with open(filename, "rb") as f:
            content = f.read(128)
            try:
                return content.decode('utf-8')
            except Exception:
                return "[binary]"
    except Exception as e:
        return str(e)

def _try_get_acl(absolute: pathlib.Path):
    # At first check if getfacl exists. If it doesn't, subprocess.run cannot
    # be called, or it will throw Exception.
    getfacl = "/usr/bin/getfacl"
    if not pathlib.Path(getfacl).is_file():
        return "[acl_failed]"
    result = subprocess.run([getfacl, str(absolute)], capture_output=True, timeout=15, check=False)
    if result.returncode != 0:
        return "[acl_failed]"
    perm_pattern = re.compile(r"^\w+:\w+:.*$")
    ret = ""
    for line in result.stdout.decode("utf-8").split("\n"):
        print(line)
        if perm_pattern.match(line):
            ret += line 
    if ret:
        return "ACL_EXTRA " + ret
    return "ACL_NONE"

def get_file_permissions(absolute):
    """File permissions."""
    result = ""
    try:
        absolute = pathlib.Path(absolute)
        stats = absolute.stat()
        result += pwd.getpwuid(stats.st_uid)[0]
        result += ":"
        result += grp.getgrgid(stats.st_gid)[0]
        result += ":"
        result += oct(stats.st_mode)

        result += "   "
        result += _try_get_acl(absolute)
    except Exception as e:
        result = f"Error {e}"
    return result

class FileHasher:

    def __init__(self):
        self.short_names = dict()
        self.exe_hashes = dict()
        self.file_hashes = dict()

    def get(self, name):
        full_path = None
        hash_target = self.file_hashes
        # Handle cached
        if full_p:= self.short_names.get(name):
            if full_p in self.exe_hashes:
                return self.exe_hashes[full_p]
            return self.file_hashes[full_p] or None
        # Try to resolve name
        if full_path == None:
            full_path = FileHasher.try_find_executable(name)
            hash_target = self.exe_hashes
        if full_path == None:
            full_path = FileHasher.try_get_absolute(name)

        if not full_path:
            return None


        self.short_names[name] = full_path
        hexdigest = self._compute_hash(full_path)
        # Cache
        hash_target[full_path] = hexdigest

        return hexdigest


    def _compute_hash(self, path):
        try:
            hasher = hashlib.md5()
            with open(path, "rb") as f:
                while chunk := f.read(4096):
                    hasher.update(chunk)
            return hasher.hexdigest()
        except Exception:
            return None


    def _to_exe(self, path):
        path = shutil.which(name)
        if path == None:
            return None
        self.short_names[name] = path
        hexdigest = self._compute_hash(path)
        self.exe_hashes[path] = hexdigest
        return hexdigest

    def _to_filename(self, path):
        path = pathlib.Path(path)
        if path.exists():
            return str(path)
        return None


    @staticmethod
    def try_get_absolute(command: str):
        """Return None on failure"""
        path = pathlib.Path(command)
        if not path.exists():
            return None
        if path.is_dir():
            return None
        return str(path.absolute())

    @staticmethod
    def try_find_executable(command: str):
        """Return None on failure"""
        return shutil.which(str(command))

global_hasher = FileHasher()

########################## BEGIN SYSTEMD ##################################

def cut_command_line(command: str):
    # First handle quotes
    parts = list()
    results = (None, str(command), )
    # Double quotes
    while True:
        results = _cut_quotes(results[1], '"')
        if results == None:
            break
        piece = results[0]
        parts.append(piece)
    # Single quotes
    results = (None, str(command), )
    while True:
        results = _cut_quotes(results[1], "'")
        if results == None:
            break
        piece = results[0]
        parts.append(piece)
    # Spaces
    parts += command.split(sep=" ")
    clean_parts = list()

    for part in parts:
        clean_part = _try_remove_systemd_specials(part)
        if part != clean_part:
            clean_parts.append(clean_part)

    return parts + clean_parts

class SystemdService:

    class ExecCommand:

        def __init__(self, command_line):
            self.command = command_line
            self.hashes = list()

        def to_dict(self):
            result = dict()
            result["command"] = self.command
            result["hashes"] = self.get_paths()
            return result

        def get_paths(self):
            """Try to find executables inside command"""
            result = list()
            for cut_command in cut_command_line(str(self.command)):
                if hash := global_hasher.get(cut_command):
                    result.append({'name': cut_command, "hash": hash})
            return result



    def __init__(self):
        self.name = ""
        self.enabled = False
        self.state = "[missing]"
        self.commands = list()
        self._queried = False

    @classmethod
    def from_dict(cls, dictionary):
        service = cls()
        service.name = dictionary["unit_file"]
        service.enabled = dictionary["state"]
        service.state = dictionary["state"]
        return service

    def to_dict(self):
        commands = list()
        result = dict()
        result["unit"] = self.name
        result["state"] = self.state
        for command in self.commands:
            commands.append(command.to_dict())
        result["commands"] = commands
        return result

    def enabled(self):
        return self.state == "enabled"

    def __str__(self):
        return f"{self.name}\t{self.enabled}"

    @staticmethod
    def parse_cat(ini_content):
        section_lines = get_unit_section(ini_content, "Service")
        results = list()
        for values in section_lines:
            if values[0] in ["ExecStart", "ExecStartPre", "ExecStartPost", "ExecReload", "ExecStop", "ExecStopPost"]:
                results.append(values[1])
        return results

    def query_details(self):
        if self._queried:
            return

        command = "systemctl --full --no-pager cat".split()
        command.append(self.name)
        result = subprocess.run(command, capture_output=True, timeout=5, check=True)
        self._queried = True
        command_lines = self.parse_cat(result.stdout.decode())
        for command in command_lines:
            self.commands.append(SystemdService.ExecCommand(command))


def _call_list_services():
    command = "systemctl --full --no-pager list-unit-files --type=service -o json"
    result = subprocess.run(command.split(), capture_output=True, timeout=15, check=True)
    return result.stdout

############################# END SYSTEMD ##################################

########################## BEGIN FILE HASHING ###############################
def main_services():
    services_json = _call_list_services()
    services_obj = json.loads(services_json)
    services = list()
    for service_json in services_obj:
        service = SystemdService.from_dict(service_json)
        service.query_details()
        services.append(service.to_dict())
    return services

def _try_get_path_file(path):
    result = dict()
    path = pathlib.Path(path)
    absolute = str(path.absolute())
    result["path"] = absolute
    try:
        result["size"] = path.stat().st_size
        result["hash"] = global_hasher.get(absolute) or "None"
        result["sample"] = get_some_bytes(absolute)
        result["permissions"] = get_file_permissions(absolute)
    except Exception as e:
        result["error"] = str(e)
    return result

def _try_get_path_dir(path):
    result = list()
    path = pathlib.Path(path)
    if not path.is_dir():
        return result
    for entry in os.walk(path):
        for fname in entry[2]:
            pure_path = pathlib.PurePath(entry[0]).joinpath(fname)
            result.append(_try_get_path_file(str(pure_path)))
    return result


def _try_get_path(path):
    """ Returns array of objects"""
    path = pathlib.Path(path)
    if not path.exists():
        return list()
    if path.is_dir():
        return _try_get_path_dir(path)
    return [_try_get_path_file(path)]

########################## END FILE HASHING ###############################

def _get_proc_details(folder_path):
    result = dict()
    folder_path = pathlib.PurePath(folder_path)

    result["cmd"] = get_some_bytes(str(folder_path.joinpath("cmdline")))
    result["comm"] = get_some_bytes(str(folder_path.joinpath("comm"))).strip()
    result["hash"] = global_hasher.get(str(folder_path.joinpath("exe")))
    return result

def main_proc():
    proc = pathlib.Path("/proc")
    proc_pattern = re.compile(r"^/proc/\d+/?$")
    result = list()
    for dire in proc.iterdir():
        if proc_pattern.match(str(dire)):
            result.append(_get_proc_details(dire))
    return result

def main_files():
    results = list()
    for path in global_files:
        results += _try_get_path(path)
    return results


def main(args):
    result = dict()
    result["issue"] = get_some_bytes("/etc/issue").strip()
    result["hostname"] = get_some_bytes("/etc/hostname").strip()
    if args.systemd:
        services = main_services()
        result["systemd"] = services

    if args.files:
        result["files"] = main_files()

    if args.proc:
        result["proc"] = main_proc()
    print(yaml.dump(result))



if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Systemd config extractor")
    parser.add_argument(
        "--no-systemd",
        action="store_const",
        default=True,
        help="Disable systemd collection",
        const=False,
        dest="systemd",
    )
    parser.add_argument(
        "--no-files",
        action="store_const",
        default=True,
        help="Disable file enumeration",
        const=False,
        dest="files",
    )
    parser.add_argument(
        "--no-proc",
        action="store_const",
        default=True,
        help="Dont iterate proc",
        const=False,
        dest="proc",
    )
    args = parser.parse_args()
    #service = SystemdService()
    #service.name = "cockpit.service"
    #service.query_details()
    #print(yaml.dump(service.to_dict()))
    main(args)
