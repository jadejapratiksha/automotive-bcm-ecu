import re
import subprocess
import sys
from pathlib import Path
import json
from datetime import datetime


def generate_html_report(
    report_file: Path,
    total: int,
    passed: int,
    failed: int,
    ignored: int,
    status: str,
) -> None:
    """
    Generate a basic HTML test report.
    """

    generated_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Automotive BCM Test Report</title>

    <style>
        body {{
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            padding: 40px;
        }}

        .report {{
            max-width: 700px;
            margin: auto;
            background-color: white;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }}

        h1 {{
            text-align: center;
        }}

        table {{
            width: 100%;
            border-collapse: collapse;
            margin-top: 25px;
        }}

        th,
        td {{
            border: 1px solid #dddddd;
            padding: 12px;
            text-align: left;
        }}

        th {{
            background-color: #eeeeee;
        }}

        .pass {{
            color: green;
            font-weight: bold;
        }}

        .fail {{
            color: red;
            font-weight: bold;
        }}
    </style>
</head>

<body>
    <div class="report">
        <h1>Automotive BCM ECU</h1>
        <h2>Ceedling Unit Test Report</h2>

        <table>
            <tr>
                <th>Metric</th>
                <th>Result</th>
            </tr>

            <tr>
                <td>Total Tests</td>
                <td>{total}</td>
            </tr>

            <tr>
                <td>Passed Tests</td>
                <td>{passed}</td>
            </tr>

            <tr>
                <td>Failed Tests</td>
                <td>{failed}</td>
            </tr>

            <tr>
                <td>Ignored Tests</td>
                <td>{ignored}</td>
            </tr>

            <tr>
                <td>Status</td>
                <td class="{"pass" if status == "PASS" else "fail"}">
                    {status}
                </td>
            </tr>

            <tr>
                <td>Generated</td>
                <td>{generated_time}</td>
            </tr>
        </table>
    </div>
</body>
</html>
"""

    report_file.write_text(
        html_content,
        encoding="utf-8",
    )


def remove_ansi_codes(text: str) -> str:
    """
    Remove terminal color and formatting codes from Ceedling output.
    """

    ansi_pattern = re.compile(r"\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])")
    return ansi_pattern.sub("", text)


def parse_test_summary(output: str) -> tuple[int, int, int]:
    """
    Extract passed, failed, and ignored test counts
    from Ceedling console output.

    Returns:
        Tuple containing:
        (passed_tests, failed_tests, ignored_tests)
    """

    clean_output = remove_ansi_codes(output)

    # Common Unity/Ceedling format:
    # 55 Tests 0 Failures 0 Ignored
    unity_match = re.search(
        r"(\d+)\s+Tests?\s+(\d+)\s+Failures?\s+(\d+)\s+Ignored",
        clean_output,
        re.IGNORECASE,
    )

    if unity_match:
        total_tests = int(unity_match.group(1))
        failed_tests = int(unity_match.group(2))
        ignored_tests = int(unity_match.group(3))

        passed_tests = total_tests - failed_tests - ignored_tests

        return passed_tests, failed_tests, ignored_tests

    # Alternate Ceedling format:
    # TESTED: 55
    # PASSED: 55
    # FAILED: 0
    # IGNORED: 0
    tested_match = re.search(
        r"TESTED\s*:\s*(\d+)",
        clean_output,
        re.IGNORECASE,
    )

    passed_match = re.search(
        r"PASSED\s*:\s*(\d+)",
        clean_output,
        re.IGNORECASE,
    )

    failed_match = re.search(
        r"FAILED\s*:\s*(\d+)",
        clean_output,
        re.IGNORECASE,
    )

    ignored_match = re.search(
        r"IGNORED\s*:\s*(\d+)",
        clean_output,
        re.IGNORECASE,
    )

    if passed_match and failed_match:
        passed_tests = int(passed_match.group(1))
        failed_tests = int(failed_match.group(1))

        ignored_tests = (
            int(ignored_match.group(1))
            if ignored_match
            else 0
        )

        return passed_tests, failed_tests, ignored_tests

    # Another possible format:
    # Total Tests: 55
    # Test Failures: 0
    total_match = re.search(
        r"Total\s+Tests?\s*:\s*(\d+)",
        clean_output,
        re.IGNORECASE,
    )

    failure_match = re.search(
        r"(?:Test\s+)?Failures?\s*:\s*(\d+)",
        clean_output,
        re.IGNORECASE,
    )

    if total_match and failure_match:
        total_tests = int(total_match.group(1))
        failed_tests = int(failure_match.group(1))

        ignored_tests = (
            int(ignored_match.group(1))
            if ignored_match
            else 0
        )

        passed_tests = total_tests - failed_tests - ignored_tests

        return passed_tests, failed_tests, ignored_tests

    print("WARNING: Ceedling test summary could not be parsed.")

    print("\nPossible summary lines:")

    for line in clean_output.splitlines():
        lower_line = line.lower()

        if any(
            keyword in lower_line
            for keyword in [
                "test",
                "pass",
                "fail",
                "ignored",
            ]
        ):
            print(line)

    return 0, 0, 0


def main() -> int:
    """
    Run all Ceedling unit tests, save the console output,
    and print a test summary.

    Returns:
        0 when all tests pass.
        Nonzero when tests fail or Ceedling cannot run.
    """

    project_root = Path(__file__).resolve().parents[2]

    reports_folder = project_root / "reports"
    reports_folder.mkdir(exist_ok=True)

    log_file = reports_folder / "ceedling_output.txt"

    print("========================================")
    print("Automotive BCM Test Runner")
    print("========================================")
    print(f"Project root: {project_root}")
    print("Command: ceedling test:all")
    print()

    try:
        result = subprocess.run(
            ["cmd", "/c", "ceedling", "test:all"],
            cwd=project_root,
            capture_output=True,
            text=True,
            check=False,
        )

    except FileNotFoundError:
        print("ERROR: Windows command processor was not found.")
        return 1

    complete_output = result.stdout + result.stderr

    clean_output = remove_ansi_codes(complete_output)

    log_file.write_text(
        clean_output,
        encoding="utf-8",
    )

    print(clean_output)

    passed, failed, ignored = parse_test_summary(clean_output)

    total = passed + failed + ignored
    summary = {
    "total_tests": total,
    "passed_tests": passed,
    "failed_tests": failed,
    "ignored_tests": ignored,
    "status": "PASS" if failed == 0 else "FAIL",
    }

    summary_file = reports_folder / "test_summary.json"

    with open(summary_file, "w", encoding="utf-8") as file:
        json.dump(
            summary,
            file,
            indent=4,
    )

    html_report_file = reports_folder / "test_report.html"

    generate_html_report(
        report_file=html_report_file,
        total=total,
        passed=passed,
        failed=failed,
        ignored=ignored,
        status=summary["status"],
    )

    print()
    print("========================================")
    print("TEST SUMMARY")
    print("========================================")
    print(f"Total  : {total}")
    print(f"Passed : {passed}")
    print(f"Failed : {failed}")
    print(f"Ignored: {ignored}")
    print("========================================")

    print(f"\nCeedling output saved to:\n{log_file}")
    print(f"Test summary saved to:\n{summary_file}")
    print(f"HTML report saved to:\n{html_report_file}")

    if result.returncode == 0:
        print("\nRESULT: ALL UNIT TESTS PASSED")
    else:
        print("\nRESULT: ONE OR MORE UNIT TESTS FAILED")

    return result.returncode


if __name__ == "__main__":
    sys.exit(main())