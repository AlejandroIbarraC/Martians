import parser as parser
import report as report


def main():
    timelines = parser.parse_timeline()
    arribos, nombre_algoritmo, marcianos = parser.parse_marcianos()
    report.plot_report(marcianos, timelines, arribos, nombre_algoritmo)

main()