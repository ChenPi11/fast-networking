#!/bin/env python3
# -*- mode: python -*-
# vi: set fileencoding=utf-8
# vi: set ft=python sts=4 ts=4 sw=4 et:
# * This file is part of the Fast Networking.
# *
# * The Fast Networking Library is free software; you can redistribute it
# * and/or modify it under the terms of the GNU Lesser General Public
# * License as published by the Free Software Foundation; either version 3
# * of the License, or (at your option) any later version.
# *
# * The Fast Networking Library is distributed in the hope that it will be
# * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# * Lesser General Public License for more details.
# *
# * You should have received a copy of the GNU Lesser General Public
# * License along with the Fast Networking Library; see the file LICENSE.
# * If not, see <https://www.gnu.org/licenses/>.

"""Utility to mix all C sources files into one."""

import argparse
import datetime
import pathlib
import re
import sys
import typing

from align import COMMENT_NOTE_C, align_center

__all__ = [
    "FILENAME_PATTERN",
    "MixerError",
    "MixerFindError",
    "MixerInvalidIndexError",
    "MixerPreprocessError",
    "MixerPreprocessNoEndError",
    "find_c_files",
    "mix_c_files",
    "preprocess_c_file",
]

__version__ = "0.1.0"
__author__ = "ChenPi11"
__license__ = "LGPL-3.0-or-later"
__copyright__ = f"2025, {__author__}"


# We scan all files and directories recursively in "<0-9><0-9>-*" pattern.
FILENAME_PATTERN = re.compile(r"^(\d{2})-.*$")


def _is_int(s: str) -> bool:
    try:
        int(s)
        return True  # noqa: TRY300
    except ValueError:
        return False


class MixerError(Exception):
    """Custom exception for mixer errors."""


class MixerFindError(MixerError):
    """Custom exception for mixer find errors."""


class MixerInvalidIndexError(MixerFindError, ValueError):
    """Custom exception for mixer invalid index errors."""


class MixerPreprocessError(MixerError, ValueError):
    """Custom exception for mixer preprocess errors."""


class MixerPreprocessNoEndError(MixerPreprocessError):
    """Custom exception for mixer preprocess no end errors."""


def find_c_files(directory: pathlib.Path) -> list[pathlib.Path]:
    """Find all C source files in the given directory.

    Args:
        directory (pathlib.Path): The directory to search.

    Returns:
        list[pathlib.Path]: Sorted list of C source file paths.

    """
    objects: list[pathlib.Path] = [
        obj
        for obj in directory.rglob("*", recurse_symlinks=True)
        if (
            obj.is_dir()
            or (
                obj.is_file()
                and obj.suffix.lower()
                in {
                    ".c",
                    ".h",
                }
            )
        )
        and FILENAME_PATTERN.match(obj.name)
    ]
    objects_vec: list[tuple[pathlib.Path, int]] = []
    for obj in objects:
        m = FILENAME_PATTERN.match(obj.name)
        if m:
            idx = m.group(1)
            if not _is_int(str(idx)):
                msg = f"Invalid index {idx!r} in filename {obj.name!r}."
                raise MixerInvalidIndexError(msg)
            objects_vec.append((obj, int(str(idx))))

    # Sort by index first, then by name.
    objects_vec.sort(key=lambda x: (x[1], x[0].name))

    files: list[pathlib.Path] = []
    for obj, _ in objects_vec:
        if obj.is_file():
            files.append(obj)
        elif obj.is_dir():
            files.extend(find_c_files(obj))
        else:
            msg = f"Unknown object type: {obj!r}."
            raise MixerFindError(msg)

    return files


def preprocess_c_file(file: pathlib.Path) -> str:
    """Preprocess a C source with my special rules.

    Args:
        file (pathlib.Path): The C source file path.

    Returns:
        str: The preprocessed C source code.

    """
    code: str = ""
    no_merge: bool = False
    with file.open("r", encoding="utf-8") as f:
        for line in f:
            # Skip include directives.
            pattern = line.strip().replace(" ", "").replace("\t", "")
            if pattern == "/*@NOMRG*/":
                no_merge = True
                continue
            if pattern == "/*@ENDNOMRG*/":
                no_merge = False
                continue

            if not no_merge:
                code += line
                continue

    if no_merge:
        msg = f"Not closed /*@NOMRG*/ in file {file!s}."
        raise MixerPreprocessNoEndError(msg)

    return code


def mix_c_files(
    directory: pathlib.Path,
    output: typing.TextIO,
    *,
    log_file: typing.TextIO | None = None,
    verbose_log: bool = False,
) -> None:
    """Mix all C source files into one.

    Args:
        directory (pathlib.Path): The directory to search for C source files.
        output (typing.TextIO): The output file object.
        log_file (typing.TextIO | None): The log file object. Defaults to None.
        verbose_log (bool): Whether to enable verbose logging.
            Defaults to False.

    """
    files = find_c_files(directory)
    for file in files:
        relpath = file.relative_to(directory)
        title = align_center(str(relpath), comment_note=COMMENT_NOTE_C)
        output.write(f"{title}\n")
        code = preprocess_c_file(file)
        output.write(code)
        output.write("\n\n")
        if log_file:
            if verbose_log:
                log_file.write(f"{relpath!s}\n")
            else:
                log_file.write(".")
            log_file.flush()


def mix_c_dirs(
    directories: list[pathlib.Path],
    output: typing.TextIO,
    *,
    log_file: typing.TextIO | None = None,
    verbose_log: bool = False,
) -> None:
    """Mix all C source files from multiple directories into one.

    Args:
        directories (list[pathlib.Path]): The directories to search for C
            files.
        output (typing.TextIO): The output file object.
        log_file (typing.TextIO | None): The log file object. Defaults to None.
        verbose_log (bool): Whether to enable verbose logging.
            Defaults to False.

    """
    for directory in directories:
        title = (
            align_center(
                f"Directory: {directory.name}",
                comment_note=COMMENT_NOTE_C,
            )
            + "\n"
        )
        output.write(title)
        mix_c_files(
            directory,
            output,
            log_file=log_file,
            verbose_log=verbose_log,
        )


def get_mixed_header() -> str:
    """Get the mixed file header.

    Returns:
        str: The mixed file header.

    """
    now = datetime.datetime.now(tz=datetime.timezone.utc)
    timestr = now.strftime("%Y-%m-%d %H:%M:%S %Z")
    header_lines = [
        "/*",
        " * This file is auto-generated by the mixer utility.",
        f" * Generated on {timestr}.",
        " *",
        " * Do not edit this file directly. Any changes will be lost.",
        " */",
        "",
    ]
    header = "\n".join(header_lines)
    header += align_center("Pre definitions", comment_note=COMMENT_NOTE_C)
    header += "\n#define MIXED_FILE 1\n"
    return header


def _show_version(argv0: str) -> None:
    version_text = (
        f"{argv0} ({__version__})\n"
        f"{__copyright__}\n"
        "License LGPLv3+: GNU Lesser General Public License "
        "version 3 or later <https://gnu.org/licenses/lgpl.html>\n"
        "This is free software: you are free to change and redistribute it.\n"
        "There is NO WARRANTY, to the extent permitted by law.\n"
        "Written by ChenPi11."
    )
    sys.stdout.write(version_text + "\n")


class _MyVersionAction(argparse.Action):
    """Custom action to show version information."""

    def __call__(
        self,
        parser: argparse.ArgumentParser,
        _namespace: argparse.Namespace,
        _values: str | typing.Sequence[typing.Any] | None,
        _option_string: str | None = None,
    ) -> None:
        """Show the version information and exit.

        Args:
            parser (ArgumentParser): The argument parser.
            _namespace (Namespace): The namespace.
            _values (str | Sequence[Any] | None): The values.
            _option_string (str | None, optional): The option string.
                Defaults to None.

        """
        _show_version(sys.argv[0])
        parser.exit(0)


def get_mixer_main_parser() -> argparse.ArgumentParser:
    """Get the argument parser for the mixer utility.

    Returns:
        argparse.ArgumentParser: The argument parser for the mixer utility.

    """
    parser = argparse.ArgumentParser(
        description="Mix all C source files into one.",
    )
    parser.add_argument(
        "directories",
        type=pathlib.Path,
        nargs="+",
        help="The directories to search for C source files.",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=pathlib.Path,
        default=pathlib.Path("mixed_output.c"),
        help="The output file path. Default is 'mixed_output.c'."
        " Use '-', 'STDOUT' for stdout",
    )
    parser.add_argument(
        "-V",
        "--verbose",
        action="store_true",
        help="Enable verbose logging.",
    )
    parser.add_argument(
        "--version",
        action=_MyVersionAction,
        nargs=0,
        help="Show the version information and exit.",
    )
    return parser


def mixer_main(args: list[str] | None = None) -> typing.NoReturn:
    """Run the mixer utility.

    Args:
        args (list[str] | None): The command-line arguments.

    """
    parser = get_mixer_main_parser()
    parsed_args = parser.parse_args(args)

    output_path = pathlib.Path(parsed_args.output)
    directories = [pathlib.Path(str(d)) for d in parsed_args.directories]
    verbose_log = bool(parsed_args.verbose)
    log_file = sys.stderr

    try:
        if str(output_path).upper() in {"-", "STDOUT"}:
            output_file = sys.stdout
            with output_file:
                header = get_mixed_header() + "\n"
                output_file.write(header)
                mix_c_dirs(
                    directories,
                    output_file,
                    log_file=log_file,
                    verbose_log=verbose_log,
                )
        else:
            output_file = output_path.open("w", encoding="utf-8")
            with output_file:
                header = get_mixed_header() + "\n"
                output_file.write(header)
                mix_c_dirs(
                    directories,
                    output_file,
                    log_file=log_file,
                    verbose_log=verbose_log,
                )
    except (MixerError, OSError) as e:
        sys.stderr.write(f"Error: {e}\n")
        sys.exit(1)

    sys.exit(0)


if __name__ == "__main__":
    mixer_main(sys.argv[1:])
