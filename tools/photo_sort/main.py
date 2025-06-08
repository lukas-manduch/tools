import argparse
import hashlib
import os
import shutil
from pathlib import Path, PurePath
from datetime import datetime

from pillow_heif import register_heif_opener
from PIL import Image, ExifTags


register_heif_opener()

HELP_SOURCE = """Folder that will be searched for existing photos"""
HELP_DEST = """Folder where photost from SOURCE will be copied and renamed.
This can be the same folder as SOURCE."""

PHOTO_DATE_FORMAT = "%Y:%m:%d %H:%M:%S"
# E.g. 2019:09:28 16:23:33

def _get_date(path):
    with Image.open(path) as image:
        exif = image.getexif()
    datestring = exif.get(ExifTags.Base.DateTime, "")
    if len(datestring) > 0:
        photo_date = datetime.strptime(datestring, PHOTO_DATE_FORMAT)
    else:
        raise ValueError(f"Photo  {path} doesn't have date tag")
    return photo_date

def has_date(path):
    try:
        _get_date(path)
        return True
    except ValueError as e:
        return False


def compute_md5(path):
    with open(path, "rb") as f:
        data = f.read()
    h = hashlib.md5(data)
    return h.hexdigest()

def compute_name(base_path: Path, photo_path):
    photo_path = Path(photo_path)
    base_path = Path(base_path)
    extension = photo_path.suffix
    hashstring = compute_md5(photo_path)
    try:
        date = _get_date(photo_path)
        filename = f"{date.year}/{date.month}/{date.year}_{date.month}_{date.day}_{hashstring[:4]}{extension}"
    except ValueError:
        filename = f"unknown/{hashstring[:6]}{extension}"
    return PurePath.joinpath(base_path, filename)

def make_folders(path):
    os.makedirs(path, mode=0o770, exist_ok=True)

def copy_file(original, new_file):
    new_file = Path(new_file)
    if new_file.exists():
        hash1 = compute_md5(original)
        hash2 = compute_md5(new_file)
        if hash1 != hash2:
            raise Exception(f"NonMatching HASH {original} != {new_file}")
        return
    shutil.copyfile(original, new_file)

if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(
                        prog='photosort',
                        description='What the program does',
                        epilog='Text at the bottom of help')

        parser.add_argument('-s', '--source', help=HELP_SOURCE, required=True)
        parser.add_argument('-d', '--destination', help=HELP_DEST, required=True)
        args = parser.parse_args()
        source_dir = Path(args.source).resolve()
        destination_dir = PurePath(args.destination)
        if not source_dir.is_dir():
            raise ValueError(f"Source dir doesn't exist {source_dir}")
        count_photos = 0
        count_bad_photos = 0
        for filename in source_dir.glob("**/*"):
            if not filename.is_file():
                continue
            count_photos += 1
            if not has_date(filename):
                print(f"Found photo without dates {filename}")
                count_bad_photos += 1
            new_path = compute_name(destination_dir, filename)
            make_folders(PurePath(new_path).parent)
            copy_file(filename, new_path)
        print("-------------------------")
        print(f"TOTAL_PROCESSED:   \t{count_photos}")
        print(f"TOTAL_WITH_DATE:   \t{count_photos - count_bad_photos}")
        print(f"TOTAL_WITHOUT_DATE:\t{count_bad_photos}")

    except Exception as err:
        print(f"Error occured: {err}")
        raise


