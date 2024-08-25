#!/usr/bin/env python3

import os
import subprocess
import re

def get_crc32(file_path, field_name):
    """
    Executes the packet_info command and extracts the CRC32 value for the specified field.
    
    :param file_path: Path to the .traceboy file.
    :param field_name: Field name whose CRC32 value we want to extract.
    :return: CRC32 value as a string.
    """
    # Run the packet_info command
    result = subprocess.run(['bin/packet_info', file_path], stdout=subprocess.PIPE, text=True)
    
    # Extract the relevant CRC32 value using regex
    pattern = rf'{field_name}: ([0-9a-f]+)'
    match = re.search(pattern, result.stdout)
    
    if match:
        return match.group(1)
    else:
        raise ValueError(f"{field_name} not found in file: {file_path}")

def find_matching_files(target_file, directory):
    """
    Finds files in the directory where start_state_crc32 matches the end_state_crc32 of the target file.
    
    :param target_file: Path to the target .traceboy file.
    :param directory: Directory containing other .traceboy files to search through.
    """
    try:
        # Get the end_state_crc32 from the target file
        target_end_crc32 = get_crc32(target_file, 'end_state_crc32')
    except ValueError as e:
        print(e)
        return

    # Iterate over each file in the directory
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".traceboy"):
                file_path = os.path.join(root, file)
                try:
                    # Get the start_state_crc32 from the current file
                    start_state_crc32 = get_crc32(file_path, 'start_state_crc32')
                    
                    # Check if the CRC32 values match
                    if start_state_crc32 == target_end_crc32:
                        print(f"Match found: {file_path}")
                except ValueError:
                    # Skip files that don't contain the necessary fields
                    continue

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) != 3:
        print("Usage: python script.py <path_to_target_traceboy_file> <directory_of_traceboy_files>")
    else:
        target_traceboy_file = sys.argv[1]
        traceboy_directory = sys.argv[2]
        find_matching_files(target_traceboy_file, traceboy_directory)
