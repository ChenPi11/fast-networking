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

"""Utility to generate an aligned title and output.

Like this:

# ========== TITLE ==========

"""

import argparse
import sys
import typing

__all__ = ["AlignError", "AlignWidthError", "align_center"]

__version__ = "0.1.0"
__author__ = "ChenPi11"
__license__ = "LGPL-3.0-or-later"
__copyright__ = f"Copyright (C) 2025 {__author__}"


class AlignError(Exception):
    """Custom exception for alignment errors."""


class AlignWidthError(AlignError):
    """Custom exception for alignment width errors."""


def align_center(
    title: str,
    *,
    width: int = 80,
    fillstr: str = "=",
    comment_note: tuple[str, str] = ("# ", ""),
) -> str:
    """Generate an aligned title string.

    Args:
        title (str): The title to align.
        width (int, optional): The width of the aligned title. Defaults to 80.
        fillstr (str, optional): The fill string to use. Defaults to "=".
        comment_note (tuple[str, str], optional): The comment note to use.
            Use (start, end). Defaults to ("# ", "").

    Returns:
        str: The aligned title string.

    Raises:
        AlignWidthError: If the width is too small to fit the title.

    """
    title = f" {title.strip()} "
    if width < len(title) + len(comment_note[0]) + len(comment_note[1]):
        msg = f"Width {width} is too small to fit title {title!r}."
        raise AlignWidthError(msg)

    total_fill_length = (
        width
        - len(title)
        - len(comment_note[0])
        - len(comment_note[1])  # Subtract lengths of comment notes.
    ) // len(fillstr)
    left_fill = total_fill_length // 2
    right_fill = total_fill_length - left_fill

    return (
        f"{comment_note[0]}{fillstr * left_fill}{title}"
        f"{fillstr * right_fill}{comment_note[1]}"
    )


# Python-style comment note.
COMMENT_NOTE_PYTHON = ("# ", "")

# C-style comment note.
COMMENT_NOTE_C = ("/* ", " */")

# No comment note.
COMMENT_STYLE_NONE = ("", "")

# Default comment note.
COMMENT_NOTE_DEFAULT = COMMENT_NOTE_C

# Comment styles.
STYLES: dict[str, tuple[str, str]] = {
    "python": COMMENT_NOTE_PYTHON,
    "c": COMMENT_NOTE_C,
    "none": COMMENT_STYLE_NONE,
}


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


def get_align_main_parser() -> argparse.ArgumentParser:
    """Get the argument parser for the align utility.

    Returns:
        argparse.ArgumentParser: The argument parser for the align utility.

    """
    parser = argparse.ArgumentParser(
        description="Generate an aligned title string.",
    )
    parser.add_argument(
        "title",
        type=str,
        help="The title to align.",
    )
    parser.add_argument(
        "-s",
        "--style",
        type=str,
        choices=["python", "c", "none"],
        default="none",
        help="The comment style to use.",
    )
    parser.add_argument(
        "-w",
        "--width",
        type=int,
        default=80,
        help="The width of the aligned title.",
    )
    parser.add_argument(
        "-f",
        "--fillstr",
        type=str,
        default="=",
        help="The fill string to use.",
    )
    parser.add_argument(
        "--comment-start",
        type=str,
        default="",
        help="The comment start string to use. Will override style if set.",
    )
    parser.add_argument(
        "--comment-end",
        type=str,
        default="",
        help="The comment end string to use. Will override style if set.",
    )
    parser.add_argument(
        "-v",
        "--version",
        action=_MyVersionAction,
        nargs=0,
        help="Show the version information and exit.",
    )

    return parser


def align_main(args: list[str] | None = None) -> typing.NoReturn:
    """Run the align utility."""
    parser = get_align_main_parser()
    parsed_args = parser.parse_args(args)

    # Apply style template.
    comment_note = STYLES[parsed_args.style]
    if parsed_args.comment_start:
        comment_note = (parsed_args.comment_start, comment_note[1])
    if parsed_args.comment_end:
        comment_note = (comment_note[0], parsed_args.comment_end)
    width = parsed_args.width
    fillstr = parsed_args.fillstr

    try:
        aligned_title = align_center(
            title=parsed_args.title,
            width=width,
            fillstr=fillstr,
            comment_note=comment_note,
        )
        sys.stdout.write(aligned_title + "\n")
        sys.exit(0)
    except AlignWidthError as e:
        sys.stderr.write(f"Error: {e}\n")
        sys.exit(1)


if __name__ == "__main__":
    align_main(sys.argv[1:])
