import argparse
import json
import sys
import subprocess
import re

import yaml


def get_unit_section(content, section_name):
    """Return key value pairs from content, that are under [section_name]"""
    section_pattern = re.compile(r'^\s*\[(?P<name>\w+)\]\s*$')
    line_pattern = re.compile(r'^\s*(?P<key>\w+)=(?P<value>.+)$')
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
            results.append((match["key"], match["value"],))
    return results





class SystemdService():

    class ExecCommand:

        def __init__(self, command_line):
            self.command = command_line


        def to_dict(self):
            result = dict()
            result["command"] = self.command
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
            if values[0] in ["ExecStart", "ExecStartPre", "ExecStartPost"]:
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
    result = subprocess.run(command.split(), capture_output=True, timeout=5, check=True)
    return result.stdout


def main():
    services_json = _call_list_services()
    services_obj = json.loads(services_json)
    services = list()
    for service_json in services_obj:
        service = SystemdService.from_dict(service_json)
        service.query_details()
        services.append(service.to_dict())
    print(yaml.dump(services))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Systemd config extractor")
    parser.add_argument("--no-disabled", action="store_const", default=False, help="Only parse enabled units", const=True, dest="enabled")
    parser.add_argument("--no-hash", action="store_const", default=True, help="Compute hash for paths", const=False, dest="hash")
    parser.parse_args()

    service = SystemdService()
    service.name = "borg.service"
    service.query_details()
    #print(yaml.dump(service.to_dict()))
    main()



