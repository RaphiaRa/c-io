# SPDX-FileCopyrightText: 2025 c-io Contributers
#
# SPDX-License-Identifier: MPL-2.0

# Simple script to amalgamate C header files into a single file.
# Don't use this in other projects, it's just a simple script specific to this project.

import os
import re
import argparse
import networkx as nx

logging = False
include_dir = "include"
license_header = ""
got_license_header = False

def read_file(filepath):
    """Read the content of a file."""
    with open(filepath, "r") as file:
        return file.read()


def write_file(filepath, content):
    """Write content to a file."""
    with open(filepath, "w") as file:
        file.write(content)


def log(message):
    """Log a message to the amalgmate.log file."""
    if not logging:
        return
    with open("amalgamate.log", "a") as log_file:
        log_file.write(message + "\n")


def extract_includes(content):
    """Extract #include directives from content."""
    includes = set()
    include_pattern = re.compile(r'#include\s*<(io.*?)>')
    for match in include_pattern.finditer(content):
        includes.add(match.group(1))
    return includes


def build_include_graph(includes):
    """Build a directed graph from the include directives."""
    graph = nx.DiGraph()
    for include in includes:
        dependencies = extract_includes(read_file(include))
        # Get the top-level directory of the include file
        dependencies = [os.path.join(include_dir, dep) for dep in dependencies]
        # Order the dependencies lexicographically to ensure a deterministic order
        dependencies = sorted(dependencies)
        log(f"Dependencies for {include}: {dependencies}")
        for dependency in dependencies:
            graph.add_edge(include, dependency)
    return graph


def get_ordered_includes(includes):
    """Order the includes based on the dependencies."""
    graph = build_include_graph(includes)
    return list(reversed(list(nx.topological_sort(graph))))


def order_includes(include_files):
    """Collect all local headers used in the source files."""

    # We assume that all files are in the same directory
    #dir = os.path.dirname(include_files[0])
    # First, order the includes lexicographically, to ensure a deterministic order
    include_files = sorted(include_files)
    log("All includes: " + str(include_files))
    #log("Finding ordered includes in " + dir)
    ordered = get_ordered_includes(include_files)
    log("Ordered includes: " + str(ordered))

    # Some headers are not included by other headers. 
    # We need to manually add them
    # to the ordered list
    for include in include_files:
        if include not in ordered:
            ordered.append(include)
    return ordered


def remove_header_guards(content):
    """Remove header guards from content."""
    lines = content.splitlines()
    new_lines = []
    state = "find_first_ifndef"
    guard_name = None
    depth = 0
    for line in lines:
        if state == "find_first_ifndef":
            # Ignore all leading lines until we find the first #ifndef directive
            if not line.startswith("#ifndef"):
                new_lines.append(line)
                continue
            match = re.match(r"#ifndef\s+(.*)[//.*]{0,1}", line)
            if match:
                guard_name = match.group(1)
                state = "find_first_define"
            else:
                raise ValueError("Expected header guard #ifndef directive")
        elif state == "find_first_define":
            # We expect the next line to be the #define directive
            match = re.match(r"#define\s+(.*)[//.*]{0,1}", line)
            if match and match.group(1) == guard_name:
                state = "find_endif"
            else:
                raise ValueError("Expected header guard #define directive")
        elif state == "find_endif":
            # Count the number of nested #ifndef, #ifdef and #if directives
            # If we're at the same level as the first #ifndef directive
            # then we've found the end of the header guard
            if (
                line.startswith("#ifndef")
                or line.startswith("#ifdef")
                or line.startswith("#if")
            ):
                depth += 1
                new_lines.append(line)
            elif line.startswith("#endif"):
                if depth == 0:
                    state = "done"
                else:
                    depth -= 1
                    new_lines.append(line)
            else:
                new_lines.append(line)
        elif state == "done":
            new_lines.append(line)
    return "\n".join(new_lines)

def cleanup_includes(content):
    """Remove obsolete #include directives from content."""
    lines = content.splitlines()
    new_lines = []
    for line in lines:
        if line.startswith("#include") and re.search(r'#include\s*<io.*?>', line):
            continue
        else:
            new_lines.append(line)

    return "\n".join(new_lines)

def remove_license_header(content):
    """Remove the header, we'll add it back later."""
    # We simply gonna remove the top comment block
    lines = content.splitlines()
    new_lines = []
    license_lines = []
    if not lines[0].startswith("/*"):
        return content
    in_license = True
    for line in lines:
        if in_license:
            license_lines.append(line)
            if line.strip() == "*/":
                in_license = False
                continue
        else:
            new_lines.append(line)
    global got_license_header
    if not got_license_header:
        global license_header
        license_header = "\n".join(license_lines)
        got_license_header = True
    return "\n".join(new_lines)

def amalgamate(output_file):
    """Amalgamate C project files into a single file."""
    amalgamated_content = ""
    # Top level includes and defines
    amalgamated_content += "#ifndef IO_IO_H\n"
    amalgamated_content += "#define IO_IO_H\n"
    amalgamated_content += "#define IO_WITH_AMALGAMATION 1\n\n"

    # Find all header files in the include directory recursively
    include_files = []
    for root, _, files in os.walk(include_dir):
        for file in files:
            # Filter out non-header files
            if not file.endswith(".h"):
                continue
            include_files.append(os.path.join(root, file))
    # Find local headers used in the source files
    sorted_headers = order_includes(include_files)

    # Remove #include directives from the local headers
    # and write the content to the amalgamated file
    for header in sorted_headers:
        log(f"Adding {header} to amalgamated file")
        content = read_file(header)
        content = remove_license_header(content)
        content = cleanup_includes(content)
        content = remove_header_guards(content)
        amalgamated_content += f"/* Start of {header} */\n"
        amalgamated_content += content + "\n"
        amalgamated_content += f"/* End of {header} */\n"
    amalgamated_content += "#endif\n"
    amalgamated_content = license_header + "\n\n" + amalgamated_content
    # Write the final amalgamated content to the output file
    write_file(output_file, amalgamated_content)
    print(f"Amalgamated file created at {output_file}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Amalgamate C source files into a single file."
    )
    parser.add_argument(
        "-i",
        "--include-dir",
        default="include",
        help="The directory containing the header files.",
    )
    parser.add_argument(
        "-o",
        "--output",
        default="amalgamated.c",
        help="The output file for the amalgamated code.",
    )

    args = parser.parse_args()
    include_dir = args.include_dir
    amalgamate(args.output)
