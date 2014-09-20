    Usage: ds2-dump [options]
    Dumps a DS2 packet from an ECU
    
    Options:
      -h, --help                             Displays this help.
      -v, --version                          Displays version information.
      -p, --port <port>                      Read from serial port <port>.
      --dpp-source-dir <dpp-source-dir>      Specify location of DPP-JSON files
      --dpp-dir <dpp-dir>                    Specify location of DPP database
      -r, --reload, --load                   Load JSON data into SQL db
      -e, --ecu <ecu>                        The ECU to operate on (family name,
                                             numerical address, or UUID).
      -f, --families                         Print the known ECU families.
      -c, --ecus <family>                    Print the known ECUs for a given
                                             family.
      -o, --operations                       Print the known operations for a given
                                             ECU.  ECU UUID must be specified with
                                             --ecu.
      -n, --iterate <n>                      Iterate <n> number of times.
      -i, --input-packet <input-packet>      Treat this argument as a packet
                                             instead of reading from the serial
                                             port.  Base 16, space delimited.
      -P, --probe                            Probe an ECU at <ecu> for its
                                             identity.
      -A, --probe-all                        Probe all known ECU addresses and
                                             print the results.
      -J, --run-operation <operation>        Run an operation on an ECU, prints
                                             results as JSON to stdout.  Must also
                                             specify --ecu.
      -D, --data-log <ecu-jobs-and-results>  Create a CSV log, write until
                                             interrupted.