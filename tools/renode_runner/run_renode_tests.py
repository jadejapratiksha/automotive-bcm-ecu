"""
Renode test runner for the Automotive BCM ECU project.

This script:
1. Locates the project root.
2. Verifies that Renode and the STM32 ELF exist.
3. Starts Renode using the BCM startup script.
4. Captures Renode output.
5. Saves the output into the reports directory.

The UART behavior will be validated later after the firmware boot issue
is resolved.
"""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path


def find_project_root() -> Path:
    """
    Locate the project root using this script's directory.

    Expected location:
        tools/renode_runner/run_renode_tests.py
    """

    return Path(__file__).resolve().parents[2]


def find_renode_executable() -> str | None:
    """
    Locate Renode on Windows.

    Renode may be available through either:
        renode
        renode.exe
        renode-test
    """

    candidates = [
        "renode",
        "renode.exe",
        "Renode.exe",
    ]

    for candidate in candidates:
        executable = shutil.which(candidate)

        if executable is not None:
            return executable

    return None


def main() -> int:
    project_root = find_project_root()

    elf_file = (
        project_root
        / "platform"
        / "stm32f407"
        / "Debug"
        / "stm32f407.elf"
    )

    renode_script = (
        project_root
        / "simulation"
        / "renode"
        / "bcm.resc"
    )

    reports_directory = project_root / "reports"
    reports_directory.mkdir(parents=True, exist_ok=True)

    output_file = reports_directory / "renode_output.txt"

    print("=" * 50)
    print("AUTOMOTIVE BCM RENODE TEST RUNNER")
    print("=" * 50)

    if not elf_file.exists():
        print("\nERROR: STM32 ELF file was not found:")
        print(elf_file)
        print("\nBuild the STM32CubeIDE project first.")
        return 1

    if not renode_script.exists():
        print("\nERROR: Renode startup script was not found:")
        print(renode_script)
        return 1

    renode_executable = find_renode_executable()

    if renode_executable is None:
        print("\nERROR: Renode executable was not found.")
        print("Make sure Renode is installed and available in PATH.")
        return 1

    print(f"\nProject root:\n{project_root}")
    print(f"\nELF file:\n{elf_file}")
    print(f"\nRenode script:\n{renode_script}")
    print("\nStarting Renode...")

    command = [
    renode_executable,
        "--disable-xwt",
        "--console",
        "-e",
        "include @simulation/renode/bcm.resc",
    ]

    try:
        result = subprocess.run(
            command,
            cwd=project_root,
            capture_output=True,
            text=True,
            timeout=20,
            check=False,
        )

    except subprocess.TimeoutExpired as error:
        stdout = error.stdout or ""
        stderr = error.stderr or ""

        if isinstance(stdout, bytes):
            stdout = stdout.decode(errors="replace")

        if isinstance(stderr, bytes):
            stderr = stderr.decode(errors="replace")

        combined_output = stdout + stderr

        output_file.write_text(
            combined_output,
            encoding="utf-8",
        )

        print("\nRenode ran for 20 seconds and was stopped.")
        print("This is expected while the firmware runs continuously.")
        print(f"\nRenode output saved to:\n{output_file}")

        return 0

    except OSError as error:
        print(f"\nERROR: Could not start Renode: {error}")
        return 1

    combined_output = result.stdout + result.stderr

    output_file.write_text(
        combined_output,
        encoding="utf-8",
    )

    print(combined_output)

    print(f"\nRenode output saved to:\n{output_file}")

    if result.returncode != 0:
        print(f"\nRESULT: RENODE EXITED WITH CODE {result.returncode}")
        return result.returncode

    print("\nRESULT: RENODE SCRIPT COMPLETED")
    return 0


if __name__ == "__main__":
    sys.exit(main())