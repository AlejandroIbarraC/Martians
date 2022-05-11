from parso import parse
import parser as parser
import report as report


def main():
    timelines, marcianos = parser.parse_timeline()
    arribos = parser.parse_marcianos()
    report.plot_report(marcianos, timelines, arribos)

main()