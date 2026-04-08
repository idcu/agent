#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
IDCU Agent Performance Benchmark Report Generator
Generates comprehensive HTML/Markdown reports from benchmark results
"""

import json
import argparse
import sys
import os
from datetime import datetime


def load_json_file(filepath):
    """Load JSON file"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading {filepath}: {e}", file=sys.stderr)
        return None


def generate_markdown_report(baseline, current, output_file):
    """Generate Markdown report"""
    report = []
    report.append("# IDCU Agent Performance Benchmark Report\n")
    report.append(f"**Generated on**: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n")

    if baseline:
        report.append("## Summary\n\n")
        report.append("| Benchmark | Baseline (ops/s) | Current (ops/s) | Change | Status |\n")
        report.append("|-----------|------------------|-----------------|--------|--------|\n")

        baseline_dict = {b['name']: b for b in baseline['benchmarks']}
        current_dict = {b['name']: b for b in current['benchmarks']}

        all_names = set(baseline_dict.keys()) | set(current_dict.keys())

        for name in sorted(all_names):
            base = baseline_dict.get(name)
            curr = current_dict.get(name)

            base_ops = base['ops_per_sec'] if base else 'N/A'
            curr_ops = curr['ops_per_sec'] if curr else 'N/A'

            change = ''
            status = ''

            if base and curr:
                change_pct = ((curr_ops - base_ops) / base_ops) * 100
                change = f"{change_pct:+.1f}%"
                if change_pct < -10:
                    status = '🔴 REGRESSION'
                elif change_pct < 0:
                    status = '🟡 SLOWER'
                elif change_pct > 10:
                    status = '🟢 BIG IMPROVEMENT'
                else:
                    status = '🟢 OK'

            report.append(f"| {name} | {base_ops} | {curr_ops} | {change} | {status} |\n")

    report.append("\n## Detailed Results\n\n")

    for bench in current['benchmarks']:
        report.append(f"### {bench['name']}\n\n")
        report.append(f"- **Mean time**: {bench['mean_us']:.2f} µs\n")
        report.append(f"- **Min time**: {bench['min_us']} µs\n")
        report.append(f"- **Max time**: {bench['max_us']} µs\n")
        report.append(f"- **StdDev**: {bench['stddev_us']:.2f} µs\n")
        report.append(f"- **Throughput**: {bench['ops_per_sec']:,} ops/s\n\n")

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write('\n'.join(report))

    print(f"Markdown report generated: {output_file}")


def generate_html_report(baseline, current, output_file):
    """Generate HTML report"""
    html = []
    html.append("<!DOCTYPE html>")
    html.append("<html lang='en'>")
    html.append("<head>")
    html.append("    <meta charset='UTF-8'>")
    html.append("    <meta name='viewport' content='width=device-width, initial-scale=1.0'>")
    html.append("    <title>IDCU Agent Performance Benchmark</title>")
    html.append("    <style>")
    html.append("        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }")
    html.append("        .container { max-width: 1200px; margin: 0 auto; background: white; padding: 40px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }")
    html.append("        h1 { color: #333; border-bottom: 3px solid #4CAF50; padding-bottom: 10px; }")
    html.append("        h2 { color: #555; margin-top: 30px; }")
    html.append("        h3 { color: #666; }")
    html.append("        table { width: 100%; border-collapse: collapse; margin: 20px 0; }")
    html.append("        th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }")
    html.append("        th { background: #4CAF50; color: white; }")
    html.append("        tr:hover { background: #f5f5f5; }")
    html.append("        .regression { background: #ffebee !important; }")
    html.append("        .improvement { background: #e8f5e9 !important; }")
    html.append("        .benchmark-card { background: #f9f9f9; padding: 20px; margin: 10px 0; border-radius: 8px; border-left: 4px solid #4CAF50; }")
    html.append("        .stat { display: inline-block; margin-right: 30px; }")
    html.append("        .stat-label { font-weight: bold; color: #666; }")
    html.append("        .stat-value { font-size: 1.2em; color: #333; }")
    html.append("    </style>")
    html.append("</head>")
    html.append("<body>")
    html.append("    <div class='container'>")
    html.append(f"        <h1>IDCU Agent Performance Benchmark</h1>")
    html.append(f"        <p><strong>Generated on:</strong> {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>")

    if baseline:
        html.append("        <h2>Summary Comparison</h2>")
        html.append("        <table>")
        html.append("            <tr><th>Benchmark</th><th>Baseline (ops/s)</th><th>Current (ops/s)</th><th>Change</th><th>Status</th></tr>")

        baseline_dict = {b['name']: b for b in baseline['benchmarks']}
        current_dict = {b['name']: b for b in current['benchmarks']}

        all_names = set(baseline_dict.keys()) | set(current_dict.keys())

        for name in sorted(all_names):
            base = baseline_dict.get(name)
            curr = current_dict.get(name)

            base_ops = base['ops_per_sec'] if base else 'N/A'
            curr_ops = curr['ops_per_sec'] if curr else 'N/A'

            change = ''
            status = ''
            row_class = ''

            if base and curr:
                change_pct = ((curr_ops - base_ops) / base_ops) * 100
                change = f"{change_pct:+.1f}%"
                if change_pct < -10:
                    status = '🔴 REGRESSION'
                    row_class = 'regression'
                elif change_pct < 0:
                    status = '🟡 SLOWER'
                elif change_pct > 10:
                    status = '🟢 BIG IMPROVEMENT'
                    row_class = 'improvement'
                else:
                    status = '🟢 OK'

            html.append(f"            <tr class='{row_class}'><td>{name}</td><td>{base_ops}</td><td>{curr_ops}</td><td>{change}</td><td>{status}</td></tr>")

        html.append("        </table>")

    html.append("        <h2>Detailed Results</h2>")

    for bench in current['benchmarks']:
        html.append(f"        <div class='benchmark-card'>")
        html.append(f"            <h3>{bench['name']}</h3>")
        html.append(f"            <div class='stat'><span class='stat-label'>Mean time:</span> <span class='stat-value'>{bench['mean_us']:.2f} µs</span></div>")
        html.append(f"            <div class='stat'><span class='stat-label'>Min time:</span> <span class='stat-value'>{bench['min_us']} µs</span></div>")
        html.append(f"            <div class='stat'><span class='stat-label'>Max time:</span> <span class='stat-value'>{bench['max_us']} µs</span></div>")
        html.append(f"            <div class='stat'><span class='stat-label'>StdDev:</span> <span class='stat-value'>{bench['stddev_us']:.2f} µs</span></div>")
        html.append(f"            <div class='stat'><span class='stat-label'>Throughput:</span> <span class='stat-value'>{bench['ops_per_sec']:,} ops/s</span></div>")
        html.append(f"        </div>")

    html.append("    </div>")
    html.append("</body>")
    html.append("</html>")

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write('\n'.join(html))

    print(f"HTML report generated: {output_file}")


def main():
    parser = argparse.ArgumentParser(description='Generate benchmark reports')
    parser.add_argument('--current', required=True, help='Current benchmark JSON file')
    parser.add_argument('--baseline', help='Baseline benchmark JSON file for comparison')
    parser.add_argument('--output-dir', default='.', help='Output directory for reports')
    parser.add_argument('--format', choices=['md', 'html', 'both'], default='both',
                        help='Report format (md, html, or both)')

    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    current = load_json_file(args.current)
    if not current:
        return 1

    baseline = None
    if args.baseline:
        baseline = load_json_file(args.baseline)

    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')

    if args.format in ['md', 'both']:
        md_file = os.path.join(args.output_dir, f'benchmark_report_{timestamp}.md')
        generate_markdown_report(baseline, current, md_file)

    if args.format in ['html', 'both']:
        html_file = os.path.join(args.output_dir, f'benchmark_report_{timestamp}.html')
        generate_html_report(baseline, current, html_file)

    return 0


if __name__ == '__main__':
    sys.exit(main())
