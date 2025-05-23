import requests
import os
from tqdm import tqdm
from pathlib import Path
import py7zr
import zipfile
import argparse
import shutil


def make_directory(path):
    if not os.path.exists(path):
        os.makedirs(path)


def extract_7z(input_file, output_folder):
    with py7zr.SevenZipFile(input_file, mode='r') as z:
        z.extractall(path=output_folder)
    print(f'Extraction complete. Files extracted to {output_folder}')


def extract_zip(input_file, output_folder):
    with zipfile.ZipFile(input_file, 'r') as zip_ref:
        zip_ref.extractall(output_folder)


def extract_any(input_file, output_folder):

    print("[extract] {0} => {1}".format(input_file, output_folder))

    if input_file.endswith(".7z"):
        extract_7z(input_file, output_folder)
    elif input_file.endswith(".zip"):
        extract_zip(input_file, output_folder)
    else:
        raise ValueError("Unsupported file format")


def download_file(url, output_path):
    response = requests.get(url, stream=True)

    with open(output_path, "wb") as handle:
        for data in tqdm(response.iter_content()):
            handle.write(data)


def pull_library_files(url, output_folder):

    if os.path.exists(output_folder):
        print(
            "[pull_library_files] output_folder [{0}] is already exists, skip download".format(output_folder))
        return

    compress_file_path = "build/.tmp/vendor/" + os.path.basename(url)
    make_directory(output_folder)

    print("[pull_library_files] {0}".format(compress_file_path))

    if os.path.exists(compress_file_path):
        print(
            "[pull_library_files] compress_file [{0}] is already exists, skip download".format(compress_file_path))
    else:
        download_file(url, compress_file_path)

    extract_any(compress_file_path, output_folder)


def clear_all_vendor_files():
    shutil.rmtree("vendor/libusb")
    shutil.rmtree("vendor/libusb_hid_api")
    shutil.rmtree("vendor/openssl")
    shutil.rmtree("build/.tmp/vendor")


def download_all_vendor_files():
    make_directory("build/.tmp/vendor/")

    pull_library_files(
        "https://github.com/libusb/libusb/releases/download/v1.0.27/libusb-1.0.27.7z",
        "vendor/libusb")

    pull_library_files(
        "https://github.com/libusb/hidapi/releases/download/hidapi-0.14.0/hidapi-win.zip",
        "vendor/libusb_hid_api")

    pull_library_files(
        "https://wiki.overbyte.eu/arch/openssl-1.1.1w-win64.zip",
        "vendor/openssl")


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='QDAP vendor manage script')
    subparsers = parser.add_subparsers(dest='command', help='sub command')
    parser_greet = subparsers.add_parser(
        name='clean', help="clean all vendor files")
    parser_greet = subparsers.add_parser(
        name='download', help="download all vendor files")
    args = parser.parse_args()

    if args.command == 'clean':
        clear_all_vendor_files()
    elif args.command == 'download':
        download_all_vendor_files()
    else:
        parser.print_help()

    # make_directory("build/vendor/")

    # url = "https://github.com/libusb/libusb/releases/download/v1.0.27/libusb-1.0.27.7z"
    # compress_file_path = "build/vendor/" + os.path.basename(url)
    # make_directory("vendor/libusb")
    # print("[vendor] libusb: {0}".format(compress_file_path))
    # download_file(url, compress_file_path)
    # extract_7z(compress_file_path, "vendor/libusb")

    # make_directory("vendor/libusb_hid_api")
    # url = "https://github.com/libusb/hidapi/releases/download/hidapi-0.14.0/hidapi-win.zip"
    # compress_file_path = "build/vendor/" + os.path.basename(url)
    # print("[vendor] libusb-hid-api: {0}".format(compress_file_path))
    # download_file(url, compress_file_path)
    # extract_zip(compress_file_path, "vendor/libusb_hid_api")
