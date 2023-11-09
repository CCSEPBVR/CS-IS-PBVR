#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
 pfinfo:
   reports time-range and max-subvolume number of pfi/pfl file.

 usage: pfinfo.py infile.{pfi|pfl}
 outputs to stdout: startstep endstep maxsubvol

"""
import sys, os
import struct


def ScanPFI(path):
    """
    read a single PFI file, and returns start/end/maxsubvol as list
     [in] path  pathname of PFI
     [ret] [start, end, maxsubvol] or None
    """
    try:
        ifp = open(path, 'rb')
    except:
        sys.stderr.write('error: can not open file: %s\n' % path)
        return None

    try:
        header = ifp.read(36) # 9 * 4bytes
    except:
        sys.stderr.write('error: read header from file: %s\n' % path)
        ifp.close()
        return None
    ifp.close()

    buff = struct.unpack('9i', header)
    #[staSteps, endSteps, numSubVolumes] = list(buff[6:])
    return buff[6:]


def ScanPFL(path):
    """
    read PFL file, and returns start/end/maxsubvol as list
     [in] path  pathname of PFL
     [ret] [start, end, maxsubvol] or None
    """
    try:
        ifp = open(path, 'r')
    except:
        sys.stderr.write('error: can not open file: %s\n' % path)
        return None
    dir_path = os.path.dirname(path)

    # read header
    line = ifp.readline()
    if line.strip() != '#PBVR PFI FILES':
        sys.stderr.write('error: invalid hrader, may not PFL file: %s\n' % path)
        return None

    # read the first pfi
    ret_tup = [0,0,0]
    try:
        line = ifp.readline()
    except:
        sys.stderr.write('error: read failed from file: %s\n' % path)
        ifp.close()
        return None
    xpath = os.path.join(dir_path, line.strip())
    xret = ScanPFI(xpath)
    if xret == None:
        ifp.close()
        return None
    ret_tup[:] = xret[:]

    # join follow pfi
    line = ifp.readline()
    while line:
        xpath = os.path.join(dir_path, line.strip())
        xret = ScanPFI(xpath)
        if xret == None:
            line = ifp.readline()
            continue
        if xret[0] < ret_tup[0]:
            ret_tup[0] = xret[0]
        if xret[1] > ret_tup[1]:
            ret_tup[1] = xret[1]
        ret_tup[2] += xret[2]

        line = ifp.readline()
        continue

    return ret_tup


if __name__ == '__main__':
    tgtf = ""
    if ( len(sys.argv) > 1 ):
        tgtf = sys.argv[1]
    else:
        sys.stderr.write('usage: %s infile.{pfi|pfl}\n' % sys.argv[0])
        sys.exit(-1)

    # check file suffix
    if tgtf == '-h' or tgtf == '--help':
        sys.stdout.write(__doc__)
        sys.exit(0)
    if len(tgtf) < 4:
        sys.stderr.write('error: infile name needs suffix .pfi or .pfl\n')
        sys.exit(-1)
    if tgtf[-3:] == 'pfi':
        ret = ScanPFI(tgtf)
    elif tgtf[-3:] == 'pfl':
        ret = ScanPFL(tgtf)
    else:
        sys.stderr.write('error: infile name needs suffix .pfi or .pfl\n')
        sys.exit(-1)

    if ret == None:
        sys.exit(-1)

    sys.stdout.write('%d %d %d\n' % tuple(ret))
    sys.exit(0)

